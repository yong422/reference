#include <gtest/gtest.h>
#include "gstd/net/ipchecker.h"
#include "gstd/net/mock_ipchecker.h"

class TestIpChecker : public ::testing::Test {
 protected:
  // 테스트에 필요한 설정을 설정
  // 각 테스트 케이스 시작전 호출
  virtual void SetUp() {
    //
  }

  // 테스트 자원 해제
  // 각 테스트 종료시 호출출
  virtual void TearDown() {
    //
  }
  MockIpChecker ip_checker;
};



TEST_F(TestIpChecker, Test1)
{
  // googlemock 적용 테스트중
  EXPECT_EQ(false, ip_checker.Open("GeoIP-Country.mmdb"));
}
