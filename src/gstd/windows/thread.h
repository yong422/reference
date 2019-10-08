
//  @file     thread.h
//  @author   Yongkyu Jo
//  @brief    Windows용 simple thread class
//            스레드함수를 상속하여 Run함수를 재정의한후 사용한다.

#ifndef __GSTD_THREAD_H_
#define __GSTD_THREAD_H_

#include <windows.h>
#include <process.h>

#pragma once
namespace gstd{

class Thread{
 public:
  Thread();
  virtual ~Thread();
  VOID SetID(int id){thread_number_ = id;}
  BOOL StartThread();
  BOOL IsStop(){return flag_stop_;}
  VOID Stop(){flag_stop_=TRUE;}
  DWORD Join();
  virtual VOID Run()=0;

 private:
  static UINT WINAPI _Run(LPVOID lpParam);

 private:
  BOOL flag_stop_;
  HANDLE handle_thread_;
  int thread_number_;
};

typedef Thread CThread;

} //namespace gstd
#endif
