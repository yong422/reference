
//  @file   timer.h
//  @brief  Timer 선언
//  @author  조용규
#ifndef GSTD_UTIL_TIMER_H
#define GSTD_UTIL_TIMER_H
#include <stdio.h>
#include <string.h>
#include <time.h>
#if defined(_WIN32)
#include <Windows.h>
#endif
#if defined(__gnu_linux__)
#include <unistd.h>
#include <sys/time.h>
#endif

#include <list>

#define _MY_PER_SEC   1000000
#define _MY_PER_MSEC   1000


namespace gstd{
namespace util{

// @class   Timer
// @brief  타이머기능의 클래스
class Timer{
 public:
  Timer();
  ~Timer();
  void Start();
  double Stop(bool save=true);
  void Reset();
  void Clear();
 private:
  double _sInterval(struct timeval now);
  double _mInterval(struct timeval now);
  double _uInterval(struct timeval now);

 private:
  std::list<struct timeval>   list_timeval_;  //! 스탑시간리스트
  struct timeval              start_;
  int                         stop_count_;    //! 스탑 호출카운트
};

} // namespace util
} // namespace gstd

#endif // GSTD_UTIL_TIMER_H
