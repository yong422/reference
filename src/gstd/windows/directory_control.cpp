#include <iostream>
#include <windows.h>
#include <Shlwapi.h>
#include <tchar.h>
#include <string>
#include "gstd/windows/strutil.h"
#include "gstd/windows/directory_control.h"


// PathRemoveFileSpecA 를 참조하기 위해서는 shlwapi.lib 라이브러리를 링크해야한다.

namespace gstd {
namespace windows {

bool CollectFileAttributes(const std::string& path, FileApiResult& file_api_result)
{
  DWORD file_type = INVALID_FILE_ATTRIBUTES;
  DWORD last_error = 0;
  file_api_result.Reset();
  if (path.length() > MAX_PATH) {
    file_api_result.set_file_api_error(FileApiResult::Error::kMaxPathLengthExceeded);
    return false;
  }

  file_type = ::GetFileAttributes(path.c_str());
  if (file_type == INVALID_FILE_ATTRIBUTES) { // 0xffffffff
    if ((last_error = GetLastError()) == ERROR_FILE_NOT_FOUND ||
      last_error == ERROR_PATH_NOT_FOUND) {
      // 존재하지 않는 경로
      file_api_result.set_file_api_error(FileApiResult::Error::kIsNotFound);
    } else {
      file_api_result.set_file_api_error(FileApiResult::Error::kSystemErrorCodes);
      file_api_result.set_windows_last_error(last_error);
    }
    return false;
  } else if (file_type & FILE_ATTRIBUTE_DIRECTORY) {
    file_api_result.set_file_attributes(FileAttributes::kIsDirectory);
  } else {
    file_api_result.set_file_attributes(FileAttributes::kIsFile);
  }
  return true;
}

// 참조 https://docs.microsoft.com/ko-kr/windows/desktop/FileIO/file-attribute-constants
// 해당 path 가 디렉토리인지 체크한다.
// 디렉토리 일 경우 true,  아닌 경우 false 를 리턴한다.
// TODO: SimpleZip 의 IsDirectory 함수를 directory_control 로 이전하며 이후 SimpleZip 에서
//       IsDirectory 사용을 해당 함수로 변경 한다.
bool IsDirectory(const std::string& path, FileApiResult& file_api_result)
{
  if (CollectFileAttributes(path, file_api_result)) {
    if (file_api_result.file_attributes() == FileAttributes::kIsDirectory) {
      return true;
    }
    // CollectFileAttributes 함수의 true 리턴, 디렉토리가 아닐 경우
    // path 값은 파일이므로 에러코드 변경
    file_api_result.set_file_api_error(FileApiResult::Error::kIsNotDirectory);
  }
  return false;
}

// 참조: https://docs.microsoft.com/ko-kr/windows/desktop/api/shellapi/nf-shellapi-shfileoperationa
// 현재는 ANSI 기준으로 MAX_PATH 사이즈 만큼의 경로길이 제한을 둔다.
bool DeleteDirectory(const std::string& path, const bool is_recursive, FileApiResult& file_api_result)
{
  bool result = false;
  DWORD get_last_error = 0;
  // 버퍼의 경우 double null 추가를 위해 MAX_PATH + 2
  char delete_directory_path[MAX_PATH + 2] = { 0, };

  // 디렉토리만 삭제를 위한 체크
  if ((result = IsDirectory(path, file_api_result)) == false) {
    return result;
  }
  file_api_result.Reset();
  if (!is_recursive) {
    if (RemoveDirectory(path.c_str())) {
      result = true;
    } else {
      result = false;
      if ((get_last_error = GetLastError()) == ERROR_DIR_NOT_EMPTY) {
        file_api_result.set_file_api_error(FileApiResult::Error::kIsNotEmpty);
      } else {
        file_api_result.set_windows_last_error(get_last_error);
        file_api_result.set_file_api_error(FileApiResult::Error::kSystemErrorCodes);
      }
    }
  } else {  // 하위 전체 삭제
#if _MSC_VER >= 1900
    strncpy_s(delete_directory_path, sizeof(delete_directory_path) - 2, path.c_str(), path.length());
#else
    strncpy(delete_directory_path, path.c_str(), sizeof(delete_directory_path) - 2);
#endif
    delete_directory_path[strlen(delete_directory_path) + 1] = NULL;
    // SHFILEOPSTRUCTA 에서 사용하는 경로 버퍼의 경우 하나이상의 소스에 대해 추가가 가능하다.
    // 각 개별 소스를 null 로 구분하고 마지막 종료를 double null 로 구분하므로 string 의 마지막은 double null 설정
    // 하나의 
    // TODO: 추후 여러 소스에 대한 삭제 처리 추가.

    SHFILEOPSTRUCTA shfile_op = {
      NULL,
      FO_DELETE,  // 삭제모드
      delete_directory_path,
      NULL,
      // 파일 삭제에 대해서 silent 모드로 실행
      // 관련 팝업을 띄우지 않는다.
      FOF_SILENT | FOF_NOERRORUI | FOF_NOCONFIRMATION,
      FALSE,
      NULL,
      NULL
    };

    if (!SHFileOperation(&shfile_op)) {
      if (!shfile_op.fAnyOperationsAborted) {
        result = true;
      }
    } else {
      result = false;
      file_api_result.set_windows_last_error(GetLastError());
    }
  }
  return result;
}

} // namespace windows

// 현재 위치한 절대경로를 가져온다
std::string GetAbsolutePath()
{
  std::string string_buffer;
  TCHAR path_buffer[MAX_PATH]={0,};
  GetModuleFileName(NULL, path_buffer, MAX_PATH);
  PathRemoveFileSpecA(path_buffer);
  string_buffer.assign(path_buffer);
  return string_buffer;
}

// 현재경로에서 + relative_path 에 해당하는 디렉토리로 경로를 설정한다.
std::string SetCurrentPath(std::string relative_path)
{
  std::string string_buffer;
  TCHAR path_buffer[MAX_PATH]={0,};
  GetModuleFileName(NULL, path_buffer, MAX_PATH);
  PathRemoveFileSpecA(path_buffer);
  string_buffer.assign(path_buffer);
  string_buffer = string_buffer + "\\" + relative_path;
  SetCurrentDirectory(_T(string_buffer.c_str()));
  return string_buffer;
}

std::string GetCurrentPath()
{
  TCHAR buffer[MAX_PATH] = {0,};
  GetCurrentDirectory(sizeof(buffer)-1, buffer);
  return std::string(buffer);
}


// 참조 https://msdn.microsoft.com/query/dev15.query?appId=Dev15IDEF1&l=KO-KR&k=k(FILEAPI%2FCreateFile);k(CreateFile);k(DevLang-C%2B%2B);k(TargetOS-Windows)&rd=true 
// 해당 파일의 파일사이즈를 가져온다.
int64_t GetFileSize(std::string file_name)
{
  int64_t result = -1;
  if (!file_name.empty()) {
    HANDLE handle_file = CreateFile(file_name.c_str(),
      GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
      OPEN_EXISTING, FILE_READ_ATTRIBUTES, NULL);  //FILE_ATTRIBUTE_NORMAL
    if (handle_file == INVALID_HANDLE_VALUE) {
    } else {
      LARGE_INTEGER file_size;
      
      if (!GetFileSizeEx(handle_file, &file_size)) {
        // 가져오기 실패.
        std::cout << "failed get file size" << std::endl;
      } else {
        result = static_cast<int64_t>(file_size.QuadPart);
      }
      CloseHandle(handle_file);
    }
  }
  return result;
}



} // namespace gstd