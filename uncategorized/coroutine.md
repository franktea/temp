随着coroutine ts正式进入C++20，C++已经进入协程时代了。而且visual studio和clang早就支持了，肯定很多C++er都想学习协程的使用。不幸的是目前的协程功能是给库的开发者使用的，非常难用。在此基础上提供的位于std的各种协程编写需要用到的辅助类和函数已经来不及加入C++20了，估计要等到23。所以目前来说，如果要使用协程，要么等到别人帮你包装好，要么自己学会包装。

怕难的人不会学c++（和rust），所以既然协程来了，我们也一起学起来吧。这篇叫协程入门，我尽量只写最简单最有用的东西，而且我希望是提供一个异步转协程的通用套路。

对于此文中的错误大家发现以后请及时告诉我，我会第一时间修改。

协程的出现主要是为了解决异步编程的麻烦，异步编程一般是这样的：

```
async_call(input1, intput2, ..., call_back)
```

就是用一堆输入参数再加上一个回调函数作为参数，`async_call`函数调用后立即返回，当异步操作完成时，call_back函数会被调用。

在C++中，使用异步的场景主要有两种：
1、需要等待的结果在其它进程中提供，比如说调用操作系统的异步io读写文件、通过网络发送请求到其它进程等待处理以后结果依然通过网络返回，文件读写完成或是网络请求返回时调用指定的回调函数；
2、需要等待的结果在本进程中提供，那就是在别的线程中，请求发送到别的线程，别的线程操作完成以后，调用指定的回调函数。
不管是那种情况，对于异步调用，都可以用协程改造成一个如下格式的“同步”调用：

```
co_await coro_call(input1, input2, ...)
```
其实改造起来极为简单，只要在回调函数中调用resume()恢复协程即可。对于1和2两种情况，第一种情况更简单，因为第二种可能会需要考虑同步的问题。

协程通过Promise和Awaitable接口的至少15个函数来提供给程序员定制自己协程的流程和功能，实现世界上最简单的协程需要用到其中的8个（5个Promise的函数和3个Awaitable的函数），其中Promise的函数可以先不管它，先来看Awaitable的3个函数。

对于语句`co_await blabla;`,blabla就必须实现Awaitable。`co_await`是一个新的运算符。Awaitable主要有3个函数：
1. `await_ready`：返回Awaitable实例是否已经ready。协程开始会调用此函数，如果返回true，表示你想得到的结果已经得到了，协程不需要执行了。所以大部分情况这个函数的实现是要return false。
2. `await_suspend`：挂起awaitable。该函数会传入一个`coroutine_handle`类型的参数。这是一个由编译器生成的变量。在此函数中调用handle.resume()，就可以恢复协程。
3. `await_resume`：当协程重新运行时，会调用该函数。这个函数的返回值就是`co_await`运算符的返回值。

既然这3个函数都搞清楚了，那我们只要自己实现这3个函数，就可以用`co_await`来等待结果了。`co_await`在协程中使用，但是协程的入口必须是在某个函数中，函数的返回值需要满足Promise的规范。最简单的Promise如下：

```
struct Task
{
	struct promise_type {
		auto get_return_object() { return Task{}; }
		auto initial_suspend() { return std::experimental::suspend_never{}; }
		auto final_suspend() { return std::experimental::suspend_never{}; }
		void unhandled_exception() { std::terminate(); }
		void return_void() {}
	};
};
```

下面来举个例子，在例子中其实这个Task除了作为函数返回值以外没其它作用。

以一个add函数为例：`int add100(int a)`，就是将参数a加上100，假设这个add非常耗时，把它设计成一个异步的调用，异步调用的结果当然是通过回调函数进行通知，定义如下：

```
using call_back = std::function<void(int)>;
void Add100ByCallback(int init, call_back f) //init是传入的初始值，add之后的结果由回调函数f通知
{
	std::thread t([init, f]() {
		std::this_thread::sleep_for(std::chrono::seconds(5)); // sleep一下，假装很耗时
		f(init + 100); // 耗时的计算完成了，调用回调函数
	});
	t.detach();
}
```

这个函数的调用是大家都熟悉的异步函数调用，提供一个回调函数作为参数就可以了，比如直接将得到的结果输出：

```
Add100ByCallback(5, [](int value){ std::cout<<"get result: "<<value<<"\n"; });
```

现在我们把通过回调函数的异步调用改成协程，协程和普通的回调函数不同，可以直接得到add之后的结果。普通的回调函数是不可以有返回值的。协程如下：

```
Task Add100ByCoroutine(int init, call_back f)
{
	int ret = co_await Add00AWaitable(init); //co_await可以有返回值
	f(ret);
}
```

根据前面提到的3个函数，来实现Add00AWaitable类型：

```
struct Add00AWaitable
{
	Add00AWaitable(int init):init_(init) {}
	bool await_ready() const { return false; }
	int await_resume() { return result_; }
	void await_suspend(std::experimental::coroutine_handle<> handle)
	{
	    // 定义一个回调函数，在此函数中恢复协程
		auto f = [handle, this](int value) mutable {
			result_ = value;
			handle.resume(); // 这句是关键
		};
		Add100ByCallback(init_, f); // 这个函数是之前直接采用异步可用的函数
	}
	int init_; // 将参数存在这里
	int result_; // 将返回值存在这里
};
```

启动一个协程也很简单，直接调用Add100ByCoroutine即可：

```
Add100ByCoroutine(10, [](int value){ std::cout<<"get result from coroutine: "<<value<<"\n"; });
```

按照这个思路，可以将任意异步调用轻松改造成协程，比如上面的例子是将原本的Add100ByCallback函数改成了协程。

按照例子依葫芦画瓢，然后再结合网上那些似懂非懂的文档，各位c++er都可以入门了。但是C++协程的知识非常之多，如何实在需要的时候提供15个函数中的其它函数、如何高效、如何避免不必要的加锁，可以说是高手发挥的舞台。关于协程的进一步学习可以看[这里（共有3篇）](https://lewissbaker.github.io/)。

附[本文例子源码](https://github.com/franktea/temp/blob/master/uncategorized/co_vs_callback.cpp)。