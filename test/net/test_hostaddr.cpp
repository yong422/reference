#include <gtest/gtest.h>
#include "gstd/net/hostaddr.h"

class TestHostInfo : public gstd::net::HostInfo, public ::testing::Test {
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

TEST(TestOnlyFunction, FunctionTest)
{
  //std::list<std::string> host_addr_list;
  char ip_buffer[32] = { 0, };
  // a < b
  EXPECT_LT(0, gstd::net::get_my_hostaddr(ip_buffer, sizeof(ip_buffer)));
  EXPECT_LE(7, strlen(ip_buffer));

  memset(ip_buffer, 0x00, sizeof(ip_buffer));
  EXPECT_LT(0, gstd::net::get_my_first_interface_name(ip_buffer, sizeof(ip_buffer)));

  memset(ip_buffer, 0x00, sizeof(ip_buffer));
  EXPECT_EQ(0, gstd::net::get_my_hostname(ip_buffer, sizeof(ip_buffer)));
}

TEST_F(TestHostInfo, ClassTest)
{
  std::list<std::string> host_addr_list;
  int result = 0;
  EXPECT_LT(0, (result = GetListHostAddress(&host_addr_list)));
  EXPECT_EQ(result, host_addr_list.size());
  for (int i=0; i < result; i++) {
    EXPECT_LE(7, host_addr_list.front().length());
    std::cout << "ip : " << host_addr_list.front() << std::endl;
    host_addr_list.pop_front();
  }
}