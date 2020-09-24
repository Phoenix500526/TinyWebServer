#include "Timer/TimerHeap.h"
using namespace std;

void TimerHeap::Clear(){
    m_heap.clear();
    m_ref.clear();
}


void TimerHeap::Swap(size_t i, size_t j){
    assert(i >= 0 && i < m_heap.size());
    assert(j >= 0 && j < m_heap.size());
    if(i == j)
        return;
    std::swap(m_heap[i], m_heap[j]);
    m_ref[m_heap[i].timer_id] = i;
    m_ref[m_heap[j].timer_id] = j;
}

void TimerHeap::ShiftUp(size_t i){
    assert(i >= 0 && i < m_heap.size());
    int parent = (i - 1) >> 1;
    while(parent >= 0){
        if(m_heap[parent] < m_heap[i]){
            break;
        }
        Swap(parent, i);
        i = parent;
        parent = (i - 1) >> 1;
    }
}

bool TimerHeap::ShiftDown(size_t i){
    assert(i >= 0 && i < m_heap.size());
    size_t parent = i;
    //拿到左孩子节点
    size_t child = 2 * parent + 1;
    while(child < m_heap.size()){
        if(child + 1 < m_heap.size() && m_heap[child + 1] < m_heap[child]){
            ++child;
        }
        if(m_heap[parent] < m_heap[child])
            break;
        Swap(parent, child);
        parent = child;
        child = parent * 2 + 1;
    }
    return parent > i;
}

void TimerHeap::AddTimer(int timer_id, int timeout, TimeoutCallBack const& cb){
    assert(timer_id >= 0);
    auto iter = m_ref.find(timer_id);
    if(iter == m_ref.end()){
        int i = m_heap.size();
        m_ref[timer_id] = i;
        m_heap.emplace_back(timer_id, Clock::now() + static_cast<MS>(timeout), cb);
        ShiftUp(i);
    }else{
        int i = iter->second;
        m_heap[i].expires = Clock::now() + static_cast<MS>(timeout);
        m_heap[i].cb = cb;
        if(!ShiftDown(i))
            ShiftUp(i);
    }
}


void TimerHeap::DelTimer(size_t i){
    assert(i >= 0 && i < m_heap.size());
    int leaf = m_heap.size() - 1;
    //若为叶子节点则直接删除，非叶子节点则先交换，然后删除，之后再调整。
    if(i == leaf){
        m_ref.erase(m_heap.back().timer_id);
        m_heap.pop_back();
    }else{
        Swap(leaf, i);
        m_ref.erase(m_heap.back().timer_id);
        m_heap.pop_back();
        if(!ShiftDown(i)){
            ShiftUp(i);
        }
    }
}

void TimerHeap::AdjustTimer(int timer_id, int timeout){
    assert(!m_heap.empty());
    auto iter = m_ref.find(timer_id);
    assert(iter != m_ref.end());
    int i = iter->second;
    m_heap[i].expires = Clock::now() + static_cast<MS>(timeout);
    if(!ShiftDown(i)) 
        ShiftUp(i);
}

void TimerHeap::CallBack(int timer_id){
    auto iter = m_ref.find(timer_id);
    if(iter != m_ref.end()){
        auto node = m_heap[iter->second];
        node.cb();
        DelTimer(iter->second);
    }
}

void TimerHeap::Tick(){
    while(!m_heap.empty()){
        auto node = m_heap.front();
        if(std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) { 
            break; 
        }
        node.cb();
        Pop();
    }
}

void TimerHeap::Pop(){
    assert(!m_heap.empty());
    DelTimer(0);
}

int TimerHeap::GetNextTick(){
    Tick();
    size_t res = -1;
    if(!m_heap.empty()) {
        res = std::chrono::duration_cast<MS>(m_heap.front().expires - Clock::now()).count();
        if(res < 0) { res = 0; }
    }
    return res;
}
