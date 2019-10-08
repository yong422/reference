#ifndef __GSTD_UTIL_INIT_SCRIPT_H__
#define __GSTD_UTIL_INIT_SCRIPT_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <string>
#include <vector>

namespace gstd {
namespace util {

class InitScript{
public:
  enum _OSTYPE{
    TYPE_REDHAT = 0,
    TYPE_DEBIAN = 1,
    TYPE_ERROR = 2
  };
public:
  InitScript(const char* name, const char* installPath, _OSTYPE type = TYPE_REDHAT);
  InitScript(std::string& name, std::string& installPath);
  ~InitScript();
  
  void SetDebug(){_isDebug = true;}  
  void SetPidPath(std::string pidPath){_pidPath = pidPath;}
  void SetDescription(std::string descr){_description = descr;}
  int  Create();
  int Install();
  int Uninstall();
private:
  //8 Create 하위함수. initd 스크립트를 작성한다.
  int _WriteScript();
  int _WriteScriptHeader();
  int  _WriteScriptTail();
  int _WriteScriptArgs();
  int _WriteScriptStartFunction();
  int _WriteScriptStopFunction();
  int _WriteScriptRestartFunction();

  //* Install 하위함수. initd script 복사
  int _Copy(const char* from, const char* to);
  //* Install 하위함수. chkconfig(redhat) update(debian). 를 이용 init.d 에 등록
  //* flagDel = true 일경우 등록해제모드
  int _Register(bool flagDel=false);
private:
  std::string _name;
  std::string _installPath;
  std::string _pidPath;
  std::string _description;
  std::vector<std::string> _vecInitPath;
  std::vector<std::string> _vecRegisterPath;
  std::string _buffer;
  bool _isDebug;
  InitScript::_OSTYPE _type;
};

} // namespace util
} // namespace gstd

#endif // __GSTD_UTIL_INIT_SCRIPT_H__
