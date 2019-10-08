#include <stdafx.h>
#include <Windows.h>
#include <tchar.h>
#include "gstd/windows/msgbox.h"
#include "gstd/windows/strutil.h"
#include "gstd/windows/service_control.h"
#include <iostream>

using namespace gstd;

ServiceControl::ServiceControl(LPCSTR service_name, LPCTSTR service_descr)
{
  timeout_ = _GSTD_SERVICE_DEFAULT_TIMEOUT;
  last_error_number_ = 0;
  set_flag_notice(FALSE);
  ZeroMemory(service_name_, sizeof(service_name_));
  ZeroMemory(service_description_, sizeof(service_description_));
  ZeroMemory(path_binary_, sizeof(path_binary_));
#if _MSC_VER > 1900
  if (service_name)   _tcsncpy_s(service_name_, sizeof(service_name_) - 1, service_name, strlen(service_name));
  if (service_descr)  _tcsncpy_s(service_description_, sizeof(service_description_) - 1, 
                                service_descr, strlen(service_descr));
#else
  if(service_name)    _tcsncpy(service_name_, service_name, sizeof(service_name_)-1);
  if(service_descr)   _tcsncpy(service_description_, service_descr, sizeof(service_description_)-1);
#endif
  error_message_.clear();
}

ServiceControl::~ServiceControl()
{
}

BOOL ServiceControl::Uninstall()
{
  SC_HANDLE  service_control_manager    = NULL;
  SC_HANDLE  service_handle        = NULL;
  if (!_tcslen(service_name_))  return FALSE;

  service_control_manager  = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
  if (!service_control_manager) {
    gstd::msg::ErrorMessageBox(GetLastError());
    return FALSE;
  }
  MessageBox(NULL, service_name_, "Run Title", MB_ICONWARNING);
  // 현재있는 서비스를 오픈한다. 서비스의 오픈용도. 
  // 참조 https://msdn.microsoft.com/en-us/library/ms685981(v=vs.85).aspx
  service_handle = OpenService(service_control_manager, service_name_, DELETE);
  
  if (!service_handle) {
    gstd::msg::ErrorMessageBox(GetLastError());
    CloseServiceHandle(service_control_manager);
    return FALSE;
  }
  // 서비스 삭제 
  // 참조 https://msdn.microsoft.com/query/dev10.query?appId=Dev10IDEF1&l=KO-KR&k=k(DELETESERVICE);k(DevLang-%22C%2B%2B%22)&rd=true
  if (!DeleteService(service_handle)) {
    CloseServiceHandle(service_handle);
    CloseServiceHandle(service_control_manager);
    gstd::msg::ErrorMessageBox(GetLastError());
    return FALSE;
  }
  CloseServiceHandle(service_handle);
  CloseServiceHandle(service_control_manager);
  return TRUE;
}

BOOL ServiceControl::Install()
{
  SC_HANDLE  service_control_manager    = NULL;
  SC_HANDLE  service_handle        = NULL;
  ZeroMemory(path_binary_, sizeof(path_binary_));
  if (!service_name_) return FALSE;

  GetModuleFileName(NULL, path_binary_, MAX_PATH - 1);
  if (flag_notice()) {
    MessageBox(NULL, path_binary_, "Run Title", MB_ICONWARNING);
  }
  //! 생성을위한 서비스 컨트롤러 오픈.
  service_control_manager  = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
  if (!service_control_manager) {  
    // 서비스 컨트롤러 오픈 실패. 로그 또는 메일발송 추가
    DWORD errret = GetLastError();
    gstd::msg::ErrorMessageBox(errret);
    return FALSE;
  }

  //! 서비스를 등록한다.
  service_handle  = CreateService(service_control_manager,
                                  service_name_,  //! install될 서비스명
                                  service_description_,  //! 디스플레이 서비스명 max(256)
                                  SERVICE_ALL_ACCESS,  //! 서비스 Access 권한
                                  // 서비스타입. (응용프로그램은 SERVICE_WIN32_OWN_PROCESS)
                                  SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS,  
                                  SERVICE_AUTO_START,    //! 서비스 시작옵션
                                  //SERVICE_DEMAND_START,    //! 서비스 시작옵션
                                  SERVICE_ERROR_NORMAL,    //! 서비스 에러 제어
                                  path_binary_,      //! 서비스 실행파일 경로
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL, 
                                  NULL);

  //! 서비스 등록실패시 NULL 이 리턴됨. last error 값으로 체크.
  if (!service_handle) {
    //! 등록결과 서비스핸들러 오픈실패.
    DWORD errret = GetLastError();
    CloseServiceHandle(service_control_manager);
    gstd::msg::ErrorMessageBox(errret);
    return FALSE;
  }
  CloseServiceHandle(service_control_manager);
  CloseServiceHandle(service_handle);
  return TRUE;
}

