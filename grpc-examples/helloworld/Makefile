all:
	clang++ -std=c++2a -lstdc++ -fcoroutines-ts helloworld.pb.cc helloworld.grpc.pb.cc await_greeter_async_client2_1.cc -o await_greeter_async_client2_1 -lprotobuf -lgrpc -lgrpc++
	clang++ -std=c++2a -lstdc++ -fcoroutines-ts helloworld.pb.cc helloworld.grpc.pb.cc await_greeter_async_client2.cc -o await_greeter_async_client2 -lprotobuf -lgrpc -lgrpc++
	#clang++ -std=c++2a -lstdc++ -fcoroutines-ts helloworld.pb.cc helloworld.grpc.pb.cc greeter_async_client2_boost_coroutine2.cc -o greeter_async_client2_boost_coroutine2 -lprotobuf -lgrpc -lgrpc++ -lboost_context-mt
	clang++ -std=c++2a -lstdc++ helloworld.pb.cc helloworld.grpc.pb.cc greeter_async_client2_boost_fiber.cc -o greeter_async_client2_boost_fiber -lprotobuf -lgrpc -lgrpc++ -lboost_context-mt -lboost_fiber-mt
	clang++ -std=c++2a -lstdc++ helloworld.pb.cc helloworld.grpc.pb.cc greeter_async_client2_boost_fcontext.cc -o greeter_async_client2_boost_fcontext -lprotobuf -lgrpc -lgrpc++ -lboost_context-mt
clean:
	rm -f await_greeter_async_client2_1 await_greeter_async_client2 greeter_async_client2_boost_coroutine2 greeter_async_client2_boost_fiber greeter_async_client2_boost_fcontext

