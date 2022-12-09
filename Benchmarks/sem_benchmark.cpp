#include <bits/stdc++.h>
#include <chrono>
#include <thread>
#include <mutex>
#include "timer.cpp"
#include <benchmark/benchmark.h>
#include <semaphore.h>
using namespace std;


class critical_cases
{
    public:
    critical_cases()
    {
        sem_init(&lock,0,1);
    }
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

        sem_wait(&(this->lock));
        ++counter;
        sem_post(&(this->lock));
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

        sem_wait(&(this->lock));
        ++counter;
        // Do artificial work for 50us
        timer t2;
        while (t2.elapsed()< (uint64_t)(grain_size/2)*1e5)
        {
        }
        sem_post(&(this->lock));
    }

    ////////////////////////////////////////////////////////////////////////////
    // Critical_big does all of its work in the critical section making it
    // adequate to compare with lock-based queues/linked-lists where majority of
    //  the code is under locks.
    void critical_big(std::uint64_t grain_size)
    {
        sem_wait(&(this->lock));
        ++counter;

        // Do artificial work for grain_size
        timer t;
        while (t.elapsed() < grain_size*1e5)
        {
        }
        sem_post(&(this->lock));
    }

private:
    std::uint64_t counter{};
    sem_t lock;
};


struct params{
  critical_cases *cases;
  std::uint64_t grain_size;
};


void* Critical_small(void* arg)
{
    params* p = (params*)arg;
    ((critical_cases*)(p->cases))->critical_small(p->grain_size);
    return NULL;
}


void* Critical_med(void* arg)
{
    params* p = (params*)arg;
    ((critical_cases*)(p->cases))->critical_med(p->grain_size);
    return NULL;
}


void* Critical_big(void* arg)
{
    params* p = (params*)arg;
    ((critical_cases*)(p->cases))->critical_big(p->grain_size);
    return NULL;
}


void critical_small(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    critical_cases cases;
    std::vector <pthread_t> temp;
    params * p = new params();
    p->cases = &cases;
    p->grain_size = grain_size;
    //std::vector<std::thread*> temp;
    for(uint64_t i =0;i<num_tasks;i++)
    {
        pthread_t t;
        pthread_create(&t,NULL,&Critical_small,(void*)p);
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
    
} 


void critical_med(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    critical_cases cases;
    std::vector <pthread_t> temp;
    params * p = new params();
    p->cases = &cases;
    p->grain_size = grain_size;
    //std::vector<std::thread*> temp;
    for(uint64_t i =0;i<num_tasks;i++)
    {
        pthread_t t;
        pthread_create(&t,NULL,&Critical_med,(void*)p);
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
    
} 

void critical_big(std::uint64_t num_tasks, std::uint64_t grain_size)
{
    critical_cases cases;
    std::vector <pthread_t> temp;
    params * p = new params();
    p->cases = &cases;
    p->grain_size = grain_size;
    //std::vector<std::thread*> temp;
    for(uint64_t i =0;i<num_tasks;i++)
    {
        pthread_t t;
        pthread_create(&t,NULL,&Critical_big,(void*)p);
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
    
} 


static void BM_semaphore_Small(benchmark::State& state) {
  for (auto _ : state)
    critical_small(state.range(0),state.range(1));
    benchmark::ClobberMemory();
}


static void BM_semaphore_Med(benchmark::State& state) {
  for (auto _ : state)
    critical_med(state.range(0),state.range(1));
    benchmark::ClobberMemory();
}

static void BM_semaphore_Big(benchmark::State& state) {
  for (auto _ : state)
    critical_big(state.range(0),state.range(1));
    benchmark::ClobberMemory();
}





// Register the function as a benchmark
BENCHMARK(BM_semaphore_Small)->Args({10,2})->Args({100,2})->Args({1000,2})->Args({10000,2});
BENCHMARK(BM_semaphore_Med)->Args({10,2})->Args({100,2})->Args({1000,2})->Args({10000,2});
BENCHMARK(BM_semaphore_Big)->Args({10,2})->Args({100,2})->Args({1000,2})->Args({10000,2});

//BENCHMARK(Comp_trial);`
BENCHMARK_MAIN();
