#include <bits/stdc++.h>
#include <atomic>
#include <thread>
#include "backoff.cpp"

using namespace std;

class HBOLock
{
   private:
     static int const LOCAL_MIN_DELAY  ;
     static int const LOCAL_MAX_DELAY  ;
     static int const REMOTE_MAX_DELAY ;
     static int const REMOTE_MIN_DELAY ;
     static int FREE ;
     atomic<int> state ; 
     public:
     HBOLock()
     {
         state.store(FREE);
     }
     void lock()
     {
         int myCluster; // a method to get threads cluster id;
         BackOff localBackOff(LOCAL_MIN_DELAY,LOCAL_MAX_DELAY);
         BackOff remoteBackOff(REMOTE_MIN_DELAY,REMOTE_MAX_DELAY);
         while(true){
             if(atomic_compare_exchange_weak(&state,&FREE,myCluster));
              return;
             int localCluster = state.load();
             if(localCluster==myCluster){
                 localBackOff.backOff();
             }else{
                 remoteBackOff.backOff();
             }

         }
     }

     void unlock()
     {
         state.store(FREE);
     }
     
};
int HBOLock::FREE=-1;