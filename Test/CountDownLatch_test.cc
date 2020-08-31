#include "../Tools/CountDownLatch.h"
#include <thread>
#include <vector>
#include <unistd.h>
#include <iostream>
using namespace std;

int main(void){
	vector<thread> pool;
	CountDownLatch latch(10);
	for(int i = 0;i < 10;++i){
		pool.emplace_back([&latch](){
			latch.countDown();
			latch.wait();
			cout << "this thread is " << this_thread::get_id() << '\n';
		});
		if(i & 1){
			cout << "i = " << i << ", sleep 2 s" << '\n';
			sleep(2);
		}
	}
	for(thread& t : pool){
		if(t.joinable())
			t.join();
	}
	return 0;
}
