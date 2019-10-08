#include <gtest/gtest.h>
#include "gstd/container/dictionary.h"

class TestDictionary : public ::testing::Test {
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

TEST_F(TestDictionary, Push)
{
  using namespace gstd::container;
  // string key insert test
  EXPECT_EQ(1, Dictionary::Push("key1", "value1"));
  EXPECT_EQ(1, Dictionary::Push("key2", "value2"));
  EXPECT_EQ(0, Dictionary::Push("key1", "value1"));
  EXPECT_EQ(0, Dictionary::Push("key2", "value2"));

  EXPECT_EQ(1, Dictionary::Push(1, "value11"));
  EXPECT_EQ(1, Dictionary::Push(2, "value22"));
  EXPECT_EQ(1, Dictionary::Push(3, "value33"));
  EXPECT_EQ(0, Dictionary::Push(1, "value44"));
}