// 등록된 서비스를 실행시킨다.
BOOL ServiceControl::DoStartService()
{
  BOOL result = false;
  int32_t is_stopped_result = 0;
  SC_HANDLE sc_manager = NULL;
  SC_HANDLE sc_service = NULL;
  SERVICE_STATUS_PROCESS service_status;
  DWORD start_time = 0;
  DWORD bytes_needed = 0;
  DWORD old_check_point = 0;

  if ((is_stopped_result = IsStoppedService()) == _GSTD_SERVICE_CHECK_STOPPED) {
    // 서비스가 정지중인 경우
    sc_manager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (NULL == sc_manager) {
      SetErrorInfo_();
      return result;
    }
    sc_service = OpenService(sc_manager, service_name_,SERVICE_ALL_ACCESS);
    if (NULL == sc_service) {
      CloseServiceHandle(sc_manager);
      SetErrorInfo_();
      return result;
    }
    // IsStoppedService 함수에서 STOP 된 상태인지, 정지중인지, 타임아웃인지 확인 한 상태이므로
    // 이미 정지되있는 서비스의 경우에만 해당 루틴으로 진행되므로 서비스 실행
    // TODO: 서비스의 argument 없음을 기본으로하며 추후 argument 추가 개발
    if (!StartService(sc_service, 0, NULL)) {
      SetErrorInfo_();
      goto START_CLEANUP;
    }
    if (!QueryServiceStatusEx(sc_service, SC_STATUS_PROCESS_INFO, (LPBYTE)&service_status,
                              sizeof(SERVICE_STATUS_PROCESS), &bytes_needed)) {
      SetErrorInfo_();
      goto START_CLEANUP;
    }
    start_time = GetTickCount();
    old_check_point = service_status.dwCheckPoint;
    while (service_status.dwCurrentState == SERVICE_START_PENDING) {
      Sleep(GetHintTimeCalculationResult_(service_status.dwWaitHint));
      if (!QueryServiceStatusEx(sc_service, SC_STATUS_PROCESS_INFO, (LPBYTE)&service_status,
        sizeof(SERVICE_STATUS_PROCESS), &bytes_needed)) {
        SetErrorInfo_();
        goto START_CLEANUP;
      }
      // 체크포인트 값을 체크하여 서비스 시작 진행중이면 대기.
      if (service_status.dwCheckPoint > old_check_point) {
        start_time = GetTickCount();
      } else {
        if (GetTickCount() - start_time > timeout_) {
          break;
        }
      }
    }
    if (service_status.dwCurrentState == SERVICE_RUNNING) {
      result = true;
    } else {
      SetErrorInfo_(WAIT_TIMEOUT, "DoStopService > timeout");
    }
  } else if (is_stopped_result == _GSTD_SERVICE_CHECK_RUNNING) {
    // 서비스가 동작중 인 경우. 
    // 서비스를 시작시키는 함수 이므로 실패처리. 
    // 단 에러코드는 0, 에러메시지에 동작중 표시.
    SetErrorInfo_(_GSTD_SERVICE_CHECK_RUNNING, "service is running");
  } else {
    // 서비스 시작 체크시 정지중 타임아웃인 경우는 에러처리와 동일
  }
START_CLEANUP:
  if (sc_service) CloseServiceHandle(sc_service);
  if (sc_manager) CloseServiceHandle(sc_manager);
  return result;
}

