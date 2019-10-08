
// ref
#include "gstd/windows/registry.h"
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

using namespace gstd;

#if _MSC_VER >= 1900
#define MACRO_SPRINTF sprintf_s
#else
#define MACRO_SPRINTF _snprintf
#endif

/*
  @memo
    Windows service 프로그램내에서 registry 접근시 전체 유저 공통이므로
    HKEY_CURRENT_USER (HKCU) 로 접근시
    HKEY_USERS\.DEFAULT 하위로 접근됨.
      
    HKEY_USERS\.DEFAULT 접근시 
    .DEFAULT\ 식으로 접근.
*/
Registry::Registry()
{
  handle_key_ = NULL;
  set_key_string_ = NULL;
  set_key_length_ = 0;
  last_error_number_ = 0;
  ZeroMemory(last_error_, sizeof(last_error_));
}

Registry::~Registry()
{
  if (handle_key_) {
    RegCloseKey(handle_key_);
    handle_key_ =NULL;
  }
  if (set_key_string_) {
    delete[] set_key_string_;
    set_key_string_ = NULL;
  }
  set_key_length_ = 0;
}

BOOL Registry::CreateBuffer_(DWORD len)
{
  BOOL ret=FALSE;
  if (len > 0) {
    if (set_key_string_) {
      delete[] set_key_string_;
      set_key_string_ = NULL;
    }
    set_key_string_ = new CHAR[len];
    if (set_key_string_) {
      ZeroMemory(set_key_string_, len);
      set_key_length_ = len;
      ret = TRUE;
    }
  }
  return ret;
}

/**
  HKEY 의 하위 keyPath 에 해당하는 레지스트리 키가 존재하는지 체크한다.
  키의 존재유무만 체크하고 핸들러는 close 한다.

  @return  키가 존재하면 TRUE, 없으면 FALSE 를 리턴한다.
*/
BOOL Registry::IsKey(HKEY setKey, LPCSTR keyPath)
{
  HKEY hkey = NULL;
  BOOL ret  = FALSE;
  LONG lRet = 0;
  if ((lRet = RegOpenKeyEx(setKey, keyPath, 0, KEY_READ, &hkey)) == ERROR_SUCCESS) {
    ret = TRUE;
  } else {
    TCHAR* buffer = nullptr;
    last_error_number_ = lRet;
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
      0, lRet, 0, (TCHAR*)&buffer, 0, 0);
    memset(last_error_, 0x00, sizeof(last_error_));
    strncpy_s(last_error_, sizeof(last_error_), (CHAR*)buffer, sizeof(last_error_) - 1);
    if (buffer) LocalFree(buffer);
    buffer = nullptr;
    ret = FALSE;
  }
  if(hkey)  RegCloseKey(hkey);

  return ret;
}

/**
  @brief  오픈되어있는 Registry handler 를 close 한다.
*/
VOID Registry::Close()
{
  if (handle_key_) {
    RegCloseKey(handle_key_);
  }
  handle_key_=NULL;
}

/*
  setKey 에 해당하는 key에(ex. HKEY_CURRENT_USER or HKEY_USERS) 
   keyPath 에해당하는 하위키를 오픈한다.
  원래 키가있었다면 존재하던 키를 오픈한 HKEY를 저장한다
*/
BOOL Registry::Open(HKEY setKey, LPCSTR keyPath)
{
  BOOL ret=FALSE;
  LONG lRet=0;
  if ((lRet = RegOpenKeyEx(setKey, keyPath, 0, KEY_ALL_ACCESS, &handle_key_)) == ERROR_SUCCESS) {
    ret = TRUE;
    size_t keyLen = strlen(keyPath);
    if ((keyLen> 0) && (set_key_length_ >= keyLen)) {
      ZeroMemory(set_key_string_, set_key_length_);
#if _MSC_VER >= 1900
      strncpy_s(set_key_string_, set_key_length_, keyPath, set_key_length_ - 1);
#else
      strncpy(set_key_string_, keyPath, set_key_length_-1);
#endif
    }else if((strlen(keyPath) > 0)){
      if (CreateBuffer_(keyLen+2)) {
#if _MSC_VER >= 1900
        strncpy_s(set_key_string_, set_key_length_, keyPath, set_key_length_ - 1);
#else
        strncpy(set_key_string_, keyPath, set_key_length_ - 1);
#endif
      }
    }
  } else {
    last_error_number_ = lRet;
    ZeroMemory(last_error_, sizeof(last_error_));
    MACRO_SPRINTF(last_error_, sizeof(last_error_) - 1,
      "[FAILED] RegOpenKeyEx open failed [return:%ld][error:%u]\n",
      lRet, GetLastError());
  }
  return ret;
}

