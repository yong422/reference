#ifndef GSTD_CHECK_NUMERIC_H
#define GSTD_CHECK_NUMERIC_H

#include <iostream>     // cout
// std::max 를 윈도우에서 사용할 경우 minwindef.h 에 정의된 max, min 매크로와 충돌
// 해당 매크로를 limits 헤더 참조전 삭제 처리후 사용
#if defined(_WIN32)
#ifdef max
#undef max
#undef min
#endif
#include <limits>       // numeric_limits
#else
#include <limits.h>
#endif


namespace gstd {
namespace check {

bool IsNumeric(const std::string& value);

template< typename T >
class Numeric {
public:
  Numeric() {}
  ~Numeric() {}
  static bool IsInfinite(const T &value);
  static bool IsNan(const T &value);
  static bool IsValid(const T &value);
};

//  @brief    변수의 값이 무한대의 값인지 체크.
//  @return   infinite 값일경우 true, 아닌경우 false
template< typename T >
bool Numeric<T>::IsInfinite(const T &value)
{
  T max_value = std::numeric_limits<T>::max();
  T min_value = -max_value;
  return !(min_value <= value && value <= max_value);
}

//  @brief    변수의 값이 out of domain 인지 체크.
//  @return   Nan 인 경우 true, 아니면 false
template< typename T >
bool Numeric<T>::IsNan(const T &value)
{
  return value != value;
}


//  @breif    변수의 값이 유효한 값인지 체크
//  @return   유효한 값인경우 true, 아니면 false
template< typename T >
bool Numeric<T>::IsValid(const T &value)
{
  return (!Numeric<T>::IsInfinite(value)) && (!Numeric<T>::IsNan(value));
}

} // namespace check
} // namespace gstd
#endif
