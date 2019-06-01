/*
 * async_client.cpp
 *
 *  Created on: May 31, 2019
 *      Author: frank
 */

#include <memory>
#include <iostream>
#include <string>
#include <thread>

#include <grpcpp/grpcpp.h>
#include <grpc/support/log.h>

#include <chrono>
#include <utility>
#include <string>
#include <iostream>
#include <boost/context/detail/config.hpp>
#include <boost/context/detail/fcontext.hpp>

#include <boost/context/fiber.hpp>
#include "my_helloworld.grpc.pb.h"

using grpc::Status;
using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Greeter;
using helloworld::Calculator;
using helloworld::AddRequest;
using helloworld::AddResponse;

namespace ctx = boost::context;

class AbstractClient
{
public:
	AbstractClient(CompletionQueue& cq):cq_(cq) {}
	void Yield()
	{

	}
	void Resume()
	{
		f_ = std::move(f_).resume();
	}

	void SetCoro(ctx::fiber&& f)
	{
		f_ = std::move(f);
	}
protected:
	CompletionQueue& cq_;
	ctx::fiber f_;
};

class GreeterClient : public AbstractClient
{
public:
	GreeterClient(CompletionQueue& cq, std::shared_ptr<::grpc::Channel> channel):
		AbstractClient(cq),
		stub_(Greeter::NewStub(channel)){}

	ctx::fiber SayHello(HelloRequest& request, ctx::fiber&& f)
	{
        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        ClientContext context;
        // Storage for the status of the RPC upon completion.
        Status status;
        std::unique_ptr<ClientAsyncResponseReader<HelloReply>> response_reader =
        		stub_->PrepareAsyncSayHello(&context, request, &cq_);
        // StartCall initiates the RPC call
        response_reader->StartCall();

        HelloReply response;

        // Request that, upon completion of the RPC, "reply" be updated with the
        // server's response; "status" with the indication of whether the operation
        // was successful. Tag the request with the memory address of the call object.
        response_reader->Finish(&response, &status, (void*)this);

        f = std::move(f).resume();

        std::cout<<"get reply:"<<response.ShortDebugString()<<"\n";

        return std::move(f);
	}
private:
	std::unique_ptr<Greeter::Stub> stub_;
};

class CalculatorClient : public AbstractClient
{
public:
	CalculatorClient(CompletionQueue& cq, std::shared_ptr<::grpc::Channel> channel):
		AbstractClient(cq),
		stub_(Calculator::NewStub(channel)){}
	void Add(AddRequest& request, AddResponse& response);
private:
	std::unique_ptr<Calculator::Stub> stub_;
};

// Loop while listening for completed responses.
// Prints out the response from the server.
void AsyncCompleteRpc(CompletionQueue& cq, gpr_timespec tp) {
    void* got_tag;
    bool ok = false;

    // Block until the next result is available in the completion queue "cq".
    while (CompletionQueue::GOT_EVENT == cq.AsyncNext(&got_tag, &ok, tp)) {
        // The tag in this example is the memory location of the call object
    	AbstractClient* call = static_cast<AbstractClient*>(got_tag);

        // Verify that the request was completed successfully. Note that "ok"
        // corresponds solely to the request for updates introduced by Finish().
        GPR_ASSERT(ok);

        std::cout<<"get call\n";

        call->Resume();
    }
}

void Create(GreeterClient* gc, int i)
{
	auto f = [gc, i](ctx::fiber&& f){
		std::string name = std::string("name") + std::to_string(i);
		HelloRequest request;
		HelloReply response;
		request.set_name(name);
		return gc->SayHello(request, std::move(f));
	};
	ctx::fiber coro(f);
	gc->SetCoro(std::move(coro).resume());
}

int main()
{
	CompletionQueue cq;
	std::shared_ptr<::grpc::Channel> channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());

	for(int i = 0; i < 100; ++i)
	{
		GreeterClient* gc = new GreeterClient(cq, channel);
		Create(gc, i);
	}

	while(1)
    {
    	gpr_timespec tp = gpr_time_add(gpr_now(GPR_CLOCK_MONOTONIC), gpr_time_from_millis(5, GPR_TIMESPAN));
    	AsyncCompleteRpc(cq, tp);
    	std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    std::cout<<"press ctrl+c to exit\n";
}



