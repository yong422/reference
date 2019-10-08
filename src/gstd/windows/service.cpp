
//  @file service.cpp

#include <stdafx.h>
#include <windows.h>
#include <string>
#include <tchar.h>
#include "gstd/windows/service.h"
#include "gstd/windows/directory_control.h"
#include <plog/Log.h>

namespace gstd{

Service* Service::this_ptr_ = NULL;

//  callback을 위한 함수정의
DWORD WINAPI ServiceWorkerThread (LPVOID lpParam);
VOID WINAPI CallbackServiceMain();
VOID WINAPI ServiceCtrlHandler(DWORD code);


Service::Service(LPCTSTR service_name, LPCTSTR service_descr, LPCTSTR log_name)
  :ServiceControl(service_name, service_descr)
{
  is_watcher_ = FALSE;
  is_run_watcher_ = FALSE;
  is_stop_console_ = FALSE;
  run_cycle_milliseconds_ = 1000;
  run_watcher_count_ = 0;
  current_run_watcher_count_ = 0;
  watcher_process_.clear();
  service_status_handle_ = NULL;
  ZeroMemory(&service_status_, sizeof(service_status_));
  ZeroMemory(_LOGNAME, sizeof(_LOGNAME));
#if _MSC_VER > 1900
  _tcsncpy_s(_LOGNAME, sizeof(_LOGNAME), "service.log", sizeof(_LOGNAME) - 1);
#else
  strncpy(_LOGNAME, "service.log", sizeof(_LOGNAME) - 1);
#endif
  service_stop_event_handle_ = INVALID_HANDLE_VALUE;
  this_ptr_ = this;
  if (log_name) {
    plog::init<genum::kServiceLog>(plog::info, log_name, PLOG_MAXSIZE, PLOG_FILE_ROTATE);
    plog::init<genum::kConsoleLog>(plog::info, log_name, PLOG_MAXSIZE, PLOG_FILE_ROTATE);
  } else {
    plog::init<genum::kServiceLog>(plog::info, "log/service.log", PLOG_MAXSIZE, PLOG_FILE_ROTATE);
    plog::init<genum::kConsoleLog>(plog::info, "log/console.log", PLOG_MAXSIZE, PLOG_FILE_ROTATE);
  }
  run_mode_ = genum::kDefault;
}

Service::~Service()
{
  this_ptr_ = NULL;
}

VOID Service::set_watcher_process(LPCTSTR watcher_process)
{
  if (watcher_process) {
    set_is_watcher(TRUE);
    std::string absolute_path_process
       = gstd::GetAbsolutePath() + "\\" + std::string(watcher_process);
    watcher_process_.assign(absolute_path_process);
  }
}




VOID Service::SetLogFile(LPCTSTR logfile)
{
  ZeroMemory(_LOGNAME, sizeof(_LOGNAME));
#if _MSC_VER >= 1900
  _tcsncpy_s(_LOGNAME, sizeof(_LOGNAME), logfile, sizeof(_LOGNAME) - 1);
#else
  _tcsncpy(_LOGNAME, logfile, sizeof(_LOGNAME)-1);
#endif
  plog::init<genum::kServiceLog>(plog::info, _LOGNAME, PLOG_MAXSIZE, PLOG_FILE_ROTATE);
  plog::init<genum::kConsoleLog>(plog::info, _LOGNAME, PLOG_MAXSIZE, PLOG_FILE_ROTATE);
}

BOOL Service::IsStop()
{
  if (run_mode() == genum::kConsole) {
    return is_stop_console();
  } else if (run_mode() == genum::kService) {
    return (WaitForSingleObject(service_stop_event_handle_, 0) != WAIT_OBJECT_0) ? FALSE : TRUE;
  }
  return TRUE;
}

BOOL Service::ServiceMain()
{
  set_run_mode(genum::kService);
  LOG_INFO_(genum::kServiceLog) << "start";
  SERVICE_TABLE_ENTRY ServiceTable[] = 
    {
      {service_name_, (LPSERVICE_MAIN_FUNCTION) CallbackServiceMain},
      {NULL, NULL}
    };
  if (StartServiceCtrlDispatcher(ServiceTable) == FALSE) {
    LOG_ERROR_(genum::kServiceLog) << "StartServiceCtrlDispatcher failed " 
                                   << GetLastError();
    return FALSE;
  }
  LOG_INFO_(genum::kServiceLog) << "Success Exit";
  return TRUE;
}

VOID WINAPI Service::CtrlHandler(DWORD code)
{
  switch (code) {
    case SERVICE_CONTROL_STOP :
      // 서비스가 동작중이 아닌경우는 통과
      if (service_status_.dwCurrentState != SERVICE_RUNNING)  break;

      service_status_.dwControlsAccepted = 0;
      service_status_.dwCurrentState = SERVICE_STOP_PENDING;
      service_status_.dwWin32ExitCode = 0;
      service_status_.dwCheckPoint = 4;

      if (SetServiceStatus (service_status_handle_, &service_status_) == FALSE) {
        LOG_ERROR_(genum::kServiceLog) << "SetServiceStatus returned error " 
                      << GetLastError();
      }
      LOG_INFO_(genum::kServiceLog) << "Service Stop";
      //! main second thread stop
      SetEvent (service_stop_event_handle_);
      break;
    default:
      break;
  }
}

VOID Service::CallServiceMain()
{
  DWORD Status = E_FAIL;
  HANDLE hThread = NULL;
  LOG_INFO_(genum::kServiceLog) << "Start";
  service_status_handle_ = RegisterServiceCtrlHandler (service_name_, ServiceCtrlHandler);
  if (service_status_handle_ == NULL) {
    LOG_ERROR_(genum::kServiceLog) << "Register Handler failed " << GetLastError();
    goto EXIT;
  }

  ZeroMemory (&service_status_, sizeof(service_status_));
  service_status_.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  service_status_.dwControlsAccepted = 0;
  service_status_.dwCurrentState = SERVICE_START_PENDING;
  service_status_.dwWin32ExitCode = 0;
  service_status_.dwServiceSpecificExitCode = 0;
  service_status_.dwCheckPoint = 0;

  if (SetServiceStatus(service_status_handle_, &service_status_) == FALSE) {
    LOG_ERROR_(genum::kServiceLog) << "SetServiceStatus returned error" 
                  << GetLastError();
  }
  LOG_INFO_(genum::kServiceLog) << "SetServiceStatus OK";

  service_stop_event_handle_ = CreateEvent (NULL, TRUE, FALSE, NULL);
  if (service_stop_event_handle_ == NULL) {
    LOG_ERROR_(genum::kServiceLog) << "CreateEvent() returned error " 
                  << GetLastError();
    service_status_.dwControlsAccepted = 0;
    service_status_.dwCurrentState = SERVICE_STOPPED;
    service_status_.dwWin32ExitCode = GetLastError();
    service_status_.dwCheckPoint = 1;

    if (SetServiceStatus (service_status_handle_, &service_status_) == FALSE)
    {
      LOG_ERROR_(genum::kServiceLog) << "SetServiceStatus Stopped set returned error " 
                    << GetLastError();
    }
    goto EXIT; 
  }    

  service_status_.dwControlsAccepted = SERVICE_ACCEPT_STOP;
  service_status_.dwCurrentState = SERVICE_RUNNING;
  service_status_.dwWin32ExitCode = 0;
  service_status_.dwCheckPoint = 0;

  if (SetServiceStatus (service_status_handle_, &service_status_) == FALSE) {
    LOG_ERROR_(genum::kServiceLog) << "SetServiceStatus returned error " 
                  << GetLastError();
  }

  hThread = CreateThread (NULL, 0, ServiceWorkerThread, (LPVOID)Service::Get(), 0, NULL);
  LOG_INFO_(genum::kServiceLog) << "Worker Thread create success and wait";

  //! main second thread 대기
  WaitForSingleObject (hThread, INFINITE);

  CloseHandle (service_stop_event_handle_);

  service_status_.dwControlsAccepted = 0;
  service_status_.dwCurrentState = SERVICE_STOPPED;
  service_status_.dwWin32ExitCode = 0;
  service_status_.dwCheckPoint = 3;

  if (SetServiceStatus (service_status_handle_, &service_status_) == FALSE) {
    LOG_ERROR_(genum::kServiceLog) << "Stop State SetServiceStatus returned error " 
                  << GetLastError();
  }
    
EXIT:
  LOG_INFO_(genum::kServiceLog) << "Exit";
  return;
}

DWORD Service::Worker()
{
  if (run_mode() == genum::kService)
    LOG_INFO_(genum::kServiceLog) << "==>  Service Main Start  ==>";
  else if (run_mode() == genum::kConsole)
    LOG_INFO_(genum::kConsoleLog) << "==>  Console Main Start  ==>";
  int cnt=0;
  BOOL ifstart = Start();
  UINT run_mode_check = genum::kRunning;
  if (Watcher() != genum::kRunning) {
    ifstart = FALSE;
  }

  if (ifstart) {
    //! windows service 에서 스톱이 호출될때까지 대기
    while (!IsStop()) {        
      if ((run_mode_check = Watcher()) != genum::kRunning) {
        if (run_mode_check == genum::kStop){
          LOG_INFO_(genum::kServiceLog) << "watcher process correct stop";
          break;
        } else if (run_mode_check == genum::kRestartStop) {
          LOG_ERROR_(genum::kServiceLog) << "watcher process abnormal termination";
          LOG_ERROR_(genum::kServiceLog) << "set retries count [" << run_watcher_count_
                                         << "] > current retries count [" << current_run_watcher_count_
                                         << "]";
          break;
        }
        LOG_ERROR_(genum::kServiceLog) << "process not running";
      }
  #if defined(_TEST_MODE_TIMEOUT)
      cnt++;
      if(cnt == 60)  break;
  #endif
      Run();
      Sleep(run_cycle_milliseconds());
    }
  }
  // 서비스가 정상적으로 종료처리 되는 경우 subprocess 를 terminate 시킨다
  TerminateWatcher();
  if(run_mode() == genum::kService) {
    LOG_INFO_(genum::kServiceLog) << "==>  Service Stop Start  ==>";
    Stop(static_cast<UINT>(run_mode_check));
    LOG_INFO_(genum::kServiceLog) << "==>  Service End Start  ==>";
    End();
    LOG_INFO_(genum::kServiceLog) << "==>  Service Main Exit  ==>";
  } else if (run_mode() == genum::kConsole) {
    LOG_INFO_(genum::kConsoleLog) << "==>  Console Stop Start  ==>";
    Stop(0);
    LOG_INFO_(genum::kConsoleLog) << "==>  Console End Start  ==>";
    End();
    LOG_INFO_(genum::kConsoleLog) << "==>  Console Main Exit  ==>";
  }
  return ERROR_SUCCESS;
}

/*
  program 의 console 모드
*/
BOOL Service::ConsoleMain()
{
  set_run_mode(genum::kConsole);
  HANDLE hThread = CreateThread (NULL, 0, ServiceWorkerThread, (LPVOID)Service::Get(), 0, NULL);
  LOG_INFO_(genum::kConsoleLog) << "Worker Thread create success and wait";

  //! main second thread 대기
  WaitForSingleObject (hThread, INFINITE);
  return TRUE;
}

/*
  watcher 기능 실행
  watcher 에 관리되는 프로세스가 미실행 중인 경우 프로세스 실행
  이미 프로세스가 실행 중인 경우 정상 동작 중인지 여부 체크
*/
UINT Service::Watcher()
{
  UINT result = genum::kRunning;
  if (is_watcher()){
    if (is_run_watcher()) {
      // watcher process 모니터링
      result = CheckWatcher();
    } else {
      // watcher process 실행
      result = RunWatcher();
    }
    if (IsRestartStopWatcher()) {
      // 설정된 watcher 프로세스 재시작 회수를 초과하는 경우 정지한다
      result = genum::kRestartStop;
    }
  }
  return result;
}

UINT Service::CheckWatcher()
{
  UINT result = genum::kRunning;
  DWORD exit_code = 0;
  DWORD object_return_code = WaitForSingleObject(
                                process_information_.hProcess,
                                5000);
  switch (object_return_code) {
    case WAIT_FAILED:
      LOG_ERROR_(genum::kServiceLog) << "watcher wait object failed => errno " 
                    << GetLastError();
      result = genum::kError;
      break;
    case WAIT_TIMEOUT:
      // process 동작중.
      LOG_VERBOSE_(genum::kServiceLog) << "watcher wait object timeout";
      break;
    case WAIT_ABANDONED:
      LOG_ERROR_(genum::kServiceLog) << "watcher wait object abandoned";
      ResetEvent(process_information_.hProcess);
      break;
    case WAIT_OBJECT_0:
      // process 의 시그널이 생긴 경우 (하위 프로세스 종료)
      LOG_INFO_(genum::kServiceLog) << "watcher wait object signaled";
      result = genum::kStop;
      break;
  }
  if (!GetExitCodeProcess(process_information_.hProcess, &exit_code)) {
    LOG_ERROR_(genum::kServiceLog) << "get watcher process exit error code "<<  GetLastError();
  } else {
    if (exit_code == STILL_ACTIVE) {
      result = genum::kRunning;
      LOG_VERBOSE_(genum::kServiceLog) << "get watcher process running";
    } else {
      LOG_INFO_(genum::kServiceLog) << "get watcher process exit code : " << exit_code;
      if (exit_code == 0) {
        // subprocess 의 exit code 가 0 일때만 정상종료로 간주한다.
        result = genum::kStop;
      } else {
        // 그외의 exit_code 인 경우 비정상 종료로 판단하고 subprocess 를 재시작한다.
        result = genum::kRestart;
        ++current_run_watcher_count_;
      }
      set_is_run_watcher(FALSE);
      CloseHandle(process_information_.hProcess);
    }
  }
  return result;
}

/*
  watcher 에 관리될 하위 프로세스를 실행한다.
*/
UINT Service::RunWatcher()
{
  UINT result = genum::kError;
  if (!watcher_process_.empty()) {
    STARTUPINFO startup_info = {0,};
    if (!CreateProcess(NULL,
                      _T(const_cast<LPSTR>(watcher_process_.c_str())),
                      NULL,NULL,TRUE,CREATE_NO_WINDOW,NULL,NULL,
                      //NULL,NULL,TRUE,0,NULL,NULL,
                      &startup_info, &process_information_)) {
      // watcher process 생성 실패. 에러체크.
      LOG_ERROR_(genum::kServiceLog) << "create process errno : " << GetLastError();
      result = genum::kError;
    } else {  // watcher process 생성
      LOG_INFO_(genum::kServiceLog) << "watcher start => " << watcher_process_;
      LOG_INFO_(genum::kServiceLog) << "sub process pid [" << process_information_.dwProcessId <<"]";
      CloseHandle(process_information_.hThread);
      set_is_run_watcher();
      result = genum::kRunning;
    }
  } else {  // watcher 설정 오류
    LOG_ERROR_(genum::kServiceLog) << "watcher process empty";
    result = genum::kError;
  }
  return result;
}

/*
  watcher 에서 관리하는 하위 프로세스를 강제종료 시킨다.
*/
BOOL Service::TerminateWatcher()
{
  BOOL result = FALSE;

  if (is_watcher() && is_run_watcher()) {
    if (process_information_.dwProcessId > 0 &&
        process_information_.hProcess != NULL &&
        process_information_.hProcess != INVALID_HANDLE_VALUE) {
      DWORD exit_code = 0;
      if (GetExitCodeProcess(process_information_.hProcess, &exit_code)) {
        // 종료전 하위프로세스 exit code 체크.
        if (exit_code == STILL_ACTIVE) {
          // 프로세스가 동작중일 경우 종료.
          TerminateProcess(process_information_.hProcess, 1);
        }
      } else {
        // 하위 프로세스의 exit code 가져오기 실패

      }
    }
  }
  return result;
}

// watcher 재시작 회수를 
BOOL Service::IsRestartStopWatcher()
{
  if (current_run_watcher_count_ >= run_watcher_count_) {
    return TRUE;
  }
  return FALSE;
}

/**
  @brief   콜백또는 스레드용 일반함수
*/

VOID WINAPI CallbackServiceMain()
{
  Service::Get()->CallServiceMain();
}

VOID WINAPI ServiceCtrlHandler(DWORD code)
{
  Service::Get()->CtrlHandler(code);
}


DWORD WINAPI ServiceWorkerThread (LPVOID lpParam)
{
  Service* pServ = (Service*)lpParam;
  return pServ->Worker();
}

} // namespace gstd