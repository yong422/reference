#include <errno.h>
#include <iostream>
#include <string>
#include "gstd/util/filetool.h"

namespace gstd {
namespace util {

FileTool::FileTool(int buffer_length)
{
  is_unicode_ = false;
  file_ = NULL;
  buffer_ = NULL;
  buffer_length_ = 0;
  last_get_size_ = 0;
  file_size_ = 0;
  last_error_.clear();
  memset(file_name_, 0x00, sizeof(file_name_));
  if (!CreateBuffer_(buffer_length)) {
#if defined(_WIN32)
    std::cerr << "FILETOOL ERROR : failed memory allocate : " << GetLastError();
#else
    fprintf(stderr, "FileTool ERROR : failed memory allocate (%s:%d)\n", strerror(errno), errno);
#endif
  }
}

FileTool::~FileTool()
{
  if (buffer_) {
    delete[] buffer_;
    buffer_ = NULL;
    buffer_length_ = 0;
  }
  Close();
}

bool FileTool::CreateBuffer_(int length)
{
  bool result = false;
  if (length) {
    if (!buffer_) {
      buffer_ = new char[length];
      if (buffer_) {
        buffer_length_ = length;
        result = true;
      }
    }
  }
  return result;
}

void FileTool::SetFile(const char* file_name)
{
  set_file_name(file_name);
}

void FileTool::set_file_name(const char* file_name)
{
  if (file_name) {
    memset(file_name_, 0x00, sizeof(file_name_));
#if _MSC_VER >= 1900
    strncpy_s(file_name_, sizeof(file_name_) - 1, file_name, strlen(file_name));
#else
    strncpy(file_name_, file_name, sizeof(file_name_) - 1);
#endif
  }
}

bool FileTool::Open(const char* file_name, const char* mode, const bool is_unicode)
{
  bool result = false;
  if (file_name) {
    SetFile(file_name);
  }
#if defined(_WIN32)
  // unicode 파일 읽기기능은 Windows 만 적용하도록 함.
  set_is_unicode(is_unicode);
#endif
#if _MSC_VER >= 1900
  if (strlen(file_name_) > 0) {
    errno_t errno_result = 0;
    if ((errno_result = fopen_s(&file_, file_name, mode) != 0)) {
      std::cout << "open failed : " << GetLastError() << std::endl;
    } else {
      fseek(file_, 0L, SEEK_END);
      file_size_ = static_cast<unsigned long long>(ftell(file_));
      fseek(file_, 0L, SEEK_SET);
      result = true;
    }
  }
#else
  if (strlen(file_name_) > 0) {
    if ((file_ = fopen(file_name_, mode)) == NULL) {
      fprintf(stdout, "open failed : %s(%d)\n", strerror(errno), errno);
    } else {
      fseek(file_, 0L, SEEK_END);
      file_size_ = static_cast<unsigned long long>(ftell(file_));
      fseek(file_, 0L, SEEK_SET);
      result = true;
    }
  }
#endif
  return result;
}

void FileTool::Close()
{
  if (file_) {
    fclose(file_);
    file_ = NULL;
    memset(buffer_, 0x00, buffer_length_);
    last_get_size_ = 0;
    file_size_ = 0;
  }
}

int FileTool::Read()
{
  int size = -1;
  if (buffer_ && buffer_length_ > 0 && file_) {
    memset(buffer_, 0x00, buffer_length_);
    size = fread(buffer_, sizeof(char), buffer_length_, file_);
    last_get_size_ = size;
  }
  return size;
}

int FileTool::ReadLine()
{
  int size = -1;
  wchar_t local_buffer[2048] = { 0, };
  if (is_unicode()) { // unicode file
#if defined(_WIN32)
    // unicode 는 windows 만 적용
    if (buffer_ && buffer_length_ > 0 && file_) {
      if (fgetws(local_buffer, sizeof(local_buffer) - 1, file_) != NULL) {
        int wstr_size = WideCharToMultiByte(CP_ACP, 0, local_buffer, -1, NULL, 0, NULL, NULL);
        if (buffer_length_ < wstr_size) {
          CreateBuffer_(wstr_size + 2);
        }
        if (buffer_) {
          WideCharToMultiByte(CP_ACP, 0, local_buffer, -1, buffer_, wstr_size, NULL, NULL);
          last_get_size_ = strlen(buffer_);
          size = last_get_size_;
        }
      }
    }
#endif
  } else {  // MBCS file
    if (buffer_ && buffer_length_ > 0 && file_) {
      memset(buffer_, 0x00, buffer_length_);
      if (fgets(buffer_, buffer_length_, file_) != NULL) {
        last_get_size_ = strlen(buffer_);
        size = last_get_size_;
      }
    }
  }
  return size;
}

int FileTool::Write(const char* msg, int len)
{
  int size = -1;
  if (file_) {
    size = fwrite(msg, sizeof(char), len, file_);
  }
  return size;
}

void FileTool::SetPermission(int mode)
{
  if (strlen(file_name_) > 0) {
#if _MSC_VER >= 1900
    _chmod(file_name_, mode);
#else
    chmod(file_name_, mode);
#endif
  } else {
    fprintf(stdout, "%s file permission failed : %d\n", file_name_, mode);
  }
}

} // namespace util
} // namespace gstd

/**
  @brief  파일관련 예제.
          filename에 해당하는 파일을 copyname 명의 파일로 복사한다.
          permission은 755 rwxr-xr-x
*/
#if _TEST_FILETOOL
int main(int argc, char** argv)
{
  if (argc < 3) {
    fprintf(stdout, "Usage : %s <filename> <copyname>\n", argv[0]);
    return 1;
  }
  FileTool ftool;
  FileTool fwriteTool;
  if (ftool.Open(argv[1], "rb")) {
    fprintf(stdout, "file open success => size :%llu\n", ftool.GetFileSize());
    if (fwriteTool.Open(argv[2], "wb")) {
      fwriteTool.SetPermission(0755);
      while (ftool.Read() > 0) {
        fwriteTool.Write(ftool.GetBuffer(), ftool.GetReadSize());
      }
      fwriteTool.SetPermission(0755);
    }
    ftool.Close();
    fwriteTool.Close();
  } else {
    fprintf(stdout, "file open failed\n");
  }
}
#endif
