/**
  @file  timer.cpp
  @author  조용규
  @brief  Timer 정의
*/
#if defined(_WIN32)
#include "gstd/windows/common_linux.h"
#endif
#include "timer.h"

using namespace gstd::util;
Timer::Timer()
{
  gettimeofday(&start_, NULL);
  list_timeval_.clear();
}

Timer::~Timer()
{
  Reset();
}

void Timer::Clear()
{
  Reset();
}

void Timer::Reset()
{
  memset(&start_, 0x00, sizeof(struct timeval));
  list_timeval_.clear();
}

void Timer::Start()
{
  Reset();
  gettimeofday(&start_, NULL);
  list_timeval_.push_back(start_);
}

// Start 로 부터의 시간을 소요시간을 초단위로 리턴한다.
double Timer::Stop(bool save)
{
  struct timeval now;
  gettimeofday(&now, NULL);
  if(save)  list_timeval_.push_back(now);
  return _sInterval(now);
}

double Timer::_mInterval(struct timeval now)
{
  return (double)(_uInterval(now)/_MY_PER_MSEC);
}
double Timer::_uInterval(struct timeval now)
{
  double sec = (now.tv_sec - start_.tv_sec);
  double usec = (now.tv_usec - start_.tv_usec);
  return (double)(sec*_MY_PER_SEC + usec);
}
double Timer::_sInterval(struct timeval now)
{
  return (double)(_uInterval(now)/_MY_PER_SEC);
}

#if defined(_TEST_TIMER)
int main(int argc, char** argv)
{
  Timer ltimer;
  ltimer.Start();
  for(int i=0;i<1000;i++){
  usleep(1);}
  printf("interval %.6f sec\n", ltimer.Stop());
  sleep(1);
  printf("interval %.6f sec\n", ltimer.Stop());
  usleep(1000);
  printf("interval %.6f sec\n", ltimer.Stop());
  usleep(10000);
  printf("interval %.6f sec\n", ltimer.Stop());
  return 1;
}
#endif



