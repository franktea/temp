/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include <grpc/support/log.h>
#include <thread>

#include <chrono>
#include <utility>
#include <string>
#include <iostream>
#include <boost/context/detail/config.hpp>
#include <boost/context/detail/fcontext.hpp>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "helloworld.grpc.pb.h"
#endif

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Greeter;

template< std::size_t Max, std::size_t Default, std::size_t Min >
class simple_stack_allocator
{
public:
    static constexpr std::size_t maximum_stacksize()
    { return Max; }

    static constexpr std::size_t default_stacksize()
    { return Default; }

    static constexpr std::size_t minimum_stacksize()
    { return Min; }

    void * allocate( std::size_t size) const
    {
        assert( minimum_stacksize() <= size);
        assert( maximum_stacksize() >= size);

        void * limit = malloc( size);
        if ( ! limit) throw std::bad_alloc();

        return static_cast< char * >( limit) + size;
    }

    void deallocate( void * vp, std::size_t size) const
    {
        assert( vp);
        assert( minimum_stacksize() <= size);
        assert( maximum_stacksize() >= size);

        void * limit = static_cast< char * >( vp) - size;
        free( limit);
    }
};

namespace ctx = boost::context::detail;

using stack_allocator = simple_stack_allocator<
    8 * 1024 * 1024, // 8MB
    64 * 1024, // 64kB
    8 * 1024 // 8kB
>;

class GreeterClient;
class GreeterCoro
{
public:
	GreeterCoro(std::string name, GreeterClient& greeter, ctx::fcontext_t ctx):name_(name),
		greeter_(greeter),
		ctx_(ctx){}

	ctx::transfer_t run(ctx::transfer_t t);

	void SetCtx(ctx::fcontext_t ctx)
	{
		ctx_ = ctx;
	}

	ctx::fcontext_t GetCtx()
	{
		return ctx_;
	}
private:
	std::string name_;
	GreeterClient& greeter_;
	ctx::fcontext_t ctx_; // 当前协程
};

void start( ctx::transfer_t t) {
	GreeterCoro* gr = (GreeterCoro*)t.data;
	ctx::transfer_t from = gr->run(t);
    ctx::jump_fcontext(from.fctx, 0);
}

void spawn(std::string name, GreeterClient& greeter)
{
    stack_allocator alloc;
    void * sp = alloc.allocate( stack_allocator::default_stacksize() );
    ctx::fcontext_t ctx = ctx::make_fcontext( sp, stack_allocator::default_stacksize(), start);
    assert( ctx);

    GreeterCoro* coro = new GreeterCoro(name, greeter, ctx);
    ctx::transfer_t t = ctx::jump_fcontext(ctx, coro);
    coro->SetCtx(t.fctx);
    //std::cout<<"coro "<<name<<" finished.\n";
	//alloc.deallocate( sp, stack_allocator::default_stacksize() );
}

class GreeterClient {
  public:
    explicit GreeterClient(std::shared_ptr<Channel> channel)
            : stub_(Greeter::NewStub(channel)) {}

    // Assembles the client's payload and sends it to the server.
    ctx::transfer_t SayHello(GreeterCoro* coro, const std::string& user, ctx::transfer_t t, ctx::fcontext_t ctx) {

    	std::cout<<user<<" running.....\n";

        // Data we are sending to the server.
        HelloRequest request;
        request.set_name(user);

        // Call object to store rpc data
        AsyncClientCall* call = new AsyncClientCall;

        call->coro = coro;

        // stub_->PrepareAsyncSayHello() creates an RPC object, returning
        // an instance to store in "call" but does not actually start the RPC
        // Because we are using the asynchronous API, we need to hold on to
        // the "call" instance in order to get updates on the ongoing RPC.
        call->response_reader =
            stub_->PrepareAsyncSayHello(&call->context, request, &cq_);

        // StartCall initiates the RPC call
        call->response_reader->StartCall();

        // Request that, upon completion of the RPC, "reply" be updated with the
        // server's response; "status" with the indication of whether the operation
        // was successful. Tag the request with the memory address of the call object.
        call->response_reader->Finish(&call->reply, &call->status, (void*)call);

        ctx::transfer_t from = ctx::jump_fcontext(t.fctx, 0);

        std::cout<<user<<" come back...\n";

        if (call->status.ok())
            std::cout << "Greeter received: " << call->reply.message() << std::endl;
        else
            std::cout << "RPC failed: " << call->status.error_message() << std::endl;

        // Once we're complete, deallocate the call object.
        delete call;

        return from;
    }

    // Loop while listening for completed responses.
    // Prints out the response from the server.
    void AsyncCompleteRpc(gpr_timespec tp) {
        void* got_tag;
        bool ok = false;

        // Block until the next result is available in the completion queue "cq".
        while (CompletionQueue::GOT_EVENT == cq_.AsyncNext(&got_tag, &ok, tp)) {
            // The tag in this example is the memory location of the call object
            AsyncClientCall* call = static_cast<AsyncClientCall*>(got_tag);

            // Verify that the request was completed successfully. Note that "ok"
            // corresponds solely to the request for updates introduced by Finish().
            GPR_ASSERT(ok);

            std::cout<<"get call\n";

            ctx::jump_fcontext(call->coro->GetCtx(), 0);
        }
    }

  private:

    // struct for keeping state and data information
    struct AsyncClientCall {
        // Container for the data we expect from the server.
        HelloReply reply;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        ClientContext context;

        // Storage for the status of the RPC upon completion.
        Status status;

        GreeterCoro* coro;

        std::unique_ptr<ClientAsyncResponseReader<HelloReply>> response_reader;
    };

    // Out of the passed in Channel comes the stub, stored here, our view of the
    // server's exposed services.
    std::unique_ptr<Greeter::Stub> stub_;

    // The producer-consumer queue we use to communicate asynchronously with the
    // gRPC runtime.
    CompletionQueue cq_;
};

int main(int argc, char** argv) {


    // Instantiate the client. It requires a channel, out of which the actual RPCs
    // are created. This channel models a connection to an endpoint (in this case,
    // localhost at port 50051). We indicate that the channel isn't authenticated
    // (use of InsecureChannelCredentials()).
    GreeterClient greeter(grpc::CreateChannel(
            "localhost:50051", grpc::InsecureChannelCredentials()));

    // Spawn reader thread that loops indefinitely
    //std::thread thread_ = std::thread(&GreeterClient::AsyncCompleteRpc, &greeter);

    for (int i = 0; i < 100; i++) {
        std::string user("world " + std::to_string(i));
        spawn(user, greeter);
    }

    while(1)
    {
    	gpr_timespec tp = gpr_time_add(gpr_now(GPR_CLOCK_MONOTONIC), gpr_time_from_millis(5, GPR_TIMESPAN));
    	greeter.AsyncCompleteRpc(tp);
    	std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    std::cout<<"finished all.\n";

    return 0;
}

inline ctx::transfer_t GreeterCoro::run(ctx::transfer_t t)
{
	return greeter_.SayHello(this, name_, t, ctx_);
}
