#pragma once
#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>
using namespace std;
namespace locks {
class QNodes{
  public:
   bool locked{false};
   QNodes* next{NULL};
   QNodes() = default;
};

class MCSlock
{
 atomic<QNodes*>tail{NULL};
 public:
 static thread_local unique_ptr<QNodes> myNode;
 MCSlock()=default;
 void lock(){
   QNodes* pred = tail.exchange(myNode.get(),memory_order_acquire);
   if(pred!=NULL){
     (myNode)->locked = true;
     pred->next = myNode.get();
     while((myNode)->locked){
       this_thread::sleep_for(std::chrono::nanoseconds(200));
     }
   }
 }

 void unlock(){
   if((myNode)->next==NULL){
     QNodes* tmp = myNode.get();
      if(tail.compare_exchange_strong(tmp,nullptr,memory_order_relaxed)){
        return;
      }
      while((myNode)->next==NULL){}
   }
   myNode->next->locked = false;
   myNode->next = NULL;
   
   
 }
};
thread_local unique_ptr<QNodes> MCSlock::myNode  = unique_ptr<QNodes>(new QNodes());

}    // namespace locks
