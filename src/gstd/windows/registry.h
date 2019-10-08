#ifndef GSTD_WINDOWS_REGISTRY_H
#define GSTD_WINDOWS_REGISTRY_H
// c
#include <stdio.h>
// cpp
#include <iostream>
#include <string>
#include <map>
#include <cstdint>
// windows
#include <Windows.h>
#include <Winerror.h>
#include <WinReg.h>

namespace gstd{

/**
  @brief  Windows registry 키값 접근을 위한 클래스.
  @namespace  gstd
*/
class Registry{
 public:
  Registry();
  ~Registry();
  // v1
  LPCSTR GetErrorString() { return last_error_; }
  // v2
  const char* last_error() const { return last_error_; }
  LONG last_error_number() const { return last_error_number_; }

  VOID Close();
  BOOL IsKey(HKEY key=HKEY_CURRENT_USER, LPCSTR subkey=NULL);
  BOOL Create(HKEY key=HKEY_CURRENT_USER, LPCSTR subkey=NULL);
  BOOL Open(HKEY key=HKEY_CURRENT_USER, LPCSTR subkey=NULL);
  BOOL GetValue(LPCSTR key, LPSTR strRet, DWORD length);  //! REG_SZ
  BOOL GetValue(LPCSTR key, DWORD& dwRet);        //! REG_DWORD
  BOOL SetValue(LPCSTR key, LPCSTR strVal, DWORD length);
  BOOL SetValue(LPCSTR key, DWORD val);
  BOOL DeleteValue(LPCSTR value_name);
  // 접근할 registry key 에대해 Open 이 필요
  std::int32_t  GetValueList(std::map<std::string, DWORD>& value);
  std::int32_t  GetValueList(std::map<std::string, std::string>& value);

 private:
  BOOL SetValue_(LPCSTR key, DWORD type, LPBYTE lpData, DWORD size);
  BOOL GetValue_(LPCSTR key, LPBYTE lpData, DWORD size);
  std::int32_t  GetValueList_(DWORD number_of_value, std::map<std::string, DWORD>& value);
  BOOL CreateBuffer_(DWORD len);

 private:
  // c++11 class 선언초기화 삭제. 이하버전 연동위해 생성자 초기화만 사용
  HKEY handle_key_;
  LONG last_error_number_;
  CHAR last_error_[2048];
  CHAR* set_key_string_;
  DWORD set_key_length_;
};

typedef Registry CRegistry;

}
#endif