/**
  @brief  setKey 에 해당하는 key에(ex. HKEY_CURRENT_USER) keyPath 에해당하는 하위 키를 생성한다.
      원래 키가있었다면 존재하던 키를 오픈한 HKEY를 저장한다.
*/
BOOL Registry::Create(HKEY setKey, LPCSTR keyPath)
{
  BOOL ret=FALSE;
  LONG lRet=0;
  DWORD dwRet=0;
  if (handle_key_) {
    RegCloseKey(handle_key_);
    handle_key_=NULL;
  }
  if ((lRet = RegCreateKeyEx(setKey, keyPath, 0, NULL, REG_OPTION_NON_VOLATILE, 
                            KEY_ALL_ACCESS, NULL,&handle_key_, &dwRet)) == ERROR_SUCCESS) {
    ret = TRUE;
    size_t keyLen = strlen(keyPath);
    if((keyLen> 0) && (set_key_length_ >= keyLen)){
      ZeroMemory(set_key_string_, set_key_length_);
#if _MSC_VER >= 1900
      strncpy_s(set_key_string_, set_key_length_, keyPath, set_key_length_ - 1);
#else
      strncpy(set_key_string_, keyPath, set_key_length_ - 1);
#endif
    } else if ((strlen(keyPath) > 0)) {
      if (CreateBuffer_(keyLen+1)) {
#if _MSC_VER >= 1900
        strncpy_s(set_key_string_, set_key_length_, keyPath, set_key_length_ - 1);
#else
        strncpy(set_key_string_, keyPath, set_key_length_ - 1);
#endif
      }
    }
  } else {
    ZeroMemory(last_error_, sizeof(last_error_));
    MACRO_SPRINTF(last_error_, sizeof(last_error_) - 1,
      "[FAILED] RegCreateKeyEx open failed [return:%ld][error:%u]\n",
      lRet, GetLastError());
  }
  return ret;
}

//https://msdn.microsoft.com/en-us/library/windows/desktop/ms724872(v=vs.85).aspx
std::int32_t Registry::GetValueList(std::map<std::string, DWORD>& value)
{
  std::int32_t result = 0;
  if (handle_key_) {
    TCHAR buffer_classname[MAX_PATH] = { 0, };
    TCHAR buffer_value[MAX_VALUE_NAME] = { 0, };
    DWORD buffer_classname_size = MAX_PATH;
    DWORD number_of_value = 0;
    DWORD longest_value_name = 0;
    DWORD longest_value_data = 0;
    DWORD size_of_security_descriptor = 0;
    DWORD size_of_value = 0;
    DWORD result_code = RegQueryInfoKey(handle_key_, buffer_classname, &buffer_classname_size,
      NULL, NULL, NULL, NULL, &number_of_value, &longest_value_name,
      &longest_value_data, &size_of_security_descriptor, NULL);
    if (result_code == ERROR_SUCCESS) {
      if (number_of_value) {
        for (DWORD index = 0; result_code = ERROR_SUCCESS, index < number_of_value; index++) {
          size_of_value = MAX_VALUE_NAME;
          result_code = RegEnumValue(handle_key_, index, buffer_value, &size_of_value,
            NULL, NULL, NULL, NULL);
          if (result_code == ERROR_SUCCESS) {
            //std::cout << "reg value name : " << buffer_value << std::endl;
            DWORD get_value = 0;
            if (GetValue(buffer_value, get_value)) {
              value.insert(std::make_pair(buffer_value, get_value));
            }
          }
        }
      }
    } else {
      ZeroMemory(last_error_, sizeof(last_error_));
      MACRO_SPRINTF(last_error_, sizeof(last_error_) - 1,
        "[ERROR] failed get RegQueryInfoKey [err:%d]\n", result_code);
    }
  } else {
    ZeroMemory(last_error_, sizeof(last_error_));
    MACRO_SPRINTF(last_error_, sizeof(last_error_) - 1, "[ERROR] Empty HKEY. Not Open\n");
  }
  return result;
}

BOOL Registry::DeleteValue(LPCSTR value_name)
{
  BOOL result = FALSE;
  LONG lRet = 0;
  if (handle_key_) {
    if ((lRet = RegDeleteValue(handle_key_, value_name)) == ERROR_SUCCESS) {
      result = TRUE;
    } else {
      ZeroMemory(last_error_, sizeof(last_error_));
      MACRO_SPRINTF(last_error_, sizeof(last_error_) - 1,
                    "[FAILED] Delete value failed[%s => %s] (ret=>%l lasterror=>%u)\n",
                    set_key_string_, value_name, lRet, GetLastError());
    }
  } else {
    ZeroMemory(last_error_, sizeof(last_error_));
    MACRO_SPRINTF(last_error_, sizeof(last_error_)-1,
              "[ERROR] Empty HKEY. Not Open\n");
  }
  return result;
}

