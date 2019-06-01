/*
 * async_server.cpp
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

#include "my_helloworld.grpc.pb.h"

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Status;
using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Greeter;
using helloworld::Calculator;
using helloworld::AddRequest;
using helloworld::AddResponse;

class GreeterService final : public Greeter::Service {
public:
	virtual ::grpc::Status SayHello(::grpc::ServerContext* context, const ::helloworld::HelloRequest* request, ::helloworld::HelloReply* response) override
	{
		response->set_message(std::string("hello ") + request->name());
		return Status::OK;
	}

	virtual ::grpc::Status PingPong(::grpc::ServerContext* context, const ::helloworld::PingpongRequest* request, ::helloworld::PingpongResponse* response) override
	{
		response->set_pong("pong");
		return Status::OK;
	}
};

class CalulatorService final : public Calculator::Service {
public:
	virtual ::grpc::Status Add(::grpc::ServerContext* context, const ::helloworld::AddRequest* request, ::helloworld::AddResponse* response) override
	{
		response->set_sum(request->a() + request->b());
		return Status::OK;
	}
};

void RunServer() {
	std::string server_address("0.0.0.0:50051");
	GreeterService service;
	CalulatorService cs;

	ServerBuilder builder;
	// Listen on the given address without any authentication mechanism.
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	// Register "service" as the instance through which we'll communicate with
	// clients. In this case it corresponds to an *synchronous* service.
	builder.RegisterService(&service);
	builder.RegisterService(&cs);

	// Finally assemble the server.
	std::unique_ptr<Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;

	// Wait for the server to shutdown. Note that some other thread must be
	// responsible for shutting down the server for this call to ever return.
	server->Wait();
}

int main(int argc, char** argv) {
	RunServer();
	return 0;
}

