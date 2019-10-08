#include <gtest/gtest.h>
#include "gstd/net/ip.h"

class TestIp : public ::testing::Test {
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
};



TEST_F(TestIp, Test1)
{
  // googlemock 적용 테스트중
  uint32_t ipnum1=0, ipnum2=0;
  std::string ipaddr1, ipaddr2;
  EXPECT_EQ(256, gstd::net::Ip::GetIpaddressRange("121.78.95.0/24", ipnum1, ipnum2));
  ipaddr1 = gstd::net::Ip::InetNtoa(ipnum1);
  ipaddr2 = gstd::net::Ip::InetNtoa(ipnum2);
  EXPECT_STREQ("121.78.95.0", ipaddr1.c_str());
  EXPECT_STREQ("121.78.95.255", ipaddr2.c_str());
  EXPECT_EQ(128, gstd::net::Ip::GetIpaddressRange("121.78.95.0/25", ipnum1, ipnum2));
  EXPECT_EQ(64, gstd::net::Ip::GetIpaddressRange("121.78.95.0/26", ipnum1, ipnum2));
  EXPECT_EQ(32, gstd::net::Ip::GetIpaddressRange("121.78.95.0/27", ipnum1, ipnum2));
  EXPECT_EQ(16, gstd::net::Ip::GetIpaddressRange("121.78.95.0/28", ipnum1, ipnum2));
  EXPECT_EQ(8, gstd::net::Ip::GetIpaddressRange("121.78.95.0/29", ipnum1, ipnum2));
  EXPECT_EQ(4, gstd::net::Ip::GetIpaddressRange("121.78.95.0/30", ipnum1, ipnum2));
  EXPECT_EQ(2, gstd::net::Ip::GetIpaddressRange("121.78.95.0/31", ipnum1, ipnum2));
  EXPECT_EQ(1, gstd::net::Ip::GetIpaddressRange("121.78.95.0/32", ipnum1, ipnum2));
  EXPECT_EQ(64, gstd::net::Ip::GetIpaddressRange("121.78.95.50/26", ipnum1, ipnum2));

  EXPECT_EQ(2035179494, gstd::net::Ip::InetAton("121.78.95.230"));
  EXPECT_EQ(2035179498, gstd::net::Ip::InetAton("121.78.95.234"));

  EXPECT_STREQ("121.78.95.230", gstd::net::Ip::InetNtoa(2035179494).c_str());
  EXPECT_STREQ("121.78.95.234", gstd::net::Ip::InetNtoa(2035179498).c_str());

  EXPECT_TRUE(gstd::net::Ip::IsSpecialPurposeIp("192.168.1.1"));
  EXPECT_TRUE(gstd::net::Ip::IsSpecialPurposeIp("192.168.75.136"));
  EXPECT_TRUE(gstd::net::Ip::IsSpecialPurposeIp("192.168.122.1"));
}

TEST_F(TestIp, TestIsSpecialPurposeIp)
{
  EXPECT_TRUE(gstd::net::Ip::IsSpecialPurposeIp("192.168.1.1"));
  EXPECT_TRUE(gstd::net::Ip::IsSpecialPurposeIp("192.168.75.136"));
  EXPECT_TRUE(gstd::net::Ip::IsSpecialPurposeIp("192.168.122.1"));
  EXPECT_TRUE(gstd::net::Ip::IsSpecialPurposeIp("10.14.14.1"));
  EXPECT_TRUE(gstd::net::Ip::IsSpecialPurposeIp("10.222.222.225"));
  EXPECT_FALSE(gstd::net::Ip::IsSpecialPurposeIp("121.78.95.230"));
  EXPECT_FALSE(gstd::net::Ip::IsSpecialPurposeIp("45.55.55.22"));
  EXPECT_FALSE(gstd::net::Ip::IsSpecialPurposeIp("211.115.73.98"));
  EXPECT_TRUE(gstd::net::Ip::IsSpecialPurposeIp("127.0.0.1"));
  EXPECT_FALSE(gstd::net::Ip::IsSpecialPurposeIp("5.5.5.5"));
  EXPECT_TRUE(gstd::net::Ip::IsSpecialPurposeIp("255.255.255.255"));
}
