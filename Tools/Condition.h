#ifndef TINYWEBSERVER_TOOLS_CONDITION_H
#define TINYWEBSERVER_TOOLS_CONDITION_H 

#include "Mutex.h"
#include <chrono>
#include <condition_variable>

class Condition : nocopyable{
private:
	std::condition_variable m_cond;
public:
	Condition():m_cond(){}
	~Condition() = default;
	void notify_all(){
		m_cond.notify_all();
	}
	void notify_one(){
		m_cond.notify_one();
	}
	void wait (UniqueLock& lck){
		m_cond.wait(lck.getUniqueLock());
	}
  template <class Predicate>
	void wait(UniqueLock& lck, Predicate pred){
		m_cond.wait(lck.getUniqueLock(), pred);
	}
	template <class Rep, class Period>
  std::cv_status wait_for (UniqueLock& lck, 
  					const std::chrono::duration<Rep,Period>& rel_time){
  	return m_cond.wait_for(lck.getUniqueLock(), rel_time);
  }
  template <class Rep, class Period, class Predicate>
  bool wait_for (UniqueLock& lck, 
        const std::chrono::duration<Rep,Period>& rel_time, 
   			Predicate pred){
    return m_cond.wait_for(lck.getUniqueLock(), rel_time, pred);
  }
};

#endif