// 등록된 서비스를 정지시킨다.
// 정지중 인 상태인지 체크하며 정지중(STOP_PENDING)인 경우 타임아웃 시간동안 대기한다.
// 동작중 인 경우 서비스 정지 실행 후 타임아웃 시간동안 대기한다.
// 정상 종료시 TRUE, 타임아웃 또는 오류시 GetLastError() 값을 에러 멤버변수에 저장후 종료
BOOL ServiceControl::DoStopService()
{
  BOOL result = false;
  int32_t is_stopped_result = 0;
  SC_HANDLE sc_manager = NULL;
  SC_HANDLE sc_service = NULL;
  SERVICE_STATUS_PROCESS service_status;
  DWORD start_time = GetTickCount();
  DWORD bytes_needed = 0;

  if ((is_stopped_result = IsStoppedService()) == _GSTD_SERVICE_CHECK_RUNNING) {
    // 서비스가 동작중 인 경우
    sc_manager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (NULL == sc_manager) {
      SetErrorInfo_();
      return result;
    }
    sc_service = OpenService(sc_manager, service_name_,
                            SERVICE_STOP | SERVICE_QUERY_STATUS | SERVICE_ENUMERATE_DEPENDENTS);
    if (NULL == sc_service) {
      CloseServiceHandle(sc_manager);
      SetErrorInfo_();
      return result;
    }
    if (!QueryServiceStatusEx(sc_service, SC_STATUS_PROCESS_INFO, (LPBYTE)&service_status, 
                              sizeof(SERVICE_STATUS_PROCESS), &bytes_needed)) {
      // 서비스의 상태값 가져오기 실패
      SetErrorInfo_();
      goto STOP_CLEANUP;
    }

    if (!ControlService(sc_service, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&service_status)) {
      // 서비스 정지 요청
      SetErrorInfo_();
      goto STOP_CLEANUP;
    }

    // 서비스가 정지될때까지 체크.
    // 타임아웃시 종료
    while (service_status.dwCurrentState != SERVICE_STOPPED) {
      // hint 시간을 체크하여 대기한다.
      Sleep(GetHintTimeCalculationResult_(service_status.dwWaitHint));

      if (!QueryServiceStatusEx(sc_service, SC_STATUS_PROCESS_INFO, (LPBYTE)&service_status,
                                sizeof(SERVICE_STATUS_PROCESS), &bytes_needed)) {
        // 서비스의 상태값 가져오기 실패
        SetErrorInfo_();
        goto STOP_CLEANUP;
      }
      if (service_status.dwCurrentState == SERVICE_STOPPED) {
        result = true;
        break;
      }
      if (GetTickCount() - start_time > timeout_) {
        SetErrorInfo_(WAIT_TIMEOUT, "DoStopService > timeout");
        break;
      }
    }
  } else if (is_stopped_result == _GSTD_SERVICE_CHECK_STOPPED) {
    // IsStoppedService() 호출시 종료처리 되어있는 경우 
    result = true;
  }

STOP_CLEANUP:
  if (sc_service) CloseServiceHandle(sc_service);
  if (sc_manager) CloseServiceHandle(sc_manager);
  return result;
}

