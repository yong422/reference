#ifndef GSTD_WINDOWS_COMMON_LINUX_H
#define GSTD_WINDOWS_COMMON_LINUX_H

// 리눅스에는 있으나 윈도우즈에는 없는 함수 사용을 위한 헤더파일
#if defined(_WIN32)
#include <stdint.h>
#include <time.h>

struct timezone
{
  int  tz_minuteswest;
  int  tz_dsttime;
};

//참조 https ://gist.github.com/ugovaretto/5875385
int gettimeofday(struct timeval *tv, struct timezone *tz);
#endif // _WIN32
#endif // GSTD_WINDOWS_COMMON_LINUX_H