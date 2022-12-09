#include <bits/stdc++.h>
#include <atomic>
#include "backoff.cpp"
using namespace std;
namespace locks{
class BackOffLock{
 private:
  atomic<bool> state = ATOMIC_FLAG_INIT;
  const static int MAX_DELAY=200,MIN_DELAY=2;
 public:
   void lock(){
       BackOff* backOff = new BackOff(MIN_DELAY,MAX_DELAY);
       while(1){
           while(state.load(memory_order_acquire)){this_thread::sleep_for(std::chrono::nanoseconds(200));}
           if(!state.exchange(true,memory_order_acquire)){
              return;
           }else{
              backOff->backOff();
           }
       }
   }
   void unlock(){
       state.store(false,memory_order_release);
   }
};

}