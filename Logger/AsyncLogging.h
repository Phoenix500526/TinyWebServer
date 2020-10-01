#ifndef TINYWEBSERVER_LOGGER_ASYNCLOGGING_H
#define TINYWEBSERVER_LOGGER_ASYNCLOGGING_H

#include "LogStream.h"
#include "Tools/CountDownLatch.h"
#include "Tools/Mutex.h"
#include <thread>
#include <vector>
#include <atomic>
#include <functional>

class AsyncLogging : nocopyable{
private:
	typedef FixedBuffer<kLargeBuffer> Buffer;
	typedef std::unique_ptr<Buffer> BufferPtr;
	typedef std::vector<BufferPtr> BufferPtrVec;
	std::string m_basename;
	off_t m_rollSize;
	const int m_flushInterval;
	std::atomic<bool> m_running;
	CountDownLatch m_latch;
	Mutex m_mutex;
	Condition m_cond;
	std::thread m_thread;
	BufferPtr m_curBuffer GUARDED_BY(m_mutex);
	BufferPtr m_nxtBuffer GUARDED_BY(m_mutex);
	BufferPtrVec m_buffers GUARDED_BY(m_mutex);
	void threadFunc();
public:
	AsyncLogging(std::string const& basename, off_t rollSize = 1024 * 1000 * 1000, int flushInterval = 3);
	~AsyncLogging(){
		if(m_running){
			stop();
		}
	}
	void start(){
		m_running = true;
		//m_thread(this->threadFunc, "logging");
		std::thread asyncThread(std::bind(&AsyncLogging::threadFunc, this));
		m_thread = std::move(asyncThread);
		m_latch.wait();
	}
	void stop() NO_THREAD_SAFETY_ANALYSIS{
		m_running = false;
		m_cond.notify_one();
		m_thread.join();
	}
	void append(const char* logline, size_t len);
};

#endif
