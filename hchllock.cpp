#include <bits/stdc++.h>
#include <atomic>
#include <thread>
using namespace std;
class QNodes{
    private:
     static const int TWS_MASK = 0x80000000;
     static const int SMW_MASK = 0x40000000;
     static const int CLUSTER_MASK = 0x3FFFFFFF;
     atomic<int> state;
    public:
     QNodes(){
         state.store(0);
     }
     void unlock()
     {
        int newState,oldState=0;
        newState; // A way to get current cluster id and then set it to newState
        newState|=SMW_MASK;
        newState&=(~TWS_MASK);
        do{
            oldState = state.load();
        }while(!state.compare_exchange_weak(oldState,newState));
        
     }
     bool waitForGrantOrClusterMaster()
     {
       // have to work;
     }
     int getClusterId(){
       return (state.load())&(CLUSTER_MASK);
     }
     void setClusterId(int id){
       int newState,oldState=0;
        newState=id;
        do{
            oldState = state.load();
            newState|= ((oldState)&(~CLUSTER_MASK));
        }while(!state.compare_exchange_weak(oldState,newState));
     }
     bool isSuccessorMustWait(){
        return ((state.load())&SMW_MASK)>>30;
     }
     void setSuccessorMustWait(bool flag){
        int newState,oldState=0;
        do{
            oldState = state.load();
            newState= oldState;
            newState= flag?(newState|SMW_MASK):(newState&(0xbfffffff));
        }while(!state.compare_exchange_weak(oldState,newState));
     }
     bool isTailWhenSpliced(){
         return state.load()>>31;
     }
     int setTailWhenSpliced(bool flag){
         int newState,oldState=0;
        do{
            oldState = state.load();
            newState= oldState;
            newState=newState= flag?(newState|TWS_MASK):(newState&(0x7fffffff));;
        }while(!state.compare_exchange_weak(oldState,newState));
     }
    
};
class HCLHLock
{
     static const int CLUSTER_MAX = 20;
     static thread_local QNodes* currNode;
     static thread_local QNodes* preNode;
     vector<atomic<QNodes*>>localQueues;
     atomic<QNodes*>globalQueue;
     public:
     HCLHLock(){
         localQueues.resize(CLUSTER_MAX);
         for(int i=0;i<CLUSTER_MAX;i++){
             localQueues[i].store(new QNodes());
         }
         QNodes* head = new QNodes();
         globalQueue.store(head);
     }
     void lock()
     { 
        QNodes* myNode = currNode;
        int id;// set id equals current cluster id
        QNodes* myPred = NULL;
        do
        {
          myPred = localQueues[id].load();
        }while(!localQueues[id].compare_exchange_weak(myPred,myNode));
        if(myPred!=NULL)
        {
           bool iOwnLock = myPred->waitForGrantOrClusterMaster();
           if(iOwnLock)
           {
               preNode = myPred;
               return;
           }
        }
        QNodes* localTail = NULL;
        do{
         myPred = globalQueue.load();
         localTail = localQueues[id].load();
        }while(!globalQueue.compare_exchange_weak(myPred,localTail));
        localTail->setTailWhenSpliced(true);
        while(myPred->isSuccessorMustWait()){};
        preNode = myPred;
        return;
     }
     void unlock()
     {
       QNodes* myNode = currNode;
       myNode->setSuccessorMustWait(false);
       QNodes* pred = preNode;
       pred->unlock();
       currNode = pred;
     }
};
thread_local QNodes* HCLHLock::currNode = new QNodes();
thread_local QNodes* HCLHLock::preNode = NULL;