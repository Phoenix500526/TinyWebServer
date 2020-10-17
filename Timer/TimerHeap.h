#ifndef TINYWEBSERVER_TIMER_TIMERHEAP_H
#define TINYWEBSERVER_TIMER_TIMERHEAP_H

#include <assert.h>
#include <chrono>
#include <functional>
#include <unordered_map>
#include <vector>

using TimeoutCallBack = std::function<void()>;
using Clock = std::chrono::high_resolution_clock;
using MS = std::chrono::milliseconds;
using TimeStamp = Clock::time_point;

struct TimerNode {
  int timer_id;
  TimeStamp expires;
  TimeoutCallBack cb;
  TimerNode(int id, TimeStamp timeout, TimeoutCallBack const& callback)
      : timer_id(id), expires(timeout), cb(callback) {}
  bool operator<(const TimerNode& rhs) { return expires < rhs.expires; }
};

class TimerHeap {
 protected:
  std::vector<TimerNode> m_heap;
  // m_ref 保存定时器 id 与其在最小堆中的下标
  std::unordered_map<int, size_t> m_ref;
  void Swap(size_t i, size_t j);
  void ShiftUp(size_t i);
  bool ShiftDown(size_t i);
  void DelTimer(size_t i);

 public:
  TimerHeap() { m_heap.reserve(64); }
  ~TimerHeap() { Clear(); }
  void Clear();
  void AddTimer(int timer_id, int timeout, TimeoutCallBack const& cb);
  void AdjustTimer(int timer_id, int newExpires);
  void CallBack(int timer_id);
  void Tick();
  void Pop();
  int GetNextTick();
};

#endif