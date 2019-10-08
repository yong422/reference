#include "stdafx.h"
#include "gstd/windows/strutil.h"
#include <string>
#include <iostream>

namespace gstd{

// LANGID
// LANGID https://docs.microsoft.com/ko-kr/windows/desktop/Intl/language-identifier-constants-and-strings
std::string GetErrorString(DWORD windows_error_code)
{
  LPVOID message_buffer = NULL;
  std::string result_string = "";
  DWORD result = 0;
  result = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
                        | FORMAT_MESSAGE_FROM_SYSTEM
                        | FORMAT_MESSAGE_IGNORE_INSERTS
                        , NULL, windows_error_code
                        //, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
                        , MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)
                        , (LPTSTR)&message_buffer, 0, NULL);
  if (result > 0 && message_buffer != NULL) {
    result_string.assign(static_cast<char*>(message_buffer));
  } else {
    result_string.assign("Failed GetErrorString");
  }
  if (message_buffer != NULL) LocalFree(message_buffer);
  return result_string;
}

std::string UTF8ToMBCS(const char* lpcszSrc)
{
  std::string result;
  wchar_t*  string_unicode = nullptr;
  char*     string_mbcs = nullptr;

  int need_length = MultiByteToWideChar(CP_UTF8, 0, lpcszSrc, -1, NULL, 0);
  //int need_length = MultiByteToWideChar(CP_UTF8, 0, lpcszSrc, strlen(lpcszSrc), NULL, NULL);
  
  if (need_length > 0) {
    string_unicode = new wchar_t[need_length];
    ZeroMemory(string_unicode, need_length);
    MultiByteToWideChar(CP_UTF8, 0, lpcszSrc, -1, string_unicode, need_length);
  }

  need_length = WideCharToMultiByte(CP_ACP, 0, string_unicode, -1, NULL, 0, NULL, NULL);
  if (need_length > 0) {
    string_mbcs = new char[need_length];
    ZeroMemory(string_mbcs, need_length);
    if (string_mbcs) {
      WideCharToMultiByte(CP_ACP, 0, string_unicode, -1, string_mbcs, need_length, NULL, NULL);
      result.assign(string_mbcs);
    }
  }
  if (string_unicode) delete[] string_unicode;
  if (string_mbcs) delete[] string_mbcs;

  return result;
}

std::string MBCSToUTF8(const char* lpcszSrc)
{
  std::string result = "";
  wchar_t*  string_unicode = nullptr;
  char*     string_utf8 = nullptr;

  int need_length = MultiByteToWideChar(CP_ACP, 0, lpcszSrc, -1, NULL, 0);
  if (need_length > 0) {
    string_unicode = new wchar_t[need_length];
    ZeroMemory(string_unicode, sizeof(wchar_t)*need_length);
    MultiByteToWideChar(CP_ACP, 0, lpcszSrc, -1, string_unicode, need_length);
  }

  need_length = WideCharToMultiByte(CP_UTF8, 0, string_unicode, -1, NULL, 0, NULL, NULL);
  if (need_length > 0) {
    string_utf8 = new char[need_length];
    ZeroMemory(string_utf8, need_length);
    if (string_utf8) {
      WideCharToMultiByte(CP_UTF8, 0, string_unicode, -1, string_utf8, need_length, NULL, NULL);
      result.assign(string_utf8);
    }
  }
  if (string_unicode) delete[] string_unicode;
  if (string_utf8) delete[] string_utf8;

  return result;
}

} //namespace gstd
