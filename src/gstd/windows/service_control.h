#ifndef GSTD_WINDOWS_SERVICE_CONTROL_H
#define GSTD_WINDOWS_SERVICE_CONTROL_H

#include <Windows.h>
#include <stdint.h>
#include <string>

#define _GSTD_SERVICE_DEFAULT_TIMEOUT 30000 // ms
#define _GSTD_SERVICE_CHECK_RUNNING 0
#define _GSTD_SERVICE_CHECK_STOPPED 1
#define _GSTD_SERVICE_CHECK_ERROR   -1
#define _GSTD_SERVICE_CHECK_TIMEOUT -2

#pragma once
namespace gstd {

// @class ServiceControl
// windows service contorl 클래스
// service 기본정보 변수와 install, uninstall 등의 기능사용시 사용
// CService에서 상속하여 사용한다.
class ServiceControl{
 public: 
  ServiceControl(LPCTSTR ServiceName, LPCTSTR ServiceDescr);
  virtual ~ServiceControl();
  BOOL Uninstall();
  BOOL Install();
  // 서비스를 실행하는 함수
  // 서비스 실행이 성공한 경우 TRUE, 타임아웃, 동작중 또는 실패시 FALSE 를 리턴한다.
  // 타임아웃의 경우 내부 에러번호를 확인 (WAIT_TIMEOUT)
  // 이미 실쟁중인 서비스 인 경우 내부 에러번호가 0 이다.
  BOOL DoStartService();
  // 서비스를 정지하는 함수
  // 정지가 완료되었을 경우 TRUE, 타임아웃 또는 실패시 FALSE 를 리턴한다.
  // 타임아웃의 경우 내부 에러 번호를 체크해야 한다.
  BOOL DoStopService();
  // 정지된 서비스 인지 체크하는 함수
  // 정지되었을 경우 1, 동작중인 경우 0, 기타 실패의 경우(타임아웃) -1 을 리턴한다.
  int32_t IsStoppedService();

  // varialbes control
  VOID set_flag_notice(BOOL flag_notice) { flag_notice_ = flag_notice; }
  VOID set_timeout(DWORD timeout) { timeout_ = timeout; }
  BOOL flag_notice()  {return flag_notice_;}
  DWORD last_error_number() const { return last_error_number_; }
  DWORD timeout() const { return timeout_; }
  std::string error_message() const { return error_message_; }

 private:
  // 서비스 상태에서 제공하는 힌트타임에 대해서 유효한 시간값으로 계산하는 함수
  DWORD GetHintTimeCalculationResult_(DWORD hint_time);
  // ServiceControl 클래스내에 에러를 원하는 에러를 저장한다.
  // error_number 가 0 인 경우 GetLastError() 값과 해당하는 에러 문자열을 저장한다.
  VOID SetErrorInfo_(DWORD error_number = 0, std::string error_string = "");

 protected:
  DWORD         timeout_;
  DWORD         last_error_number_;
  BOOL          flag_notice_;
  TCHAR          service_name_[512];
  TCHAR          service_description_[256];
  TCHAR          path_binary_[MAX_PATH];
  std::string   error_message_;
};
} //namespace gstd
#endif