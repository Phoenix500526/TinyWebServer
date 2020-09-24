#include <gtest/gtest.h>
#include "Timer/TimerHeap.h"

using namespace std;

void func(){}

class TimerHeap_Derived: public TimerHeap
{
public:
    TimerHeap_Derived(){}
    ~TimerHeap_Derived(){}
    vector<TimerNode>& getHeap(){
        return TimerHeap::m_heap;
    }
    unordered_map<int, size_t>& getRef(){
        return TimerHeap::m_ref;
    }
    void Swap(size_t i, size_t j){
        TimerHeap::Swap(i, j);
    }
    void ShiftUp(size_t i){
        TimerHeap::ShiftUp(i);
    }
    bool ShiftDown(size_t i){
        return TimerHeap::ShiftDown(i);
    }
    void DelTimer(int timer_id){
        TimerHeap::DelTimer(timer_id);
    }
    void AddTimer(int timer_id, int timeout, TimeoutCallBack const& cb){
        TimerHeap::AddTimer(timer_id, timeout, cb);
    }
    void AdjustTimer(int timer_id, int timeout){
        TimerHeap::AdjustTimer(timer_id, timeout);
    }
    void Tick(){
        TimerHeap::Tick();
    }
};


class TimerHeap_Unittest:public ::testing::Test
{
protected:
    TimerHeap_Unittest()
        :t0(0, Clock::now() + static_cast<MS>(5000000), func), 
        t1(1, Clock::now() + static_cast<MS>(2000000), func),
        t2(2, Clock::now() + static_cast<MS>(1000000), func), 
        t3(3, Clock::now() + static_cast<MS>(4000000), func),
        t4(4, Clock::now(), func), 
        t5(5, Clock::now() + static_cast<MS>(3000000), func){
            vector<TimerNode>& heap = Heap_.getHeap();
            unordered_map<int, size_t>& ref = Heap_.getRef();
            heap.push_back(t0);
            ref[0] = 0;
            heap.push_back(t1);
            ref[1] = 1;
            heap.push_back(t2);
            ref[2] = 2;
            heap.push_back(t3);
            ref[3] = 3;
            heap.push_back(t4);
            ref[4] = 4;
            heap.push_back(t5);
            ref[5] = 5;
        }
    ~TimerHeap_Unittest(){}
    TimerHeap_Derived Heap_;
    TimerNode t0;
    TimerNode t1;
    TimerNode t2;
    TimerNode t3;
    TimerNode t4;
    TimerNode t5;
};


TEST_F(TimerHeap_Unittest, InitTest){
    EXPECT_EQ(t1 < t0, true);
    EXPECT_EQ(t2 < t1, true);
    EXPECT_EQ(t3 < t0, true);
    EXPECT_EQ(t4 < t2, true);
    EXPECT_EQ(t5 < t3, true);
}

//Swap(t4,t5)
TEST_F(TimerHeap_Unittest, SwapTest){
    unordered_map<int, size_t>& ref = Heap_.getRef();
    vector<TimerNode>& heap = Heap_.getHeap();
    EXPECT_EQ(ref[4], 4);
    EXPECT_EQ(ref[5], 5);
    EXPECT_EQ(heap[4].expires, t4.expires);
    EXPECT_EQ(heap[5].expires, t5.expires);
    Heap_.Swap(4, 5);
    EXPECT_EQ(ref[4], 5);
    EXPECT_EQ(ref[5], 4);
    EXPECT_EQ(heap[4].expires, t5.expires);
    EXPECT_EQ(heap[5].expires, t4.expires);
}

//ShiftUp(4)
TEST_F(TimerHeap_Unittest, ShiftUpTest){
    unordered_map<int, size_t>& ref = Heap_.getRef();
    vector<TimerNode>& heap = Heap_.getHeap();
    EXPECT_EQ(ref[4], 4);
    EXPECT_EQ(ref[0], 0);
    EXPECT_EQ(ref[1], 1);
    EXPECT_EQ(heap[0].expires, t0.expires);
    EXPECT_EQ(heap[1].expires, t1.expires);
    EXPECT_EQ(heap[4].expires, t4.expires);
    Heap_.ShiftUp(4);
    EXPECT_EQ(ref[4], 0);
    EXPECT_EQ(ref[2], 2);
    EXPECT_EQ(ref[3], 3);
    EXPECT_EQ(ref[1], 4);
    EXPECT_EQ(ref[0], 1);
    EXPECT_EQ(ref[5], 5);
    EXPECT_EQ(heap[0].expires, t4.expires);
    EXPECT_EQ(heap[1].expires, t0.expires);
    EXPECT_EQ(heap[4].expires, t1.expires);
    EXPECT_EQ(heap[2].expires, t2.expires);
    EXPECT_EQ(heap[3].expires, t3.expires);
    EXPECT_EQ(heap[5].expires, t5.expires);
}

