#include <gtest/gtest.h>
#include "gstd/util/datetime.h"

class DateTimeTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
  }
  virtual void TearDown() {
  }
  using DTFormat = gstd::util::DateTime::Format;
};

TEST_F(DateTimeTest, TestDateTimeString)
{
  // 1568606400
  // 2019-09-16 13:00:00

  gstd::util::DateTime datetime(1568606400);
  
  EXPECT_STREQ("2019-09-16T13:00:00+0900", datetime.ToStringFormatIso8601().c_str());
  EXPECT_STREQ("Sep 16 13:00:00", datetime.ToStringFormatSyslog().c_str());
  EXPECT_STREQ("2019-09-16(월) 13:00:00", datetime.ToStringFormatKoreanSimple().c_str());
  EXPECT_STREQ("2019년 9월 16일 월요일 오후 1:00:00", datetime.ToStringFormatKorean().c_str());
  EXPECT_STREQ("Mon Sep 16 13:00:00 2019", datetime.ToStringFormatStandard().c_str());
}

TEST_F(DateTimeTest, TestZeroDateTimeString)
{
  // 0
  // 2019-09-16 13:00:00

  // timestamp 값을 0으로 설정시
  // 포맷별 datetime string 을 가져오는 함수는 내부 멤버변수가 0일 경우 현재시간값으로 가져오도록 설정.
  // timestamp 0 값은 초기화 값이며 제공하지 않는다.
  // 0 을 설정하더라도 timestamp 파라미터를 추가하여 호출하는 함수를 제외하고 내부에서 최신 시간값으로 설정.
  gstd::util::DateTime datetime(0);

  EXPECT_STRNE("1970-01-01T09:00:00+0900", datetime.ToStringFormatIso8601().c_str());
  EXPECT_STRNE("Jan  1 09:00:00", datetime.ToStringFormatSyslog().c_str());
  EXPECT_STRNE("1970-01-01(목) 09:00:00", datetime.ToStringFormatKoreanSimple().c_str());
  EXPECT_STRNE("1970년 1월 1일 목요일 오전 9:00:00", datetime.ToStringFormatKorean().c_str());
  EXPECT_STRNE("Thu Jan 1 09:00:00 1970", datetime.ToStringFormatStandard().c_str());

  // DateTime 클래스에서 timestamp to string 함수는 파라미터로 입력한 timestamp 값이 0 이어도 제한하지 않는다.
  EXPECT_STREQ("1970-01-01T09:00:00+0900",
              gstd::util::DateTime::TimestampToString(0, DTFormat::kIso8601).c_str());
  EXPECT_STREQ("Jan  1 09:00:00",
              gstd::util::DateTime::TimestampToString(0, DTFormat::kSyslog).c_str());
  EXPECT_STREQ("1970-01-01(목) 09:00:00",
              gstd::util::DateTime::TimestampToString(0, DTFormat::kKoreanSimple).c_str());
  EXPECT_STREQ("1970년 1월 1일 목요일 오전 9:00:00",
              gstd::util::DateTime::TimestampToString(0, DTFormat::kKorean).c_str());
  EXPECT_STREQ("Thu Jan  1 09:00:00 1970",
              gstd::util::DateTime::TimestampToString(0, DTFormat::kStandard).c_str());
}

TEST_F(DateTimeTest, TestInitialize)
{
  gstd::util::DateTime datetime(0);
  // 1568606400
  // 2019-09-16 13:00:00
  EXPECT_STRNE("1970-01-01T09:00:00+0900", datetime.ToStringFormatIso8601().c_str());
  EXPECT_STREQ("2019-09-16T13:00:00+0900", 
              gstd::util::DateTime::TimestampToString(1568606400, DTFormat::kIso8601).c_str());
  // 내부 timestamp 값 변경없음 테스트
  EXPECT_STRNE("2019-09-16T13:00:00+0900", datetime.ToStringFormatIso8601().c_str());
  datetime.set_timestamp(static_cast<uint32_t>(1568606400));
  EXPECT_STREQ("2019-09-16T13:00:00+0900", datetime.ToStringFormatIso8601().c_str());
  datetime.set_timestamp(std::time(nullptr));
  EXPECT_STRNE("2019-09-16T13:00:00+0900", datetime.ToStringFormatIso8601().c_str());
  EXPECT_NE(1568606400, datetime.timestamp());
}

TEST_F(DateTimeTest, TestEmptyResult)
{
  gstd::util::DateTime datetime(1568606400);
  
  EXPECT_TRUE(datetime.ToString(DTFormat(10)).empty());
  EXPECT_TRUE(datetime.ToString("").empty());
  
}