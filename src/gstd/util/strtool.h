#ifndef __GSTD_UTIL_STRTOOL_H__
#define __GSTD_UTIL_STRTOOL_H__

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <unordered_map>

namespace gstd {
namespace util {

//  문자열을 변경하기 위한 dictionary
//  From, To
using ReplaceDictionary = std::unordered_map< std::string /*From*/ , std::string /*To*/ >;

// @class StringTool
// @brief 
//  문자열 조작관련 클래스
class StringTool{
public:
  StringTool(){}
  ~StringTool(){}
  static int Split(std::string str, const char delimiter, std::list<std::string>* plist);
  static int Split(std::string str, const char delimiter, std::vector<std::string>* pvec);
  static int Replace(char* lpszValue, int size, const char* lpszChangeVal, const char* lpszChangedVal);
  static int Replace(std::string& str, const char* lpszChangeVal, const char* lpszChangedVal);
  //  result_string 을 
  static void ReplaceUsingDictionary(std::string& result_string, 
                                      const ReplaceDictionary& replace_dictionary);
  static std::string StringPrintf(const char* format, ...);
};

typedef StringTool CStringTool;

} // namespace util
} // namespace gstd

#if !defined(__CPRINTF_SUCCESS)
#define __CPRINTF_SUCCESS(b,x)\
  if(b)  printf("%c[%d;%dm[SUCCESS]%c[%dm %s\n", 27,1,32,27,0, x);
#endif

#if !defined(__CPRINTF_ERROR)
#define __CPRINTF_ERROR(b,x)\
  if(b)  printf("%c[%d;%dm[ERROR]%c[%dm %s\n", 27,1,31,27,0, x);
#endif

#endif // __GSTD_UTIL_STRTOOL_H__

