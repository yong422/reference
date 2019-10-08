#include "gstd/windows/common_linux.h"

#include <Windows.h>

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

// timezone 1회설정을 위한 전역 플래그
static bool g_tz_flag = false;

// 리눅스의 gettimeofday 함수와 동일한 동작
// 성공시 0, 실패시 -1 을 리턴한다.
// -1 리턴시 errno_t 를 참조하도록 한다.
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  int result = 0;
  FILETIME file_time = { 0,0 };
  uint64_t tmp_result = 0;

  if (NULL != tv) {
    GetSystemTimeAsFileTime(&file_time);
    tmp_result |= file_time.dwHighDateTime;
    tmp_result <<= 32;
    tmp_result |= file_time.dwLowDateTime;

    // epoch time (POSIX time) 으로 변환
    tmp_result -= DELTA_EPOCH_IN_MICROSECS;

    // GetSystemFileAsFileTime 은 100nanosecond 기준이므로 10 을 나누어 microsecond 로 변환
    tmp_result /= 10;

    tv->tv_sec = (long)(tmp_result / 1000000UL);
    tv->tv_usec = (long)(tmp_result % 1000000UL);
  }

  if (NULL != tz) {
    if (!g_tz_flag) {
      // TZ 의 현재 설정을 사용하여 _timezone, _daylight, _tzname 전역변수에 값을 할당.
      // 운영 체제에서 지정한 표준 시간대 정보를 사용하여 UTC 계산
      // 상세참조 https://msdn.microsoft.com/ko-kr/library/90s5c885.aspx
      _tzset();
      g_tz_flag = true;
    }
#if _MSC_VER > 1900
    long time_zone = 0;
    if (_get_timezone(&time_zone)) {
      result = -1;
    } else {
      tz->tz_minuteswest = static_cast<int>(time_zone);
      if (_get_daylight(&(tz->tz_dsttime))) {
        result = -1;
      }
    }
#else
    tz->tz_minuteswest = static_cast<int>(_timezone / 60);
    tz->tz_dsttime = _daylight;
#endif
  }

  return result;
}