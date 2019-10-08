#include <ctime>
#include <iostream>
#include <locale>
#include "gstd/util/datetime.h"

// 여러 함수에서 std::tm 을 생성하기 위한 공통 매크로
#define MACRO_INITIALIZE_BUFFER_AND_TM(BUFFER_NAME, BUFFER_SIZE, VARIABLE_NAME, ARG_TIMESTAMP)\
  char BUFFER_NAME[BUFFER_SIZE] = {0, };\
  time_t time_value = static_cast<time_t>(ARG_TIMESTAMP);\
  std::tm VARIABLE_NAME;\
  memcpy(&VARIABLE_NAME, localtime(&time_value), sizeof(VARIABLE_NAME));\

namespace gstd {
namespace util {

const char* g_kKorDayOfTheWeek[] = {"일", "월", "화", "수", "목", "금", "토"};
const char* g_kEngDayOfTheWeek[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char* g_kMonthName[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", 
                              "Sep", "Oct", "Nov", "Dec"};

std::string DateTime::TimestampToString(uint32_t timestamp, Format datetime_format)
{
  std::string result = "";
  switch (datetime_format) {
    case Format::kIso8601 :
      result = TimestampToString(timestamp, "%FT%T%z");
      break;
    case Format::kSyslog :
      result = TimestampToStringExternalFormat_(timestamp, datetime_format);
      break;
    case Format::kStandard : 
      result = TimestampToString(timestamp, "%c");
      break;
    case Format::kKoreanSimple : 
      result = TimestampToStringExternalFormat_(timestamp, datetime_format);
      break;
    case Format::kKorean : 
      result = TimestampToStringExternalFormat_(timestamp, datetime_format);
      break;
    default:
      break;
  }
  return result;
  

}
std::string DateTime::TimestampToString(uint32_t timestamp, const char* format)
{
  MACRO_INITIALIZE_BUFFER_AND_TM(buffer, 32, tm_data, timestamp)
  // strftime 호출시 버퍼부족 또는 기타 오류시 0을 리턴하며 buffer 값은 null
  std::strftime(buffer, sizeof(buffer) -1, format, &tm_data);
  return std::string(buffer);
}

std::string DateTime::ToString(const char* format)
{
  uint32_t timestamp = (!timestamp_ ? static_cast<uint32_t>(std::time(nullptr)) : timestamp_);
  return TimestampToString(timestamp, format);
}

std::string DateTime::ToString(Format datetime_format)
{
  uint32_t timestamp = (!timestamp_ ? static_cast<uint32_t>(std::time(nullptr)) : timestamp_);
  return TimestampToString(timestamp, datetime_format);
}


std::string DateTime::ToStringFormatIso8601()
{
  return ToString(Format::kIso8601);
}

std::string DateTime::ToStringFormatStandard()
{
  return ToString(Format::kStandard);
}

std::string DateTime::ToStringFormatSyslog()
{
  return ToString(Format::kSyslog);
}

std::string DateTime::ToStringFormatKorean()
{
  return ToString(Format::kKorean);
}

std::string DateTime::ToStringFormatKoreanSimple()
{
  return ToString(Format::kKoreanSimple);
}

// private



std::string DateTime::TimestampToStringExternalFormat_(uint32_t timestamp, Format datetime_format)
{
  MACRO_INITIALIZE_BUFFER_AND_TM(buffer, 64, tm_data, timestamp)
  switch (datetime_format) {
    case Format::kSyslog :
      snprintf(buffer, sizeof(buffer)-1, "%s %2d %02d:%02d:%02d",
              g_kMonthName[tm_data.tm_mon], tm_data.tm_mday, tm_data.tm_hour, 
              tm_data.tm_min, tm_data.tm_sec);
      break;
    case Format::kKoreanSimple : 
      snprintf(buffer, sizeof(buffer)-1, "%4d-%02d-%02d(%s) %02d:%02d:%02d",
              tm_data.tm_year +1900,tm_data.tm_mon  +1,tm_data.tm_mday, 
              g_kKorDayOfTheWeek[tm_data.tm_wday], tm_data.tm_hour, 
              tm_data.tm_min, tm_data.tm_sec);
      break;
    case Format::kKorean : 
      snprintf(buffer, sizeof(buffer)-1, "%4d년 %d월 %d일 %s요일 %s %d:%02d:%02d",
              tm_data.tm_year+1900, tm_data.tm_mon+1, tm_data.tm_mday, 
              g_kKorDayOfTheWeek[tm_data.tm_wday], (tm_data.tm_hour >= 12 ? "오후":"오전"),
              (tm_data.tm_hour > 12 ? tm_data.tm_hour - 12 : tm_data.tm_hour), 
              tm_data.tm_min, tm_data.tm_sec);
      break;
    default:
      break;
  }
  return std::string(buffer);
}


} // namespace util
} // namespace gstd
