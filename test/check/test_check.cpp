// src/gstd/check 의 테스트 코드

#include <gtest/gtest.h>
#include "gstd/check/filter.h"
#include "gstd/check/version.h"

class TestCheck : public ::testing::Test {
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


// filter test
TEST_F(TestCheck, TestFilter)
{
  using namespace gstd::check;
  EXPECT_FALSE(Filter::IsExistTimeRange(10,30, 16,30, 20,0));
  EXPECT_FALSE(Filter::IsExistTimeRange(10,30, 2,30, 4,30));
  EXPECT_TRUE(Filter::IsExistTimeRange(22,30, 4,30, 3,0));
  EXPECT_TRUE(Filter::IsExistTimeRange(3,30, 5,30, 4,0));

  EXPECT_FALSE(Filter::IsExistTimeRange(10,30, 23,30, 0,0));
  EXPECT_FALSE(Filter::IsExistTimeRange(22,30, 5,30, 12,0));
  EXPECT_TRUE(Filter::IsExistTimeRange(18,0, 22,0, 18,9));
  EXPECT_TRUE(Filter::IsExistTimeRange(18,0, 22,0, 18,18));
  EXPECT_TRUE(Filter::IsExistTimeRange(18,0, 18,5, 18,1));
  EXPECT_TRUE(Filter::IsExistTimeRange(18,5, 18,0, 18,18));
  EXPECT_FALSE(Filter::IsExistTimeRange(18,5, 18,0, 18,3));
  EXPECT_FALSE(Filter::IsExistTimeRange(1,0, 18,0, 22,0));
  EXPECT_TRUE(Filter::IsExistTimeRange(18,0, 18,0, 6,0));
  EXPECT_FALSE(Filter::IsExistTimeRange(18,0, 22,0, 8,0));
  EXPECT_TRUE(Filter::IsExistTimeRange(8,0, 18,0, 10,0));
  EXPECT_FALSE(Filter::IsExistTimeRange(8,0, 18,0, 5,0));
  EXPECT_FALSE(Filter::IsExistTimeRange(8,0, 18,0, 20,0));
}