#include <gtest/gtest.h>
#include "gstd/util/logger.h"


class LoggerTest : public ::testing::Test {
 protected:
  // 테스트에 필요한 설정을 설정
  // 각 테스트 케이스 시작전 호출
  virtual void SetUp() {

  }

  // 테스트 자원 해제
  // 각 테스트 종료시 호출출
  virtual void TearDown() {

  }
};

TEST_F(LoggerTest, Init){
  //gstd::Logger::InitSyslogInstance("test", gstd::genum::logger::kNotice, gstd::genum::logger::kLocal5);
  gstd::Logger::InitSyslogInstance("test", gstd::genum::logger::kDebug, gstd::genum::logger::kLocal5);
  _GSTD_SYSLOG(gstd::genum::logger::kWarn, "what are you doing?");
  _GSTD_SYSLOG(gstd::genum::logger::kWarn, "log test %d", 1,2,3,4,5);
  _GSTD_SYSLOG(gstd::genum::logger::kWarn, "%s : %s", "test", "test2");
  _GSTD_WARN_SYSLOG("Test Warning %s", "OK");
  _GSTD_CRIT_SYSLOG("Test Critical %s", "OK");
  _GSTD_INFO_SYSLOG("Test Info %s", "OK");
  _GSTD_DEBUG_SYSLOG("Test Debug %s", "OK");
  _GSTD_ERR_SYSLOG("Test Error %s", "OK");
}

