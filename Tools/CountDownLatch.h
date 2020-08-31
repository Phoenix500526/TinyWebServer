#ifndef TINYWEBSERVER_TOOLS_COUNTDOWNLATCH_H
#define TINYWEBSERVER_TOOLS_COUNTDOWNLATCH_H

#include "Mutex.h"
#include <condition_variable>

class CountDownLatch : nocopyable{
private:
	mutable Mutex m_mutex;
	std::condition_variable m_cond;
	int m_count GUARDED_BY(m_mutex);
public:
	explicit CountDownLatch(int count):m_mutex(),m_cond(),m_count(count){}	
	void wait();
	void countDown();
	int getCount() const;
};

#endif
