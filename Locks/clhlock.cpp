#pragma once

#include <atomic>
#include <thread>
#include <memory>
#include <cstdint>
using namespace std;
namespace locks {

    class QNode
{

  public:
    bool locked = false;
    QNode() = default;

};
class CLHlock 
{
  
    atomic<QNode*> tail;

    static thread_local QNode* myNode;  
    // represents the thread that came just before the current thread
    static thread_local QNode* myPred;

  public:

    CLHlock()
    {
      tail.store(new QNode());    //  storing a null QNode initially
    }
    ~CLHlock() 
    {
       delete tail;
    }
      
  
    void lock()
    {
      (myNode)->locked = true;
       myPred = (tail.exchange(myNode, std::memory_order_acquire));
      while (myPred->locked)
      {
        
        this_thread::sleep_for(std::chrono::nanoseconds(200));
      }
    }
  
    void unlock()
    {
      (myNode)->locked = false;
       
    }
  
};

thread_local QNode* CLHlock::myNode = new QNode();
thread_local QNode* CLHlock::myPred = nullptr;
}