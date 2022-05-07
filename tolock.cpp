#include<bits/stdc++.h>
#include <thread>
#include <atomic>
#include <chrono>
using namespace std;
using namespace std::chrono;
class QNodes{
 public:
  QNodes * pred{NULL};
  QNodes() = default;
};

class TOlock{
 atomic<QNodes*> tail;
 static thread_local QNodes* myNode;
 static QNodes* AVAILABLE;
 public:
 TOlock(){
   tail = NULL;
   myNode = new QNodes();
 }
 bool trylock(milliseconds time){
   auto start_point = system_clock::now();
   duration<long long,milli> patience(time);
   
   QNodes* qnode = new QNodes();
   myNode = qnode;
   qnode->pred = NULL;
   QNodes* myPred = atomic_exchange(&tail,qnode);
   if(myPred == NULL|| (myPred->pred) == AVAILABLE){
     return true;
   }
   while(system_clock::now()-start_point<patience){
     QNodes* pred = myPred->pred;
     if(pred==AVAILABLE){
       return true;
     }else if(pred!=NULL){
        myPred = pred;
     }
   }
   if(!tail.compare_exchange_strong(qnode,NULL)){
       myNode->pred = myPred;
   }

   return false;
 }

 void unlock(){
   QNodes* qnode = myNode;
   if(!tail.compare_exchange_strong(qnode,NULL)){
     qnode->pred = AVAILABLE;
   }
 }
 
};
thread_local QNodes* TOlock::myNode = new QNodes();
QNodes* TOlock::AVAILABLE = new QNodes();