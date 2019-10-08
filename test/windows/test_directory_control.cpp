// directory_control 테스트코드
#include <Windows.h>
#include <gtest/gtest.h>

#include "gstd/windows/strutil.h"
#include "gstd/windows/directory_control.h"
#include "gstd/util/filetool.h"

class TestDirectoryControl : public ::testing::Test {
  virtual void SetUp() {
    test_dir = gstd::GetCurrentPath() + "\\testdir1";
    test_file = test_dir + "\\testfile";
    test_not_exists = test_dir + "\\notexists";
    test_max_path = test_dir;
    for (int i = 0; i < MAX_PATH; i++) {
      test_max_path.append("a");
      if (i % 30) test_max_path.append("\\");
    }
  }
  virtual void TearDown() {
  }
public:
  std::string test_dir;
  std::string test_file;
  std::string test_not_exists;
  std::string test_max_path;
  gstd::windows::FileApiResult api_result;
  gstd::util::FileTool file_tool;
};

TEST_F(TestDirectoryControl, GetFileAttributesTest)
{
  OutputDebugString("Running path : ");
  OutputDebugString(gstd::GetCurrentPath().c_str());
  OutputDebugString("\n");
  // 테스트용 디렉토리, 파일 생성
  EXPECT_TRUE(CreateDirectory(test_dir.c_str(), NULL));
  EXPECT_TRUE(file_tool.Open(test_file.c_str(), "wb+"));
  EXPECT_TRUE(file_tool.Write("my test text\n", strlen("my test text\n")));
  file_tool.Close();
  // 디렉토리 체크
  EXPECT_TRUE(gstd::windows::CollectFileAttributes(test_dir, api_result));
  EXPECT_EQ(gstd::windows::FileAttributes::kIsDirectory, api_result.file_attributes());
  // 파일 체크
  EXPECT_TRUE(gstd::windows::CollectFileAttributes(test_file, api_result));
  EXPECT_EQ(gstd::windows::FileAttributes::kIsFile, api_result.file_attributes());
  // 없는경로 체크
  EXPECT_FALSE(gstd::windows::CollectFileAttributes(test_not_exists, api_result));
  EXPECT_EQ(gstd::windows::FileAttributes::kUnknown, api_result.file_attributes());
  EXPECT_EQ(gstd::windows::FileApiResult::Error::kIsNotFound, api_result.file_api_error());

  api_result.Reset();
  // max path 최대사이즈 오류체크
  EXPECT_FALSE(gstd::windows::CollectFileAttributes(test_max_path, api_result));
  EXPECT_EQ(gstd::windows::FileAttributes::kUnknown, api_result.file_attributes());
  EXPECT_EQ(gstd::windows::FileApiResult::Error::kMaxPathLengthExceeded,
            api_result.file_api_error());
  // 파일 디렉토리 삭제
  EXPECT_TRUE(DeleteFile(test_file.c_str()));
  EXPECT_TRUE(RemoveDirectory(test_dir.c_str()));
}

TEST_F(TestDirectoryControl, IsDirectoryTest)
{
  // windows 의 경우
  // build\VC15\unit_gtest_inspector
  // linux 의 경우 cmake/build

  // directory 정상 체크
  EXPECT_TRUE(CreateDirectory(test_dir.c_str(), NULL));
  EXPECT_TRUE(gstd::windows::IsDirectory(test_dir, api_result));
  EXPECT_EQ(gstd::windows::kIsDirectory, api_result.file_attributes());
  EXPECT_TRUE(RemoveDirectory(test_dir.c_str()));

  api_result.Reset();
  // max path 최대사이즈 오류체크
  EXPECT_FALSE(gstd::windows::IsDirectory(test_max_path, api_result));
  EXPECT_EQ(gstd::windows::FileAttributes::kUnknown , api_result.file_attributes());
  EXPECT_EQ(gstd::windows::FileApiResult::Error::kMaxPathLengthExceeded,
            api_result.file_api_error());
}

TEST_F(TestDirectoryControl, IsNotDirectoryTest)
{
  EXPECT_TRUE(CreateDirectory(test_dir.c_str(), NULL));
  EXPECT_TRUE(file_tool.Open(test_file.c_str(), "wb+"));
  EXPECT_TRUE(file_tool.Write("my test text\n", strlen("my test text\n")));
  file_tool.Close();
  // IsDirectory 함수의 파일 체크 실패
  EXPECT_FALSE(gstd::windows::IsDirectory(test_file, api_result));
  EXPECT_NE(gstd::windows::kIsDirectory, api_result.file_attributes());
  // 속성값 파일 체크
  EXPECT_EQ(gstd::windows::kIsFile, api_result.file_attributes());
  // 에러코드 체크
  EXPECT_EQ(gstd::windows::FileApiResult::Error::kIsNotDirectory,
    api_result.file_api_error());
  // 테스트 디렉토리 파일삭제
  EXPECT_TRUE(DeleteFile(test_file.c_str()));
  EXPECT_TRUE(RemoveDirectory(test_dir.c_str()));
}

TEST_F(TestDirectoryControl, DeleteDirectoryTest)
{
  EXPECT_TRUE(CreateDirectory(test_dir.c_str(), NULL));
  EXPECT_TRUE(file_tool.Open(test_file.c_str(), "wb+"));
  EXPECT_TRUE(file_tool.Write("my test text\n", strlen("my test text\n")));
  file_tool.Close();

  // 파일이 잇는 디렉토리에 대한 체크 및 윈도우 삭제 함수 실패테스트
  EXPECT_TRUE(gstd::windows::IsDirectory(test_dir, api_result));
  EXPECT_EQ(gstd::windows::kIsDirectory, api_result.file_attributes());
  EXPECT_FALSE(gstd::windows::IsDirectory(test_file, api_result));
  EXPECT_EQ(gstd::windows::kIsFile, api_result.file_attributes());
  EXPECT_FALSE(RemoveDirectory(test_dir.c_str()));
  EXPECT_EQ(ERROR_DIR_NOT_EMPTY, GetLastError());

  // 파일 삭제 실패 테스트
  EXPECT_FALSE(gstd::windows::DeleteDirectory(test_file, false, api_result));
  EXPECT_EQ(gstd::windows::FileApiResult::Error::kIsNotDirectory,
    api_result.file_api_error());
  EXPECT_FALSE(gstd::windows::DeleteDirectory(test_file, true, api_result));
  EXPECT_EQ(gstd::windows::FileApiResult::Error::kIsNotDirectory,
    api_result.file_api_error());

  // recursive = false 로 비어있지 않은 디렉토리 삭제 불가
  EXPECT_FALSE(gstd::windows::DeleteDirectory(test_dir, false, api_result));
  EXPECT_EQ(gstd::windows::FileApiResult::kIsNotEmpty, 
            api_result.file_api_error());

  // recursive 옵션으로 하위 파일 또는 디렉토리 전체삭제
  EXPECT_TRUE(gstd::windows::DeleteDirectory(test_dir, true, api_result));
  EXPECT_EQ(gstd::windows::FileApiResult::kSuccess,
            api_result.file_api_error());
}