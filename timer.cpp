#include <chrono>

class timer{
  std::chrono::high_resolution_clock clock;
  std::chrono::high_resolution_clock::time_point start;
  public:
  timer(){
      start = clock.now();
  }
   public:
  std::uint64_t elapsed(){
      return (clock.now()-start).count();
  }

};