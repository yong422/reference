// ykjo reference
// Copyright 2019 ykjo. All rights severved.
//
// Author: ykjo@gabia.com
// 
// https://en.cppreference.com/w/cpp/chrono/c/strftime
//

#ifndef GSTD_UTIL_DATETIME_H
#define GSTD_UTIL_DATETIME_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctime>
#include <string>

namespace gstd {
namespace util {

//  @class  gstd::util::DateTime
//  @brief  date time 과 관련된 기능을 수행하는 클래스
//          기준 시간값은 생성시 설정된 timestamp 를 이용한다.
//          timestamp 를 parameter 로 전달받는 함수는 전달 받는 timestamp 값을 이용하여 datetime string 으로 변환한다.
//          timestamp 값을 parameter 로 받지 않는 함수는 멤버변수 timestamp 를 이용한다.
//            단, 멤버변수 timestamp 가 0으로 설정된 경우 현재시간값 std::time(nullptr) 을 이용한다.
//                내부 멤버변수 timestamp_ 값은 0 설정은 초기화를 의미한다.
//
//  TODO:           
class DateTime {

public:
  enum Format {
    kStandard = 0,        // cpp standard                   ex. Mon Sep 16 13:00:00 2019
    kIso8601 = 1,         // YYYY-mm-ddThh:mm:ss+-hh:mm     ex. 2019-09-16T13:00:00+0900
    kKorean = 2,          // 년 월 일 요일 오전(오후) 시:분:초 ex. 2019년 9월 16일 월요일 오후 1:00:00
    kKoreanSimple = 3,    // 년-월-일(요일(kor)) 시:분:초     ex. 2019-09-16(월) 13:00:00
    kSyslog = 4,          // 월(eng) 일(num) 시:분:초        ex. Sep 16 13:00:00
  };

  //  설정된 시간값이 없을 경우 최신 시간값으로 설정하여 생성.
  //  0 으로 설정할 경우 내부 설정시간 값이 없다는 초기화의 의미.
  DateTime(uint32_t set_time = std::time(nullptr)) : timestamp_(set_time) {}
  ~DateTime() {}

  //  옵션에 해당하는 방식으로 timestamp 값을 datetime string 으로 변환하는 함수
  //  실패시 null(empty string)
  static std::string TimestampToString(uint32_t timestamp, Format datetime_format = Format::kIso8601);
  static std::string TimestampToString(uint32_t timestamp, const char* format="%FT%T%z");
  

  //  각 함수명에 맞는 포맷의 datetime string 을 리턴하는 함수.
  //  생성시 설정한 timestamp 를 사용하며 값이 0 일 경우 최신 시간값에 대해서 생성한다.
  // iso 8601 - 2019-09-16T13:00:00+0900
  std::string ToStringFormatIso8601();
  // cpp standard - Mon Sep 16 13:00:00 2019
  std::string ToStringFormatStandard();
  // syslog format - Sep 16 13:00:00
  std::string ToStringFormatSyslog();
  // 2019-09-16(월) 13:00:00
  std::string ToStringFormatKoreanSimple();
  // 2019년 9월 16일 월요일 오후 1:00:00
  std::string ToStringFormatKorean();

  // 설정된 시간을 std::strftime format 값에 맞는 datetime string 으로 리턴하는 함수
  std::string ToString(const char* format="%FT%T%z");
  // 설정된 시간을 Format enum type 에 맞는 datetime string 으로 리턴하는 함수
  std::string ToString(Format datetime_format = Format::kIso8601);

  // getter
  uint32_t    timestamp() const { return timestamp_; }
  // setter
  void        set_timestamp(uint32_t timestamp) { timestamp_ = timestamp; }
  void        set_timestamp(time_t timestamp)   { timestamp_ = static_cast<uint32_t>(timestamp); }

private:
  // strftime 에서 지원하지 않는 특정 datetime format string 을 생성하여 리턴하는 함수.
  // Format::kKorean, Format::kKoreanSimple, Format::kSyslog
  static std::string TimestampToStringExternalFormat_(uint32_t timestamp, Format datetime_format);

  uint32_t timestamp_;
};

} // namespace util
} // namespace gstd

#endif // GSTD_UTIL_DATETIME_H
