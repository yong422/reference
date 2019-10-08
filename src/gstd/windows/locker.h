#ifndef GSTD_WINDOWS_LOCKER_H
#define GSTD_WINDOWS_LOCKER_H

#if defined(_WIN32)
#include <Windows.h>
#include <WinBase.h>
#elif defined(_LINUX)
#include <pthread.h>
#endif

namespace gstd{

class Locker{
 public:
  Locker();
  ~Locker();
  VOID Lock();
  VOID Unlock();

 private:
#if defined(_WIN32)
  PCRITICAL_SECTION      critical_section_;
#elif defined(_LINUX)
  pthread_mutex_t*       mutex_;
#endif
};
typedef Locker CLocker;
} //namespace gstd
#endif