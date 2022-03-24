#pragma once

#include <atomic>

namespace locks {
  using namespace std;
class TTAS 
{
    atomic<bool> flag_state = ATOMIC_FLAG_INIT;

  public:
    void lock()
    {
      
      while (true)
      {
        while (flag_state.load(memory_order_acquire))
        {
            //    busy wait .... do nothing   //
        };

        /*
            We wait till the above while loop returns.
            This would be when the lock is free and flag_state would be false.
            Then we try to acquire the lock.
        */

        if (!flag_state.exchange(true, memory_order_acquire))
        {
          return;
        }

      }
    }

    void unlock()
    {
      flag_state.store(false, memory_order_release);
    }
};
    

}    // namespace locks
