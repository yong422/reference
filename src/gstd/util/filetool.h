#ifndef GSTD_UTIL_FILETOOL_H
#define GSTD_UTIL_FILETOOL_H
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <string>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#if defined(_WIN32)
#include <Windows.h>
#include <io.h>
#else
#include <unistd.h>
#include <zlib.h>
#endif

#define FTOOL_DEFAULT_BUFLEN  2048

//  @class  FileTool
//  @brief  fopen, fread, fwrite 이용 파일을 처리하는 클래스.
//          파일 read/write 접근권한등 파일에 관련된 기능지원.

namespace gstd {
namespace util {

class FileTool {
public:
  FileTool(int buflen = FTOOL_DEFAULT_BUFLEN);
  ~FileTool();

  //* setter
  void SetFile(const char* file);


  const bool IsOpen() const { return (!file_) ? false : true; }
  const char* GetBuffer() const { return buffer_; }
  const int GetReadSize() const { return last_get_size_; }
  const int GetLastSize() const { return last_get_size_; }
  const unsigned long long GetFileSize() const { return file_size_; }

  // getter, setter v2
#if defined(_WIN32)
  void set_is_unicode(const bool& value) { is_unicode_ = value; }
#endif
  void set_file_name(const char* file_name);
  const char* buffer() const { return buffer_; }
  const int last_get_size() const { return last_get_size_; }
  const uint64_t file_size() const { return file_size_; }
  std::string last_error() const { return last_error_; }
  bool is_unicode() const { return is_unicode_; }

  //* execute
  void SetPermission(int mode = 0755);
  bool Open(const char* file, const char* mode, const bool is_unicode = false);
  void Close();
  int Read();
  // 텍스트를 라인단위로 읽어온다.
  int ReadLine();
  int Write(const char* msg, int len);

private:
  bool CreateBuffer_(int len);

private:
  FILE*               file_;
  bool                is_unicode_;
  char                file_name_[256];
  char*               buffer_;
  int                 buffer_length_;
  int                 last_get_size_;
  unsigned long long  file_size_;
  std::string         last_error_;
};

} // namespace util
} // namespace gstd

#endif
