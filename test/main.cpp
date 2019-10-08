#include <gtest/gtest.h>

// reference gtest 메인함수
// linux, windows 의 모든 gtest 는 해당 메인 함수를 포함하여 사용한다.
// windows 의 crtdbg 등의 추가가 필요한 경우 해당 main 에 작업
int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

#if defined(_WITH_GMOCK)
  ::testing::InitGoogleMock(&argc, argv);
#endif

  return RUN_ALL_TESTS();
}
