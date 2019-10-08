#include <stdafx.h>
#include <Windows.h>
#include "gstd/windows/thread.h"

using namespace gstd;

Thread::Thread()
{
  flag_stop_ = FALSE;
  handle_thread_ = NULL;
}

Thread::~Thread()
{
  CloseHandle(handle_thread_);
  handle_thread_ = NULL;
}

// 스레드시작
BOOL Thread::StartThread()
{
  // 이전 정지이력이 있을경우 TRUE로 되어있으므로 매시작시 초기화.
  flag_stop_ = FALSE;
  handle_thread_ = (HANDLE)_beginthreadex(NULL, 0, &Thread::_Run, 
                                          (LPVOID)this, 0, 0);
  return (handle_thread_ == NULL) ? FALSE : TRUE;
}

// 스레드등록 콜백함수.
unsigned int WINAPI Thread::_Run(LPVOID lpParam)
{
  Thread* pTh = static_cast<Thread*>(lpParam);
  pTh->Run();
  return 0;
}

/*
  pthread join.
  thread가 종료될때까지 무한대기.
*/
DWORD Thread::Join()
{
  return WaitForSingleObject (handle_thread_, INFINITE);
}