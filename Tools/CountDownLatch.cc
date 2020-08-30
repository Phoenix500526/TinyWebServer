#include "CountDownLatch.h"

void CountDownLatch::wait(){
	UniqueLock lck(m_mutex);
	while(m_count > 0){
		m_cond.wait(lck.getUniqueLock());
	}
}

void CountDownLatch::countDown(){
	UniqueLock lck(m_mutex);
	--m_count;
	if(m_count == 0){
		m_cond.notify_all();
	}
}

int CountDownLatch::getCount() const{
	UniqueLock lck(m_mutex);
	return m_count;
}