BOOL Registry::GetValue(LPCSTR key, DWORD& ret)
{
  return GetValue_(key, (LPBYTE)&(ret), sizeof(DWORD));
}

BOOL Registry::GetValue(LPCSTR key, LPSTR strRet, DWORD length)
{
  return GetValue_(key, (LPBYTE)strRet, length);
}

BOOL Registry::GetValue_(LPCSTR key, LPBYTE lpData, DWORD size)
{
  BOOL ret=FALSE;
  if (handle_key_) {
    DWORD dwNeed=0;
    LONG lRet = 0;
    if ((lRet= RegQueryValueEx(handle_key_, key, 0, NULL, NULL, &dwNeed)) == ERROR_SUCCESS) {
      if (dwNeed > 0) {
        LPBYTE data = new BYTE[dwNeed];
        ZeroMemory(data, dwNeed);
        if ((lRet= RegQueryValueEx(handle_key_, key, 0, NULL, data, &dwNeed)) == ERROR_SUCCESS) {
          if(dwNeed <= size)  memcpy((LPVOID)lpData, (LPVOID)data, dwNeed);
          else                memcpy((LPVOID)lpData, (LPVOID)data, size-1);
          lpData[size-1] = NULL;
          ret = TRUE;
        }
        delete[] data;
      }
    }else{
    ZeroMemory(last_error_, sizeof(last_error_));
    MACRO_SPRINTF(last_error_, sizeof(last_error_) - 1, "[FAILED] Empty key[%s\\%s]\n",
      set_key_string_, key);
    }
  } else {
    ZeroMemory(last_error_, sizeof(last_error_));
    MACRO_SPRINTF(last_error_, sizeof(last_error_) - 1, "[ERROR] Empty HKEY. Not Open\n");
  }
  return ret;
}

BOOL Registry::SetValue(LPCSTR key, LPCSTR strVal, DWORD length)
{
  return SetValue_(key, REG_SZ, (LPBYTE)strVal, length);
}

BOOL Registry::SetValue(LPCSTR key, DWORD val)
{
  return SetValue_(key, REG_DWORD, (LPBYTE)&val, sizeof(DWORD));
}

BOOL Registry::SetValue_(LPCSTR key, DWORD type, LPBYTE lpData, DWORD size)
{
  BOOL ret=FALSE;
  LONG lRet=0;
  if (handle_key_) {
    if ((lRet= RegSetValueEx(handle_key_, key, 0, type, lpData, size)) == ERROR_SUCCESS) {
      ret=TRUE;
    } else {
      ZeroMemory(last_error_, sizeof(last_error_));
      MACRO_SPRINTF(last_error_, sizeof(last_error_) - 1,
                "[FAILED] Set value failed[%s => %s] (ret=>%l lasterror=>%u)\n",
                set_key_string_, key, lRet, GetLastError());
    }
  } else {
    ZeroMemory(last_error_, sizeof(last_error_));
    MACRO_SPRINTF(last_error_, sizeof(last_error_)-1,
              "[ERROR] Empty HKEY. Not Open\n");
  }
  return ret;
}


#if defined(_TEST_REGISTRY)
int _tmain(int argc, _TCHAR* argv[])
{
  gstd::Registry Reg;
  if (!Reg.IsKey(HKEY_CURRENT_USER, "Software\\Test1")) {
    fprintf(stdout, "Software\\Test1 empty\n");
  }

  if (Reg.Create(HKEY_CURRENT_USER, "Software\\Test1")) {
    if (Reg.SetValue("MYVALUE1", "my string", strlen("my string"))) {
      fprintf(stdout, "set string value success\n");
    }
    if (Reg.SetValue("MYVALUE2", (DWORD)40)) {
      fprintf(stdout, "set dword value success\n");
    }

    CHAR myvalue1[32] = {0,};
    DWORD myvalue2 = 0;
    if (Reg.GetValue("MYVALUE1", myvalue1, sizeof(myvalue1)-1)) {
      fprintf(stdout, "MYVALUE1 : %s\n", myvalue1);
    }

    if (Reg.GetValue("MYVALUE2", myvalue2)) {
      fprintf(stdout, "MYVALUE2 : %u\n", myvalue2);
    }
  }
  Reg.Close();
  Sleep(30000);
  return 1;
}
#endif