int32_t ServiceControl::IsStoppedService()
{
  int32_t result = _GSTD_SERVICE_CHECK_ERROR;
  SC_HANDLE sc_manager;
  SC_HANDLE sc_service;
  SERVICE_STATUS_PROCESS service_status = { 0, };
  DWORD start_time = GetTickCount();
  DWORD bytes_needed = 0;
  DWORD wait_time = 0;

  sc_manager = OpenSCManager(
                            NULL, // local computer
                            NULL,  // ServicesActive database 
                            SC_MANAGER_ALL_ACCESS); // full access rights 
  if (NULL == sc_manager) {
    SetErrorInfo_();
    return result;
  }
  sc_service = OpenService(sc_manager, service_name_, 
                          SERVICE_STOP |
                          SERVICE_QUERY_STATUS |
                          SERVICE_ENUMERATE_DEPENDENTS);
  
  if (NULL == sc_service) {
    CloseServiceHandle(sc_manager);
    SetErrorInfo_();
    return result;
  }
  if (!QueryServiceStatusEx(sc_service, SC_STATUS_PROCESS_INFO,
                            (LPBYTE)&service_status, sizeof(SERVICE_STATUS_PROCESS),
                            &bytes_needed)) {
    SetErrorInfo_();
    goto ISSTOP_CLEANUP;
  }

  if (service_status.dwCurrentState == SERVICE_STOPPED) {
    // 서비스는 이미 정지인 상태
    result = _GSTD_SERVICE_CHECK_STOPPED;
    goto ISSTOP_CLEANUP;
  }
  
  // 서비스가 정지중에 있는 경우 대기하며 체크.
  while (service_status.dwCurrentState == SERVICE_STOP_PENDING) {
#if defined(_DEBUG) || defined(_TEST_MODE)
    std::cout << "service > " << service_name_  << " > stop pending.." << std::endl;
#endif

    Sleep(GetHintTimeCalculationResult_(service_status.dwWaitHint));
    if (!QueryServiceStatusEx(sc_service, SC_STATUS_PROCESS_INFO,
                              (LPBYTE)&service_status, sizeof(SERVICE_STATUS_PROCESS),
                              &bytes_needed)) {
      SetErrorInfo_();
      goto ISSTOP_CLEANUP;
    }
    if (service_status.dwCurrentState == SERVICE_STOPPED) {
#if defined(_DEBUG) || defined(_TEST_MODE)
      std::cout << "Service stopped successfully > " << service_name_ << std::endl;
#endif
      result = _GSTD_SERVICE_CHECK_STOPPED;
      goto ISSTOP_CLEANUP;
    }
    if (GetTickCount() - start_time > timeout_) {
#if defined(_DEBUG) || defined(_TEST_MODE)
      std::cout << "Service stop timeout > " << service_name_ << std::endl;
#endif
      SetErrorInfo_(WAIT_TIMEOUT, "IsStoppedService > timeout");
      result = _GSTD_SERVICE_CHECK_TIMEOUT;
      goto ISSTOP_CLEANUP;
    }
  }
  result = _GSTD_SERVICE_CHECK_RUNNING;

// 타임아웃 또는 오류로 실패시 STOP_CLEANUP 으로 이동.
ISSTOP_CLEANUP:
  if (sc_service) CloseServiceHandle(sc_service);
  if (sc_manager) CloseServiceHandle(sc_manager);
  return result;
}

// private

// 참조 : https://docs.microsoft.com/en-us/windows/desktop/services/svccontrol-cpp
// 서비스 체크 대기시간은 힌트타임의 1/10 정도가 좋으며 1~10 sec 사이로 설정해야 한다.
DWORD ServiceControl::GetHintTimeCalculationResult_(DWORD hint_time)
{
  DWORD wait_time = 0;
  wait_time = hint_time / 10;
  if (wait_time < 1000)       wait_time = 1000;
  else if (wait_time > 10000) wait_time = 10000;
  return wait_time;
}

// 에러정보를 내부변수에 저장한다.
// 변수로 받은 에러번호와 에러 문자열이 있을 경우 해당값을 설정한다.
// 변수값이 기본값인 경우 마지막 윈도우즈 발생 에러 번호와 에러 문자열을 내부에 저장한다.
VOID ServiceControl::SetErrorInfo_(DWORD error_number, std::string error_string)
{
  if (!error_number && error_string.empty()) {
    last_error_number_ = GetLastError();
    error_message_ = gstd::GetErrorString(last_error_number_);
  } else {
    last_error_number_ = error_number;
    error_message_ = error_string;
  }
}