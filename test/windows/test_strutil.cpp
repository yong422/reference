#include "gtest/gtest.h"
// 테스트를 위해 locale en-US 로 변경


#include <Windows.h>
#include "gstd/windows/strutil.h"
TEST(TestStrutil, Check) {
  std::string ret = gstd::GetErrorString(4);
  EXPECT_STREQ(gstd::GetErrorString(4).c_str(), "The system cannot open the file.\r\n");
}