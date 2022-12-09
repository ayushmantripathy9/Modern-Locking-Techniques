#include <iostream>
#include <thread>
#include <chrono>
using namespace std;

class BackOff{
 int minDelay,maxDelay,limit,delay; 
 public:
   BackOff(int min_d,int max_d){
       minDelay = min_d;
       maxDelay = max_d;
       limit = minDelay;
   }
   void backOff(){
       delay = rand()%limit;
       limit = min(maxDelay,2*limit);
       this_thread::sleep_for(std::chrono::nanoseconds(delay));
   }
};