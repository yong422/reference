#include <stdafx.h>

#include "gstd/windows/locker.h"


using namespace gstd;

Locker::Locker()
{
#if defined(_WIN32)
  critical_section_ = NULL;
  critical_section_ = new CRITICAL_SECTION;
  if(critical_section_)  InitializeCriticalSection(critical_section_);
#elif defined(_LINUX)
  mutex_ = NULL;
  mutex_ = new pthread_mutex_t;
  if(mutex_)  pthread_mutex_init(mutex_, NULL);
#endif
}

Locker::~Locker()
{
#if defined(_WIN32)
  if (critical_section_) {
    DeleteCriticalSection(critical_section_);
    delete critical_section_;
    critical_section_ = NULL;
  }
#elif defined(_LINUX)
  if (mutex_) {
    delete mutex_;
    mutex_ = NULL;
  }
#endif
}

VOID Locker::Lock()
{
#if defined(_WIN32)
  EnterCriticalSection(critical_section_);
#elif defined(_LINUX)
  pthread_mutex_lock(mutex_)
#endif
}

VOID Locker::Unlock()
{
#if defined(_WIN32)
  LeaveCriticalSection(critical_section_);
#elif defined(_LINUX)
  pthread_mutex_unlock(mutex_);
#endif
}