#include <bits/stdc++.h>
#include <atomic>


using namespace std;
namespace locks{
   class TAS{
  atomic_flag state = ATOMIC_FLAG_INIT;
  public:
   void lock(){
      while(state.test_and_set(memory_order_acquire)){this_thread::sleep_for(std::chrono::nanoseconds(200));}
   }
   void unlock(){
       state.clear(memory_order_release);
   }
};

}