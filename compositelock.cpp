#include <bits/stdc++.h>
#include <thread>
#include <atomic>
#include <chrono>
#include "backoff.cpp"

using namespace std;
enum State{FREE,RELEASED,WAITING,ABORTED};
class QNodes{
   public:
   atomic<int> state;
   QNodes* pred;
   public:
   QNodes(){
       state.store(FREE);
   }
};
class CompositeLock{
    public:
    vector <QNodes*> waiting;
    static thread_local QNodes* myNode;
    atomic<QNodes*> tail;

    /* 
      Set SIZE,MIN_DELAY,MAX_DELAY
      before executing
    */
     static const int SIZE=100000;
     static const int MIN_DELAY=2;
     static const int MAX_DELAY=20;
     public:
      QNodes* acquireQNode(BackOff *backoff,std::chrono::high_resolution_clock::time_point startTime,uint64_t patience)
      {
         QNodes* node = waiting[rand()%SIZE];
         QNodes* currtail=NULL;
         
         int s = 0;
         while(true){
             s = 0;
            if(node->state.compare_exchange_weak(s,2)){
               return node;
            }
            currtail = tail.load();
            int state = node->state.load();
           
            if(state==3||state==1){
                 QNodes* myPred = NULL;
                 if(state==3){
                     myPred = (node->pred);
                 }
                 if(tail.compare_exchange_weak(
                    currtail,
                    myPred
                    )){
                      node->state.store(2);
                      return node;
                 }
                 
                 
            }
            backoff->backOff();
            if((std::chrono::high_resolution_clock::now()-startTime).count()>patience*1e6){
                throw std::runtime_error("Timeout");
            } 

        }
    }
    QNodes* spliceQNodes(QNodes* &node,std::chrono::high_resolution_clock::time_point startTime,uint64_t patience)
    {
        QNodes* currtail=NULL;
        do{
            currtail = tail.load();
            if((std::chrono::high_resolution_clock::now()-startTime).count()>patience*1e6){
                node->state.store(FREE);
                throw std::runtime_error("Timeout");
            } 

        }while(!tail.compare_exchange_weak(
               currtail,
               node));
        return currtail;
    }
    void waitForPredecessor(QNodes* &node,QNodes* &pred,std::chrono::high_resolution_clock::time_point startTime,uint64_t patience)
    {
      
      if(pred ==NULL){
          myNode = (node);
          return;
      }
      int predState = pred->state.load();
      while(predState!=1)
      {
        if(predState==3)
        {
          QNodes* temp = pred;
          pred = (pred->pred);
          temp->state.store(0);
        }
        if((std::chrono::high_resolution_clock::now()-startTime).count()>patience*1e6)
        {
                node->pred = pred;
                node->state.store(ABORTED);
                throw std::runtime_error("Timeout");
        } 
        predState = pred->state.load();
      }

      pred->state.store(0);
      myNode = node;
      return;
    }     
    public:
     CompositeLock(){
         waiting.resize(SIZE);
         tail.store(NULL);
         for(int i=0;i<SIZE;i++){
             waiting[i] = (new QNodes());
         }
     }
     bool trylock(uint64_t patience=4)
     {
        std::chrono::high_resolution_clock::time_point start_point= std::chrono::high_resolution_clock::now();
        BackOff *backoff= new BackOff(MIN_DELAY,MAX_DELAY);
        try
        {
            QNodes* qnode = acquireQNode(backoff,start_point,patience);
            QNodes* pred = spliceQNodes(qnode,start_point,patience);
            waitForPredecessor(qnode,pred,start_point,patience);
             return true;
        }catch(std::runtime_error e){
            return false;
        } 
     }
     void unlock()
     {
          myNode->state.store(1);
          myNode = NULL;
          
    } 

};
thread_local QNodes* CompositeLock::myNode = NULL;