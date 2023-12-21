all:
	clang++ -std=c++2a echo_server.cpp -I `brew --prefix boost`/include -L `brew --prefix boost`/lib -lboost_coroutine -lboost_system -o server
	clang++ -std=c++2a async_echo_client.cpp -I `brew --prefix boost`/include -L `brew --prefix boost`/lib -lboost_coroutine -lboost_system -o client

clean:
	rm -f server client

