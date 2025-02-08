#pragma once
#include <chrono>
using namespace std::chrono;

class Timer
{
  public:
    Timer()
    {
        last = steady_clock::now();
    }

    duration<float> Mark() noexcept
    {

        const auto old             = last;
        last                       = steady_clock::now();
        const duration<float> time = last - old;
        return time;
    }
    duration<float> Peek() const noexcept
    {
        const duration<float> time = steady_clock::now() - last;
        return time;
    }

  private:
    std::chrono::steady_clock::time_point last;
};
