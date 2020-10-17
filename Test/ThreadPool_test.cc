#include "Pools/ThreadPool.h"
#include <unistd.h>
#include <iostream>
using namespace std;

void func(int idx) {
  cout << this_thread::get_id() << ": 第 " << idx << " 个任务\n";
  sleep(1);
  cout << "任务 " << idx << " 结束\n";
}

int main(void) {
  ThreadPool* t_pool = ThreadPool::GetInstance();
  t_pool->start(10, 2);
  cout << "Before : " << t_pool->getTaskSize() << endl;
  int i = 1;
  for (; i <= 5; ++i) {
    t_pool->AddTask(bind(func, i));
  }
  cout << "First test : " << t_pool->getTaskSize() << endl;
  sleep(1);
  for (; i <= 25; ++i) {
    t_pool->AddTask(bind(func, i));
  }
  cout << "Second test : " << t_pool->getTaskSize() << endl;
  sleep(1);
  for (; i <= 55; ++i) {
    t_pool->AddTask(bind(func, i));
  }
  cout << "Thrid test : " << t_pool->getTaskSize() << endl;
  sleep(1);
  cout << "Remain Tasks : " << t_pool->getTaskSize() << endl;
  sleep(2);
  cout << "Remain Tasks : " << t_pool->getTaskSize() << endl;
  t_pool->stop();
  return 0;
}