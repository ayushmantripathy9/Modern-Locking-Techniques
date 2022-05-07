#include <bits/stdc++.h>
#include <chrono>
#include <thread>
#include <benchmark/benchmark.h>
#include "timer.cpp"

#include "compositelock.cpp"


using namespace std;

template <typename LockType>
class critical_cases
{
    public:
    void base_case(std::uint64_t grain_size)
    {
        // Do artificial work for grain_size
        timer t;
        while (t.elapsed()< (uint64_t)grain_size*1e5)
        {
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Critical_small does absolutely minimum work in the critical section
    // making it adequate to compare with situation where atomicity is expected
    // from a minor code section.
    void critical_small(std::uint64_t grain_size)
    {
        // Do artificial work for grain_size
        timer t;
        while (t.elapsed()< (uint64_t)grain_size*1e5)
        {
        }

        if(lock.trylock())
        {
            ++counter;
            lock.unlock();
        }
        else
        {
           failed++;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Critical_med does half the work in the critical section making it
    // adequate to compare with parallel graph algorithms where decent chunk of
    //  work is done under locked conditions.
    void critical_med(std::uint64_t grain_size)
    {
        // Do artificial work for grain_size/2
        timer t1;
        while (t1.elapsed()< (uint64_t)(grain_size/2)*1e5)
        {
        }

        if(lock.trylock())
        {
            ++counter;
            // Do artificial work for 50us
            timer t2;
            while (t2.elapsed()< (uint64_t)(grain_size/2)*1e5)
            {
            }
            lock.unlock();
        }
        else
        {
           failed++;
        }

        
    }

    ////////////////////////////////////////////////////////////////////////////
    // Critical_big does all of its work in the critical section making it
    // adequate to compare with lock-based queues/linked-lists where majority of
    //  the code is under locks.
    void critical_big(std::uint64_t grain_size)
    {
        
        if(lock.trylock())
        {
            ++counter;
            // Do artificial work for grain_size
            timer t;
            while (t.elapsed() < grain_size*1e5)
            {
            }
            lock.unlock();
        }
        else
        {
           failed++;
        }

        
    }

private:
    std::uint64_t counter{};
    LockType lock{};
public:
    atomic<int>failed{0};
};

template <typename LockType>
struct params{
  critical_cases<LockType> *cases;
  std::uint64_t grain_size;
};

template <typename LockType>
void* Critical_small(void* arg)
{
    params<LockType>* p = (params<LockType>*)arg;
    ((critical_cases<LockType>*)(p->cases))->critical_small(p->grain_size);
    return NULL;
}

template <typename LockType>
void* Critical_med(void* arg)
{
    params<LockType>* p = (params<LockType>*)arg;
    ((critical_cases<LockType>*)(p->cases))->critical_med(p->grain_size);
    return NULL;
}

template <typename LockType>
void* Critical_big(void* arg)
{
    params<LockType>* p = (params<LockType>*)arg;
    ((critical_cases<LockType>*)(p->cases))->critical_big(p->grain_size);
    return NULL;
}

template <typename LockType>
double critical_small(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    critical_cases<LockType> cases;
    std::vector <pthread_t> temp;
    params<LockType> * p = new params<LockType>();
    p->cases = &cases;
    p->grain_size = grain_size;
    //std::vector<std::thread*> temp;
    for(uint64_t i =0;i<num_tasks;i++)
    {
        pthread_t t;
        pthread_create(&t,NULL,&Critical_small<LockType>,(void*)p);
        temp.push_back(t);
        //temp.push_back(new thread(&critical_cases<LockType>::critical_small,&cases,grain_size)); 
        //cases.critical_small(grain_size);
    }
    for(uint64_t i =0;i<num_tasks;i++)
    {
        pthread_join(temp[i],NULL);
        //temp[i]->join(); 
        //cases.critical_small(grain_size);
    }
    
    return (double)cases.failed;
} 

template <typename LockType>
double critical_med(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    critical_cases<LockType> cases;
    std::vector <pthread_t> temp;
    params<LockType> * p = new params<LockType>();
    p->cases = &cases;
    p->grain_size = grain_size;
    //std::vector<std::thread*> temp;
    for(uint64_t i =0;i<num_tasks;i++)
    {
        pthread_t t;
        pthread_create(&t,NULL,&Critical_med<LockType>,(void*)p);
        temp.push_back(t);
        //temp.push_back(new thread(&critical_cases<LockType>::critical_small,&cases,grain_size)); 
        //cases.critical_small(grain_size);
    }
    for(uint64_t i =0;i<num_tasks;i++)
    {
        pthread_join(temp[i],NULL);
        //temp[i]->join(); 
        //cases.critical_small(grain_size);
    }
    return (double)cases.failed;
   
    
} 

template <typename LockType>
double critical_big(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    critical_cases<LockType> cases;
    std::vector <pthread_t> temp;
    params<LockType> * p = new params<LockType>();
    p->cases = &cases;
    p->grain_size = grain_size;
    //std::vector<std::thread*> temp;
    for(uint64_t i =0;i<num_tasks;i++)
    {
        pthread_t t;
        pthread_create(&t,NULL,&Critical_big<LockType>,(void*)p);
        temp.push_back(t);
        //temp.push_back(new thread(&critical_cases<LockType>::critical_small,&cases,grain_size)); 
        //cases.critical_small(grain_size);
    }
    for(uint64_t i =0;i<num_tasks;i++)
    {
        pthread_join(temp[i],NULL);
        //temp[i]->join(); 
        //cases.critical_small(grain_size);
    }
    
    return (double)cases.failed;
} 


template <typename LockType>
static void BM_Lock_Small(benchmark::State& state) {
  int count = 0;
  double failed = 0.0;
  for (auto _ : state){
     failed += critical_small<LockType>(state.range(0),state.range(0));
     count++;
  }
    
  cout<<"Terminated threads fraction "<< failed/(count*state.range(0))<<endl;
}

template <typename LockType>
static void BM_Lock_Med(benchmark::State& state) {
  int count = 0;
  double failed = 0.0;
  for (auto _ : state){
     failed += critical_med<LockType>(state.range(0),state.range(1));
     count++;
  }
    
  cout<<"Terminated threads fraction "<< failed/(count*state.range(0)) <<endl;
}

template <typename LockType>
static void BM_Lock_Big(benchmark::State& state) {
  int count = 0;
  double failed = 0.0;
  for (auto _ : state){
     failed += critical_big<LockType>(state.range(0),state.range(1));
     count++;
  }
    
  cout<<"Terminated threads fraction "<< failed/(count*state.range(0)) <<endl;
}




// Register the function as a benchmark


BENCHMARK(BM_Lock_Small<CompositeLock>)->Args({10,2})->Args({100,2})->Args({1000,2})->Args({10000,2})->Args({100000,2});
BENCHMARK(BM_Lock_Med<CompositeLock>)->Args({10,2})->Args({100,2})->Args({1000,2})->Args({10000,2})->Args({100000,2});
BENCHMARK(BM_Lock_Big<CompositeLock>)->Args({10,2})->Args({100,2})->Args({1000,2})->Args({10000,2})->Args({100000,2});



BENCHMARK_MAIN();