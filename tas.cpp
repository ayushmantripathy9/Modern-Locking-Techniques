#pragma once

#include <iostream>

#include <atomic>

namespace locks{
   using namespace std;
class TAS 
{

   atomic<bool> flag_state = ATOMIC_FLAG_INIT;

public:

   void lock()
   {
      while (flag_state.exchange(true, memory_order_acquire))
      {
         //    busy wait ... do nothing      //
      }
   }

   void unlock()
   {
      flag_state.store(false, memory_order_release);
   }

};

}
