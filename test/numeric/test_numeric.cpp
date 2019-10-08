#include "gtest/gtest.h"
#if defined(_WIN32)
#include <Windows.h>
#endif
#include <errno.h>
#include <cmath>
#include "gstd/check/numeric.h"

// invalid 함수 테스트
TEST(TestNumeric, CheckInvalid) {
  double i = 0;
  double val = acos(-1.1);
  EXPECT_EQ(true, gstd::check::Numeric<double>::IsNan(val));
  // 0/0 infinite
  EXPECT_EQ(true, gstd::check::Numeric<int>::IsInfinite(i / val));
  val = std::numeric_limits<double>::infinity();
  EXPECT_EQ(true, gstd::check::Numeric<double>::IsInfinite(val));
}

// valid 함수 테스트
TEST(TestNumeric, CheckValid) {
  double i = 0;
  EXPECT_EQ(true, gstd::check::Numeric<double>::IsValid(5));
  // acos 함수의 argument 의 범위는 -1 ~ 1
  // 그외 Nan 리턴
  // errno 33
  double val = acos(-1);
  EXPECT_EQ(true, gstd::check::Numeric<double>::IsValid(val));
  val = acos(-1.1);
  EXPECT_EQ(false, gstd::check::Numeric<double>::IsValid(val));
  EXPECT_EQ(33, errno);
#if defined(_WIN32)
  EXPECT_EQ(126, GetLastError());
#endif
  EXPECT_FALSE(gstd::check::Numeric<uint32_t>::IsValid(UINT_MAX + 1));
  EXPECT_TRUE(gstd::check::Numeric<uint32_t>::IsValid(UINT_MAX));
  EXPECT_FALSE(gstd::check::Numeric<double>::IsValid(std::numeric_limits<double>::infinity()));

}

TEST(TestNumeric, TestIsNumeric)
{
  EXPECT_TRUE(gstd::check::IsNumeric("011102202"));
  EXPECT_TRUE(gstd::check::IsNumeric("111023332211102"));
  EXPECT_TRUE(gstd::check::IsNumeric("002211110000"));
  EXPECT_TRUE(gstd::check::IsNumeric("0000000011"));
  EXPECT_FALSE(gstd::check::IsNumeric("00000X0011"));
  EXPECT_FALSE(gstd::check::IsNumeric("00000X0011"));
  EXPECT_FALSE(gstd::check::IsNumeric("111110011A"));
  EXPECT_FALSE(gstd::check::IsNumeric("00O00X0011"));
  EXPECT_FALSE(gstd::check::IsNumeric("0001111-22231"));
  EXPECT_FALSE(gstd::check::IsNumeric("\\22210488\\"));
  EXPECT_FALSE(gstd::check::IsNumeric("{343433333/~fg"));
}