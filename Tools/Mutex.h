#ifndef TINYWEBSERVER_MUTEX_H
#define TINYWEBSERVER_MUTEX_H

#include <mutex>
#include <thread>
#include <assert.h>
#include "nocopyable.h"

// Enable thread safety attributes only with clang.
// The attributes can be safely erased when compiling with other compilers.
#if defined(__clang__) && (!defined(SWIG))
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   __attribute__((x))
#else
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   // no-op
#endif

#define CAPABILITY(x) \
	  THREAD_ANNOTATION_ATTRIBUTE__(capability(x))

#define SCOPED_CAPABILITY \
	  THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

#define GUARDED_BY(x) \
	  THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

#define PT_GUARDED_BY(x) \
	  THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))

#define ACQUIRED_BEFORE(...) \
	  THREAD_ANNOTATION_ATTRIBUTE__(acquired_before(__VA_ARGS__))

#define ACQUIRED_AFTER(...) \
	  THREAD_ANNOTATION_ATTRIBUTE__(acquired_after(__VA_ARGS__))

#define REQUIRES(...) \
	  THREAD_ANNOTATION_ATTRIBUTE__(requires_capability(__VA_ARGS__))

#define REQUIRES_SHARED(...) \
	  THREAD_ANNOTATION_ATTRIBUTE__(requires_shared_capability(__VA_ARGS__))

#define ACQUIRE(...) \
	  THREAD_ANNOTATION_ATTRIBUTE__(acquire_capability(__VA_ARGS__))

#define ACQUIRE_SHARED(...) \
	  THREAD_ANNOTATION_ATTRIBUTE__(acquire_shared_capability(__VA_ARGS__))

#define RELEASE(...) \
	  THREAD_ANNOTATION_ATTRIBUTE__(release_capability(__VA_ARGS__))

#define RELEASE_SHARED(...) \
	  THREAD_ANNOTATION_ATTRIBUTE__(release_shared_capability(__VA_ARGS__))

#define TRY_ACQUIRE(...) \
	  THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_capability(__VA_ARGS__))

#define TRY_ACQUIRE_SHARED(...) \
	  THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_shared_capability(__VA_ARGS__))

#define EXCLUDES(...) \
	  THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

#define ASSERT_CAPABILITY(x) \
	  THREAD_ANNOTATION_ATTRIBUTE__(assert_capability(x))

#define ASSERT_SHARED_CAPABILITY(x) \
	  THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_capability(x))

#define RETURN_CAPABILITY(x) \
	  THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

#define NO_THREAD_SAFETY_ANALYSIS \
	  THREAD_ANNOTATION_ATTRIBUTE__(no_thread_safety_analysis)

// End of thread safety annotations} 

class CAPABILITY("Mutex") Mutex : nocopyable{
public:
	Mutex():m_mutex(), holder_(){}
	~Mutex(){
		assert(holder_ == std::thread::id());
	}

	void lock() ACQUIRE() {
		m_mutex.lock();
		assignHolder();
	}

	void unlock() RELEASE(){
		unassignHolder();
		m_mutex.unlock();
	}

	bool isLockedByThisThread() const{
		return holder_ == std::this_thread::get_id();
	}

	void assertLocked() ASSERT_CAPABILITY(this){
		assert(isLockedByThisThread());
	}

	std::mutex& getMutex(){
		return m_mutex;
	}

private:
	friend class UniqueLock;

	void assignHolder(){
		holder_ = std::this_thread::get_id();
	}
	void unassignHolder(){
		holder_ = std::thread::id();
	}
	std::mutex m_mutex;
	std::thread::id holder_;
};


class SCOPED_CAPABILITY UniqueLock : nocopyable{
public:
	explicit UniqueLock(Mutex& mutex) ACQUIRE(mutex) :m_mutex(mutex), lck_(mutex.getMutex()){
		m_mutex.assignHolder();
	}
	~UniqueLock() RELEASE(){
		m_mutex.unassignHolder();
	}

	std::unique_lock<std::mutex>& getUniqueLock(){
		return lck_;
	}
private:
	Mutex& m_mutex;
	std::unique_lock<std::mutex> lck_;
};


#define UniqueLock(x) error "Missing guard object name"

#endif
