proto:
	protoc my_helloworld.proto --cpp_out=.
	protoc my_helloworld.proto --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin`
	
server:
	clang++ -std=c++2a -lstdc++ my_helloworld.pb.cc my_helloworld.grpc.pb.cc server.cpp -o server -lprotobuf -lgrpc -lgrpc++

client:
	clang++ -std=c++2a -lstdc++ async_client.cpp my_helloworld.pb.cc my_helloworld.grpc.pb.cc -o async_client -lprotobuf -lgrpc -lgrpc++ -lboost_context-mt

all:
	make client server

clean:
	rm -f server async_client

