#ifndef TINYWEBSERVER_POOLS_THREADPOOL_H
#define TINYWEBSERVER_POOLS_THREADPOOL_H

#include <list>
#include <vector>
#include <thread>
#include <atomic>
#include <utility>
#include "Tools/Mutex.h"
#include "Tools/Condition.h"


template<typename T>
class SyncQueue{
private:
	std::list<T> m_queue;
	mutable Mutex m_mutex;
	Condition m_notEmpty GUARDED_BY(m_mutex);
	Condition m_notFull GUARDED_BY(m_mutex);
	size_t m_maxSize;
	std::atomic<bool> m_needStop;
private:
	template <typename F>
	void Add(F&& x){
		UniqueLock lck(m_mutex);
		m_notFull.wait(lck, [this]{return m_needStop || m_queue.size() < m_maxSize;});
		if(m_needStop.load())
			return;
		m_queue.push_back(std::forward<F>(x));
		m_notEmpty.notify_one();
	}
public:
	SyncQueue(): m_needStop(false){}
	void setMaxSize(int const maxSize){
		UniqueLock lck(m_mutex);
		m_maxSize = maxSize;
	}
	bool full() const{
		UniqueLock lck(m_mutex);
		return m_queue.size() >= m_maxSize;
	}
	bool empty() const{
		UniqueLock lck(m_mutex);
		return m_queue.empty();
	}
	void stop() NO_THREAD_SAFETY_ANALYSIS{
		m_needStop.store(true);
		m_notFull.notify_all();
		m_notEmpty.notify_all();
	}
	void put(T&& x){
		Add(std::forward<T>(x));
	}
	void put(const T& x){
		Add(x);
	}
	void take(T& t){
		UniqueLock lck(m_mutex);
		m_notEmpty.wait(lck, [this]{return m_needStop || !m_queue.empty();});
		if(m_needStop.load())
			return;
		t = m_queue.front();
		m_queue.pop_front();
		m_notFull.notify_one();
	}
	size_t size(){
		UniqueLock lck(m_mutex);
		return m_queue.size();
	}
	size_t capacity() const{
		return m_maxSize;
	}
	size_t remain(){
		UniqueLock lck(m_mutex);
		return m_maxSize - m_queue.size();
	}
};


class ThreadPool{
private:
	using Task = std::function<void()>;
	std::vector<std::shared_ptr<std::thread>> m_pool;
	SyncQueue<Task> m_workqueue;
	int m_threadNum;
	std::once_flag m_flag;
	std::atomic<bool> m_running;
public:
	static ThreadPool* GetInstance(){
		static ThreadPool pool;
		return &pool;
	}
	void AddTask(const Task& task){
		m_workqueue.put(task);
	}
	void AddTask(Task&& task){
		m_workqueue.put(std::forward<Task>(task));
	}

	size_t getTaskSize(){
		return m_workqueue.size();
	}

	void start(int threadNum = std::thread::hardware_concurrency(), int maxSize = 10000){
		m_workqueue.setMaxSize(maxSize);
		m_threadNum = threadNum;
		//IO 密集型线程池
		m_pool.reserve(m_threadNum * 2 + 1);
		m_running.store(true);
		for(int i = 0;i < m_threadNum * 2 + 1;++i){
			m_pool.emplace_back(std::make_shared<std::thread>(&ThreadPool::run, this));
		}
	}
	void stop(){
		m_workqueue.stop();
		m_running.store(false);
		for(auto Thread: m_pool){
			if(Thread)
				Thread->join();
		}
		m_pool.clear();
	}
private:
	ThreadPool(){}
	~ThreadPool(){
		std::call_once(m_flag, [this]{stop();});
	}
	void run(){
		while(m_running){
			Task task;
			m_workqueue.take(task);
			if(m_running)
				task();
			/*
			connectionRAII mysqlcon(&task->mysql, m_connPool);
            task->process();
            */
		}
	}
};

#endif