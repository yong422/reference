#include <gtest/gtest.h>
#include "gstd/util/strtool.h"


class StringToolTest : public ::testing::Test {
 protected:
  // 테스트에 필요한 설정을 설정
  // 각 테스트 케이스 시작전 호출
  virtual void SetUp() {
    test_string.clear();
  }

  // 테스트 자원 해제
  // 각 테스트 종료시 호출출
  virtual void TearDown() {

  }
  gstd::util::StringTool tool;
  std::string test_string;
};

TEST_F(StringToolTest, Replace)
{
  test_string = "dfdf-34gs-335a-34sd";
  EXPECT_LT(0, tool.Replace(test_string, "-", ""));
  EXPECT_STREQ(test_string.c_str(), "dfdf34gs335a34sd");

}

TEST_F(StringToolTest, Split)
{
  std::vector<std::string> test_vector;
  test_string = "a,b,c,d,e,f,g,h,i,j";

  EXPECT_EQ(10, tool.Split(test_string, ',', &test_vector));
  EXPECT_EQ(0, strcmp(test_vector[0].c_str(), "a"));
  EXPECT_EQ(0, strcmp(test_vector[1].c_str(), "b"));
  EXPECT_EQ(0, strcmp(test_vector[2].c_str(), "c"));
  EXPECT_NE(0, strcmp(test_vector[3].c_str(), "f"));
  EXPECT_NE(0, test_vector[4].compare("j"));
}

TEST_F(StringToolTest, StringPrintf)
{
  std::string test_case = "test format 556-power-test-6005";
  std::string test_result = gstd::util::StringTool::StringPrintf("test %s %d-power-%s-%d", "format", 556, "test", 6005);
  EXPECT_STREQ(test_case.c_str(), test_result.c_str());
  test_result = gstd::util::StringTool::StringPrintf("test %s %d-power-%s-%d", "format", 556, "test", 605);
  EXPECT_STRNE(test_case.c_str(), test_result.c_str());
}

TEST_F(StringToolTest, TestReplaceUsingDictionary)
{
  gstd::util::ReplaceDictionary replace_dictionary = {
                                                    {"from_a", "development"}, {"{company}", "Google"}, 
                                                    {"<language>", "C++"}
                                                    };
  std::string test_result_string = "C++ is one of the main development languages used by many of Google's open-source projects. As every C++ programmer knows, the language has many powerful features, but this power brings with it complexity, which in turn can make code more bug-prone and harder to read and maintain.";
  std::string result_string = "<language> is one of the main from_a languages used by many of {company}'s open-source projects. As every <language> programmer knows, the language has many powerful features, but this power brings with it complexity, which in turn can make code more bug-prone and harder to read and maintain.";
  gstd::util::StringTool::ReplaceUsingDictionary(result_string, replace_dictionary);
  EXPECT_STREQ(test_result_string.c_str(), result_string.c_str());
}