#include<bits/stdc++.h>
#include<atomic>
using namespace std;
namespace locks{
class TTAS{
  atomic<bool> state = ATOMIC_FLAG_INIT;
  public:
  void lock(){
     while(1){
       while(state.load(memory_order_acquire)){this_thread::sleep_for(std::chrono::nanoseconds(200));};
       if(!state.exchange(true,memory_order_acquire)){
           return;
       }
     }
  }
  void unlock(){
    state.store(false,memory_order_release);
  }
};
}