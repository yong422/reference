#ifndef GSTD_WINDOWS_DIRECTORY_CONTROL_H
#define GSTD_WINDOWS_DIRECTORY_CONTROL_H

#include <string>
#include <cstdint>

// TODO: gstd/windows 의 경우 현재 gstd 만 추가된 namespace 에 windows 를 추가하는 작업 필요
//       1차적으로 새로이 생성되는 코드들에 대해서 gstd::windows namespace 를 적용
namespace gstd {

namespace windows {

// 실제 파일 속성의 체크가 가능한 항목에 대해서만 설정
// Unknown 의 경우 기본값이며, 실패된 경우의 값
enum FileAttributes {
  kUnknown      = 0,   // 기본값
  kIsDirectory  = 1,   // 디렉토리
  kIsFile       = 2    // 파일
};

// @class FileApiResult
// @brief directory_control 내부 함수에서 사용하는 결과 클래스
//        
class FileApiResult {

public:
  enum Error {
    kSystemErrorCodes       = -6,   // windows error 확인필요
    kMaxPathLengthExceeded  = -5,   // 지원하는 최대 경로길이 초과
    kIsNotDirectory         = -4,   // 디렉토리가 아니다
    kIsNotEmpty             = -3,   // 비어있지 않은 디렉토리
    kIsNotFound             = -2,   // 파일 없음.
    kIsEmpty                = -1,   // 찾을수 없음.
    kSuccess                = 0     // 성공 (default)
  };

  FileApiResult() {
    Reset();
  }
  ~FileApiResult() {}

  void Reset() {
    file_attributes_ = FileAttributes::kUnknown;
    file_api_error_ = FileApiResult::Error::kSuccess;
    windows_last_error_ = ERROR_SUCCESS;
  }
  void set_file_attributes(const FileAttributes& file_attributes) {
    file_attributes_ = file_attributes;
  }
  void set_windows_last_error(DWORD windows_last_error) {
    windows_last_error_ = windows_last_error;
  }
  void set_file_api_error(const FileApiResult::Error& file_api_error) {
    file_api_error_ = file_api_error;
  }

  FileAttributes        file_attributes()     const { return file_attributes_; }
  DWORD                 windows_last_error()  const { return windows_last_error_; }
  FileApiResult::Error  file_api_error()      const { return file_api_error_; }

private:
  FileAttributes        file_attributes_;     // 파일의 속성값
  FileApiResult::Error  file_api_error_;      // FileApi 호출 에러 결과
  DWORD                 windows_last_error_;  // Error::kUnknown 일 경우 상세한 윈도우 에러번호
};

// path 에 해당하는 파일의 속성을 가져온다 성공시 true,  실패면 false 를 리턴한다.
// true 일 경우 FileApiResult::file_attributes() 확인
// false 일 경우 FileApiResult::file_api_error() 참조
//   FileApiResult::file_api_error() == FileApiResult::Error::kSystemErrorCodes 일 경우
//   윈도우 시스템 에러코드 확인 -> FileApiResult::windows_last_error()
bool CollectFileAttributes(const std::string& path, FileApiResult& file_api_result);

// 디렉토리를 삭제한다. 성공시 true, 실패시 false
// is_recursive = true 일 경우 하위 디렉토리 및 파일 전체 삭제
// return false 일 경우 FileApiResult 값 참조
// FileApiResult::windows_last_error() 의 윈도우즈 에러 코드 참조
bool DeleteDirectory(const std::string& path, const bool is_recursive, FileApiResult& file_api_result);

// 디렉토리인지 체크한다. 디렉토리면 true, 아니면 false
// false 일 경우
// FileApiResult::file_api_error() 값이 FileApiResult::Error::kSuccess 인 경우
//  파일속성 FileApiResult::file_attributes() 참조.
bool IsDirectory(const std::string& path, FileApiResult& file_api_result);

} // namespace windows

// 현재 파일의 절대경로(파일제외)를 스트링으로 받는함수
std::string GetAbsolutePath();
// 현재경로로 부터 변수로받은 상대경로로 이동하여 디렉토리 설정
std::string SetCurrentPath(std::string relative_path);
// 현재의 경로를 절대경로 스트링으로 받는함수
std::string GetCurrentPath();
// 변수로 받은 파일의 절대경로에 해당하는 파일의 사이즈를 리턴하는 함수.
int64_t GetFileSize(std::string file_name);

} //namespace gstd

#endif  // GSTD_WINDOWS_DIRECTORY_CONTROL_H