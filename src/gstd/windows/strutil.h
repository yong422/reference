#ifndef __GSTD_STRUTIL_H__
#define __GSTD_STRUTIL_H__
#define _USE_ATL
#include <string>
namespace gstd{

//  std::string     MultiByteToUTF8(const char* lpcszSrc);
//  std::wstring    MultiByteToUnicode(const char* lpcszSrc);
  std::string GetErrorString(DWORD windows_error_code);
  std::string UTF8ToMBCS(const char* lpcszSrc);
  std::string MBCSToUTF8(const char* lpcszSrc);
} //namespage gstd

#endif