//ShiftDown(0)
TEST_F(TimerHeap_Unittest, ShiftDownTest){
    unordered_map<int, size_t>& ref = Heap_.getRef();
    vector<TimerNode>& heap = Heap_.getHeap();
    EXPECT_EQ(ref[0], 0);
    EXPECT_EQ(ref[2], 2);
    EXPECT_EQ(ref[5], 5);
    EXPECT_EQ(heap[0].expires, t0.expires);
    EXPECT_EQ(heap[2].expires, t2.expires);
    EXPECT_EQ(heap[5].expires, t5.expires);
    EXPECT_EQ(Heap_.ShiftDown(0), true);
    EXPECT_EQ(ref[0], 5);
    EXPECT_EQ(ref[1], 1);
    EXPECT_EQ(ref[2], 0);
    EXPECT_EQ(ref[3], 3);
    EXPECT_EQ(ref[4], 4);
    EXPECT_EQ(ref[5], 2);
    EXPECT_EQ(heap[0].expires, t2.expires);
    EXPECT_EQ(heap[2].expires, t5.expires);
    EXPECT_EQ(heap[5].expires, t0.expires);
}

//DelTimer(0)
TEST_F(TimerHeap_Unittest, DelTimerTest){
    unordered_map<int, size_t>& ref = Heap_.getRef();
    vector<TimerNode>& heap = Heap_.getHeap();
    Heap_.DelTimer(5);
    EXPECT_EQ(ref[0],0);
    EXPECT_EQ(heap[0].expires, t0.expires);
    EXPECT_EQ(ref[1],1);
    EXPECT_EQ(heap[1].expires, t1.expires);
    EXPECT_EQ(ref[2],2);
    EXPECT_EQ(heap[2].expires, t2.expires);
    EXPECT_EQ(ref[3],3);
    EXPECT_EQ(heap[3].expires, t3.expires);
    EXPECT_EQ(ref[4],4);
    EXPECT_EQ(heap[4].expires, t4.expires);
    EXPECT_EQ(heap.size(), 5);
    EXPECT_EQ(ref.find(5) == ref.end(), true);

    Heap_.DelTimer(0);
    EXPECT_EQ(heap.size(), 4);
    EXPECT_EQ(ref.find(0) == ref.end(), true);
    EXPECT_EQ(ref[4], 0);
    EXPECT_EQ(heap[0].expires, t4.expires);
    EXPECT_EQ(ref[1], 1);
    EXPECT_EQ(heap[1].expires, t1.expires);
    EXPECT_EQ(ref[2], 2);
    EXPECT_EQ(heap[2].expires, t2.expires);
    EXPECT_EQ(ref[3], 3);
    EXPECT_EQ(heap[3].expires, t3.expires);
}


TEST_F(TimerHeap_Unittest, AddTimerTest){
    unordered_map<int, size_t>& ref = Heap_.getRef();
    vector<TimerNode>& heap = Heap_.getHeap();
    Heap_.AddTimer(6, 1500, func);
    EXPECT_EQ(ref[0], 2);
    EXPECT_EQ(heap[2].expires, t0.expires);
    EXPECT_EQ(ref[1], 1);
    EXPECT_EQ(ref[2], 6);
    EXPECT_EQ(heap[6].expires, t2.expires);
    EXPECT_EQ(ref[3], 3);
    EXPECT_EQ(ref[4], 4);
    EXPECT_EQ(ref[5], 5);
    EXPECT_EQ(ref[6], 0);
}

//AdjustTimer(4)
TEST_F(TimerHeap_Unittest, AdjustTimerTest){
    unordered_map<int, size_t>& ref = Heap_.getRef();
    vector<TimerNode>& heap = Heap_.getHeap();
    Heap_.AdjustTimer(4, 1500);
    EXPECT_EQ(ref[0],1);
    EXPECT_EQ(heap[1].expires, t0.expires);
    EXPECT_EQ(ref[1],4);
    EXPECT_EQ(heap[4].expires, t1.expires);
    EXPECT_EQ(ref[2],2);
    EXPECT_EQ(ref[3],3);
    EXPECT_EQ(ref[4],0);
    EXPECT_EQ(ref[5],5);
}

TEST_F(TimerHeap_Unittest, TickTest){
    unordered_map<int, size_t>& ref = Heap_.getRef();
    vector<TimerNode>& heap = Heap_.getHeap();
    Heap_.Tick();
    EXPECT_EQ(ref[0],0);
    EXPECT_EQ(heap[0].expires, t0.expires);
    EXPECT_EQ(ref[1],1);
    EXPECT_EQ(heap[1].expires, t1.expires);
    EXPECT_EQ(ref[2],2);
    EXPECT_EQ(heap[2].expires, t2.expires);
    EXPECT_EQ(ref[3],3);
    EXPECT_EQ(heap[3].expires, t3.expires);
    EXPECT_EQ(ref[4],4);
    EXPECT_EQ(heap[4].expires, t4.expires);
    EXPECT_EQ(ref[5],5);
    EXPECT_EQ(heap[5].expires, t5.expires);
    Heap_.ShiftUp(4);
    Heap_.Tick();
    EXPECT_EQ(ref[0], 1);
    EXPECT_EQ(heap[1].expires, t0.expires);
    EXPECT_EQ(ref[1], 4);
    EXPECT_EQ(heap[4].expires, t1.expires);
    EXPECT_EQ(ref[2], 0);
    EXPECT_EQ(heap[0].expires, t2.expires);
    EXPECT_EQ(ref[3], 3);
    EXPECT_EQ(ref.find(4) == ref.end(), true);
    EXPECT_EQ(heap.size(), 5);
}