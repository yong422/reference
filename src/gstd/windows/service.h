#ifndef GSTD_WINDOWS_SERVICE_H
#define GSTD_WINDOWS_SERVICE_H

#include <string>
#include <stdint.h>
#include "gstd/windows/service_control.h"
#pragma once

#if !defined(PLOG_MAXSIZE)
#define PLOG_MAXSIZE 10*1024*1024 // (bytes)
#endif
// plog 파일의 최대 생성개수
#if !defined(PLOG_FILE_ROTATE)
#define PLOG_FILE_ROTATE 2
#endif

namespace gstd{
namespace genum{

enum RunMode{
  kDefault = 0,
  kService = 0,
  kConsole = 1,
  kWindows = 2
};
enum WatcherCode{
  kRunning = 0,     // watcher process 동작중
  kRestart = 1,     // watcher process 재시작 필요
  kStop = 2,        // watcher process 정상종료
  kRetriesStop = 3, // watcher process 재시작 종료
  kRestartStop = 3, // "
  kError = 4      
};
// plog 기본사용을 위한 index 정의
enum PlogIndex{
  kServiceLog = 1,
  kConsoleLog = 2
};

}//namespace genum


//  @brief  Windows service 프로그램을 위한 클래스
//          해당클래스를 상속하여 가상함수를 재정의하여 사용한다.
class Service : public ServiceControl
{
 public:
  Service(LPCTSTR service_name, LPCTSTR service_descr, LPCTSTR log_name = NULL);
  virtual ~Service();

  static Service* Get() {return this_ptr_;}
  static Service* this_ptr()  {return this_ptr_;}
  HANDLE GetServiceStopEvent()  {return service_stop_event_handle_;}
  VOID SetLogFile(const char* logname);
  // 정지가 호출됬는지 체크
  BOOL IsStop();

  // 해당 프로그램을 콘솔모드로 실행할때 호출될 main function
  BOOL ConsoleMain();
  // 해당 프로그램을 service 모드로 호출될때의 main function (외부호출)
  BOOL ServiceMain();
  // 등록된 외부컨트롤러 콜백에서 호출될 실제 서비스 콘트롤러
  VOID WINAPI CtrlHandler(DWORD code);
  // 등록된 외부콜백함수에서 호출될 실제 서비스 콜백함수.
  VOID CallServiceMain();
  // 서브스레드에서 동작할 worker
  DWORD Worker();
  // service 정지를 그외 스레드에서 요청하기 위한 함수
  VOID ServiceStop(){SetEvent(service_stop_event_handle_);}

  //! 메인스레드에서 종료요청이 들어올때 실행될 내용 재정의.
  //! 타스레드(통신스레드)에서 프로그램 종료를 요청할수있으므로 public
  virtual VOID Stop(UINT stop_code) = 0;

  // variables control
  VOID set_is_watcher(BOOL value=TRUE)  {is_watcher_ = value;}
  VOID set_is_run_watcher(BOOL value=TRUE)  {is_run_watcher_ = value;}
  VOID set_run_cycle_milliseconds(DWORD value){run_cycle_milliseconds_ = value;}
  VOID set_run_watcher_count(DWORD value) {run_watcher_count_ = value;}
  VOID set_watcher_process(LPCTSTR watcher_process);
  VOID set_is_stop_console(BOOL value) { is_stop_console_ = value; }

  BOOL is_watcher() {return is_watcher_;}
  BOOL is_run_watcher() {return is_run_watcher_;}
  DWORD run_cycle_milliseconds() {return run_cycle_milliseconds_;}
  DWORD run_watcher_count() {return run_watcher_count_;}
  std::string watcher_process() {return watcher_process_;}

 private:
  // Service를 사용하기 위해서 상속후 정의가 필요한 가상함수
  // 상세내용은 README.md 참조
  virtual BOOL Start() = 0;
  virtual VOID End() = 0;
  virtual VOID Run() = 0;

  UINT Watcher();
  UINT RunWatcher();
  UINT CheckWatcher();
  BOOL TerminateWatcher();
  BOOL IsRestartStopWatcher();

  //! variables control
  genum::RunMode run_mode() {return run_mode_;}
  BOOL is_stop_console() {return is_stop_console_;}
  VOID set_run_mode(genum::RunMode value) {run_mode_ = value;}
 public:
  TCHAR _LOGNAME[256];

 private:
  HANDLE                service_stop_event_handle_;
  SERVICE_STATUS        service_status_;
  SERVICE_STATUS_HANDLE service_status_handle_;
  PROCESS_INFORMATION   process_information_;
  std::string watcher_process_;
  BOOL is_watcher_;
  BOOL is_run_watcher_;
  BOOL is_stop_console_;            // console 모드일 경우 정지 플래그
  DWORD run_cycle_milliseconds_;
  DWORD run_watcher_count_;         // watcher process 재실행 최대횟수 (0일 경우 무한)
  DWORD current_run_watcher_count_; // 현재 watcher process 의 재실행 횟수
  genum::RunMode run_mode_; 

  static Service*  this_ptr_;
};

typedef Service Console;

} //namespace gstd
#endif