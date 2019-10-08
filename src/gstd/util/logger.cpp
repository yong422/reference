#include "logger.h"

#include <string>
#include <string.h>
#include <stdio.h>

#define __FACILITY_CODES(x) (x<<3)

namespace gstd {

uint32_t Logger::buffer_size_ = GSTD_LOGGER_DEFAULT_BUFFER_SIZE;
pthread_mutex_t Logger::syslog_mutex_ = PTHREAD_MUTEX_INITIALIZER;
uint32_t Logger::syslog_facility_ = 0;

std::string __PrettyFunction(std::string pretty_function)
{
  size_t colons = pretty_function.find("::");
  size_t begin = pretty_function.substr(0,colons).rfind(" ") + 1;
  size_t end = pretty_function.rfind("(") - begin;

  return pretty_function.substr(begin,end);// + "()"; 
}

void Logger::InitSyslogInstance(const char* process_name, 
                                genum::logger::LogSeverity minimum_severity,
                                genum::logger::SysLogFacility set_facility)
{
  buffer_size_ = GSTD_LOGGER_DEFAULT_BUFFER_SIZE;
  setlogmask(LOG_UPTO(minimum_severity));
  syslog_facility_ = static_cast<uint32_t>(__FACILITY_CODES(set_facility));
  if (process_name) {
    openlog(process_name, LOG_CONS|LOG_NDELAY|LOG_PID, syslog_facility_);
  } else {
    openlog("gstd::Logger", LOG_CONS|LOG_NDELAY|LOG_PID, syslog_facility_);
  }
  pthread_mutex_init(&syslog_mutex_, NULL);
}

// syslog close
void Logger::ExitSyslogInstance()
{
  closelog();
}

// @brief 해당 변수에 맞는 syslog 를 기록한다.
// @params  LogSeverity severity 기록할 로그의 severity
// @params  const char* pretty_function 호출되는 위치의 함수 매크로 __PRETTY_FUNCTION__ 
//                                      매크로를 직접 주거나 제공하는 매크로로 대체
// @params  uint32_t    line_number 로그가 기록되는 줄번호. 매크로 __LINE__
// @params  const char* message     기록될 로그내용
// @params  ...                     로그내용에 포함될 가변인자
void Logger::Syslog(genum::logger::LogSeverity severity, const char* pretty_function, 
                    uint32_t line_number, const char* message, ... )
{
  std::string func_name = __PrettyFunction(pretty_function);
  uint32_t buffer_size = buffer_size_;

  if (buffer_size > 2) {
    char* buffer = new char[buffer_size];
    if (buffer) {
      va_list args;
      va_start(args, message);
      memset(buffer, 0x00, buffer_size);
      vsnprintf(buffer, buffer_size-1, message, args );
      va_end(args);
      buffer[buffer_size] = '\0';
      pthread_mutex_lock(&syslog_mutex_);
      syslog(severity | syslog_facility_ , "[%s@%d] %s\n",
            func_name.c_str(), line_number, buffer);
      pthread_mutex_unlock(&syslog_mutex_);
      delete[] buffer;
      buffer = NULL;
    }
  }
}

} // namespace gstd
