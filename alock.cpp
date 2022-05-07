#include <bits/stdc++.h>
#include <atomic>
#include <thread>
using namespace std;
namespace locks{
class Alock
{
   vector<bool>flag;
   static thread_local int myslotIndex;
   atomic<int>tail;
   int size;
   public:
   Alock(int capacity=100000)
   {
       size = capacity;
       tail.store(0);
       flag.resize(capacity,false);
       flag[0] = true;
   }
   void lock()
   {
       int slot = tail.fetch_add(1,memory_order_acquire)%size;
       myslotIndex = slot;
       while(!flag[slot])
       {
         this_thread::sleep_for(std::chrono::nanoseconds(200));
       }
   }
   void unlock()
   {
       int slot = myslotIndex;
       flag[slot]=false;
       flag[(slot+1)%size]=true;
   }

};
thread_local int Alock::myslotIndex = -1;
}