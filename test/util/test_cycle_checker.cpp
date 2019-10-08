#include <gtest/gtest.h>
#include "gstd/util/cycle_checker.h"


// gstd::util::CycleChecker 테스트코드

// CycleChecker 의 기본 기능 
TEST(TestCycleChecker, CheckCycle)
{
#if __cplusplus >= 201103L
  gstd::util::CycleChecker checker({ "", "tester", "checker", "checker2" });
#else
  const char* checker_string_list[] = { "", "tester", "checker", "checker2" };
  std::vector < std::string > 
    initialize_checker_list(checker_string_list, checker_string_list + sizeof(checker_string_list)/sizeof(char*));
  gstd::util::CycleChecker checker(initialize_checker_list);
#endif
  uint32_t check_time = static_cast<uint32_t>(time(NULL));
  // 초기 체크값은 0 이므로 현재시간값으로 cycle 확인시 true
  EXPECT_TRUE(checker.CheckCycle(check_time, 30, "tester"));
  // CheckCycle 로 확인된 index 의 cycle 에 대해서는 가장 최근 체크값이 설정되므로 false
  EXPECT_FALSE(checker.CheckCycle(check_time, 30, "tester"));
  check_time += 20; // 20초 지났음을 가정
  EXPECT_FALSE(checker.CheckCycle(check_time, 30, "tester"));
  check_time += 10; // 30초 지났음을 가정
  EXPECT_TRUE(checker.CheckCycle(check_time, 30, "tester"));
  check_time += 5; // 5초 지났음을 가정
  EXPECT_TRUE(checker.CheckCycle(check_time, 5, "tester"));
  // 초기값 check
  EXPECT_TRUE(checker.CheckCycle(check_time, 30, "checker"));
  EXPECT_TRUE(checker.CheckCycle(check_time, 30, "checker2"));
  // 초기 설정안된 index 에 대한 체크 오류
  EXPECT_FALSE(checker.CheckCycle(check_time, 30, "undefined_checker"));
  EXPECT_FALSE(checker.CheckCycle(check_time, 30, "error_checker"));
}

// CycleChecker 의 Rollback test
TEST(TestCycleChecker, RollbackTest)
{
#if __cplusplus >= 201103L
  gstd::util::CycleChecker checker({ "", "tester", "checker", "checker2" });
#else
  const char* checker_string_list[] = { "", "tester", "checker", "checker2" };
  std::vector < std::string > 
    initialize_checker_list(checker_string_list, checker_string_list + sizeof(checker_string_list)/sizeof(char*));
  gstd::util::CycleChecker checker(initialize_checker_list);
#endif
  uint32_t check_time = static_cast<uint32_t>(time(NULL));
  EXPECT_TRUE(checker.CheckCycle(check_time, 30, "tester"));
  check_time += 30;
  EXPECT_TRUE(checker.CheckCycle(check_time, 30, "tester"));
  EXPECT_FALSE(checker.CheckCycle(check_time, 30, "tester"));
  // 동작 실패로 재시도가 필요. 설정된 cycle rollback
  checker.Rollback("tester");
  EXPECT_TRUE(checker.CheckCycle(check_time, 30, "tester"));
  // 존재하지 않는 index 에 대한 롤백시도
  EXPECT_FALSE(checker.Rollback("undefined_checker"));
  EXPECT_TRUE(checker.CheckCycle(check_time, 300, "checker"));
  EXPECT_FALSE(checker.CheckCycle(check_time, 300, "checker"));

  // 필요한 index 가 아닌 index 에 대한 rollback
  checker.Rollback("tester");
  EXPECT_FALSE(checker.CheckCycle(check_time, 300, "checker"));
  checker.Rollback("checker");
  EXPECT_TRUE(checker.CheckCycle(check_time, 300, "checker"));
}

TEST(TestCycleChecker, DefaultSetTest)
{
  uint32_t check_time = static_cast<uint32_t>(time(NULL));
  //empty vector
  std::vector< std::string > empty_vector;
  gstd::util::CycleChecker checker(empty_vector);
  EXPECT_TRUE(checker.CheckCycle(check_time, 300, ""));
  check_time += 300;
  EXPECT_TRUE(checker.CheckCycle(check_time, 300, ""));
  EXPECT_FALSE(checker.CheckCycle(check_time, 300, ""));
}