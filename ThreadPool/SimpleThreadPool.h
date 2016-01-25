
#ifndef SIMPLE_THREAD_POOL
#define SIMPLE_THREAD_POOL

#include <thread>
#include <chrono>
#include <future>

#include <mutex>
#include <atomic>
#include <condition_variable>

#include <deque>
#include <vector>

#include <tuple>
#include <utility>

#include <iostream>
#include <string>
#include <complex.h>


template<typename ReturnType, class CallAble, typename... ArgsTypes>
class SimpleThreadPool
{
public:
	SimpleThreadPool();
	explicit SimpleThreadPool(size_t maxthreadcount);
	SimpleThreadPool(const SimpleThreadPool& o) = delete;
	~SimpleThreadPool();
	SimpleThreadPool& operator=(const SimpleThreadPool& o) = delete;

	void AddThread(size_t num = 1);

	std::future<ReturnType> async(CallAble&& fn, ArgsTypes... args);

	//template<typename ReturnType,typename Callable, typename... CallValues>
	//std::future<ReturnType> async(Callable&& callable, CallValues&&... args);
private:
	size_t maxthreadcount;
	std::atomic_bool Stop;

	std::deque<
		std::pair<
			std::packaged_task<ReturnType(ArgsTypes...)>,
			std::tuple<ArgsTypes...>
		>
	>	TaskTodoList;
	std::mutex mtxTaskTodoList;
	std::condition_variable cvarTaskTodoList;

	std::deque<std::thread> ThreadPool;
	std::mutex mtxThreadPool;
public:

	template<typename F,typename T,std::size_t... S>
	void apply(F &f, const T& t, std::index_sequence<S...>)
	{
		f(std::get<S>(t)...);
	}

	template<typename F, typename T>
	void apply(F &f, const T& t)
	{
		apply(f, t, std::make_index_sequence<std::tuple_size<T>::value>());
	}



private:
	void threadfn();
};

template<typename ReturnType, class CallAble, typename... ArgsTypes>
SimpleThreadPool<ReturnType, CallAble, ArgsTypes...>
::SimpleThreadPool()
	:SimpleThreadPool(5)
{}

template<typename ReturnType, class CallAble, typename... ArgsTypes>
SimpleThreadPool<ReturnType, CallAble, ArgsTypes...>
::SimpleThreadPool(size_t maxthreadcount)
	: maxthreadcount(maxthreadcount),
	Stop(false)
{
	for (size_t i = 0; i != maxthreadcount; ++i)
	{
		this->ThreadPool.push_back(
			std::thread(
					&SimpleThreadPool<ReturnType, CallAble, ArgsTypes...>::threadfn,
					this
				)
			);
	}
}

template<typename ReturnType, class CallAble, typename... ArgsTypes>
SimpleThreadPool<ReturnType, CallAble, ArgsTypes...>
::~SimpleThreadPool()
{
	{
		std::unique_lock<std::mutex> u(this->mtxTaskTodoList);
		this->Stop.store(true);
		this->cvarTaskTodoList.notify_all();
	}
	{
		std::lock_guard<std::mutex> g(this->mtxThreadPool);
		for(auto &a: this->ThreadPool)
		{
			if(a.joinable())
			{
				a.join();
			}
		}
	}
}

template<typename ReturnType, class CallAble, typename... ArgsTypes>
void 
SimpleThreadPool<ReturnType, CallAble, ArgsTypes...>
::AddThread(size_t num)
{
	std::lock_guard<std::mutex> g(mtxThreadPool);
	for (size_t i = 0; i != num; ++i)
	{
		++maxthreadcount;
		this->ThreadPool.push_back(
			std::thread(
				&SimpleThreadPool<ReturnType, CallAble, ArgsTypes...>::threadfn,
				this
				)
			);
	}
}

template <typename ReturnType, class CallAble, typename ... ArgsTypes>
std::future<ReturnType> SimpleThreadPool<ReturnType, CallAble, ArgsTypes...>
::async(CallAble&& fn, ArgsTypes... args)
{
	std::packaged_task<ReturnType(ArgsTypes...)> pt(fn);
	auto fut = pt.get_future();
	std::unique_lock<std::mutex> u(this->mtxTaskTodoList);
	this->TaskTodoList.push_back(std::make_pair(std::move(pt), std::make_tuple(args...)));
	this->cvarTaskTodoList.notify_one();
	return std::move(fut);
}

template<typename ReturnType, class CallAble, typename... ArgsTypes>
void SimpleThreadPool<ReturnType, CallAble, ArgsTypes...>
::threadfn()
{
	while(!this->Stop.load())
	{
		std::pair<
			std::packaged_task<ReturnType(ArgsTypes...)>,
			std::tuple<ArgsTypes...>
		>	now;
		{
			if (this->Stop.load())
			{
				return;
			}
			std::unique_lock<std::mutex> u(this->mtxTaskTodoList);
			while (this->TaskTodoList.empty())
			{
				this->cvarTaskTodoList.wait(u);
				if (this->Stop.load())
				{
					return;
				}
			}
			if (this->Stop.load())
			{
				return;
			}
			now = std::move(this->TaskTodoList.front());
			this->TaskTodoList.pop_front();
		}
		this->apply(now.first, now.second);
	}
}

#endif	// SIMPLE_THREAD_POOL
