#ifndef __GSTD_LOGGER_H__
#define __GSTD_LOGGER_H__

#include <pthread.h>
#include <stdint.h>
#include <syslog.h>
#include <stdarg.h>

#define GSTD_LOGGER_DEFAULT_BUFFER_SIZE 4096

namespace gstd {
namespace genum {
// c++11 이하에서는 enum 명으로 접근이 불가능해 동일한 namespace 에 변수명 충돌이 발생될 여지가 있다.
// 별도의 logger namespace 를 추가하여 사용.
namespace logger {
  enum LogType {
    kSyslog = 0,
    kApplog = 1
  };
  // RFC 5424
  // 특정 프로세스용 facility
  // 기본제공되는 facility 를 1차 미지원
  enum SysLogFacility {
    kLocal0 = 16,
    kLocal1 = 17,
    kLocal2 = 18,
    kLocal3 = 19,
    kLocal4 = 20,
    kLocal5 = 21,
    kLocal6 = 22,
    kLocal7 = 23
  };
  enum LogSeverity {
    kEmergency = 0,
    kEmer = 0,
    kPanic = 0,
    kAlert = 1,
    kCritical = 2,
    kCrit = 2,
    kError = 3,
    kErr = 3,
    kWarning = 4,
    kWarn = 4,
    kNotice = 5,
    kInformational = 6,
    kInfo = 6,
    kDebug = 7
  };
} // namespace logger
} // namespace genum

//TODO: syslog 옵션 설정 기능
//TODO: 기본 로그 기능 rotate
//TODO: 다수의 로그 설정 및 접근제어 기능
// @brief   simple logger
//          설정된 facility 에 대한 syslog 작성
//          로그 작성은 매크로 사용
// @author  ykjo
// @warning CentOS5 기준으로 개발. c++98 , gcc 4.1.2 기준.
//          하위 호환이 가능하도록 개발
// @참조
//      Facility & Severity level(Priority) > https://en.wikipedia.org/wiki/Syslog

class Logger {

 public:
  // 사용할 syslog 에 대해서 초기화.
  // 프로세스에서 한번 호출
  static void InitSyslogInstance(const char* process_name, 
                                genum::logger::LogSeverity minimum_severity,
                                genum::logger::SysLogFacility set_facility);
  // syslog close 프로세스에서 한번 호출
  static void ExitSyslogInstance();
  // syslog wrtie function
  static void Syslog(gstd::genum::logger::LogSeverity severity, const char* pretty_function, 
                    uint32_t line_number, const char* message, ... );
                    
 private:
  // 프로세스의 syslog facility (local0 ~ local7)
  static uint32_t syslog_facility_;
  // 로그작성에 사용할 내부 버퍼 크기
  static uint32_t buffer_size_;
  // syslog 잠금 
  static pthread_mutex_t syslog_mutex_;
};

} // namespace gstd

// 로그 사용을 위한 매크로
// syslog 사용을 위한 기본 매크로
#define _GSTD_SYSLOG(level, message, ...) \
  gstd::Logger::Syslog(level, __PRETTY_FUNCTION__, __LINE__, message, ##__VA_ARGS__)

// severity 별 syslog 사용 매크로
#define _GSTD_ERR_SYSLOG(message, ...) \
  _GSTD_SYSLOG(gstd::genum::logger::kErr, message, ##__VA_ARGS__)
#define _GSTD_WARN_SYSLOG(message, ...) \
  _GSTD_SYSLOG(gstd::genum::logger::kWarn, message, ##__VA_ARGS__)
#define _GSTD_CRIT_SYSLOG(message, ...) \
  _GSTD_SYSLOG(gstd::genum::logger::kCrit, message, ##__VA_ARGS__)
#define _GSTD_INFO_SYSLOG(message, ...) \
  _GSTD_SYSLOG(gstd::genum::logger::kInfo, message, ##__VA_ARGS__)
#define _GSTD_DEBUG_SYSLOG(message, ...) \
  _GSTD_SYSLOG(gstd::genum::logger::kDebug, message, ##__VA_ARGS__)

#endif // __GSTD_LOGGER_H__
