#include "errno.h"
#include "gstd/util/cmdexecute.h"

namespace gstd {
namespace util {

CmdExecute::CmdExecute(int bufsize)
{
  if (bufsize>0) {
    _buf = new char[bufsize];
   _len = bufsize;
  } else {
    _buf = new char[DEFAULT_CMDEXEC_BUFSIZE];
    _len = DEFAULT_CMDEXEC_BUFSIZE;
  }
  memset(_buf, 0x00, _len);
  _file = NULL;
}

CmdExecute::~CmdExecute()
{
  if (_buf) {
    delete[] _buf;
  }
  _buf = NULL;
  //if(_file)  pclose(_file);
  if(_file)  fclose(_file);
  _file = NULL;
}

bool CmdExecute::_Open(const char* target, bool file)
{
  memset(_buf, 0x00, _len);
  if (!file)  _file = popen(target, "r");
  else        _file = fopen(target, "r");
  if (!_file)  return false;
  return true;
}

bool CmdExecute::_Close(bool file)
{
  if (_file) {
    if (file)   fclose(_file);
    else        pclose(_file);
    memset(_buf, 0x00, _len);
    _file = NULL;
  }
}

bool CmdExecute::_ReadLine()
{
  bool ret = false;
  if (_file) {
    if (_buf) {
      memset(_buf, 0x00, _len);
      if (fgets(_buf, _len, _file) != NULL)  ret = true;
    }
  }
  return ret;
}

int CmdExecute::GetStringList(const char* target, std::list<std::string>* plist, bool file)
{  
  int ret=0;
  if (plist) {
    plist->clear();
    if (_Open(target, file)) {
      while (_ReadLine()) {
        std::string instr(_buf, strlen(_buf)-1);
        plist->push_back(instr);
        ++ret;  
      }
      _Close(file);
    }
  }
  return ret;
}

int CmdExecute::GetStringVector(const char* target, std::vector<std::string>* pvec, bool file)
{
  int ret = 0;
  if (pvec) {
    pvec->clear();
    if (_Open(target, file)) {
      while (_ReadLine()) {
        std::string instr(_buf, strlen(_buf)-1);
        pvec->push_back(instr);
        ++ret;
      }
      _Close(file);
    }
  }
  return ret;
}

/**
  @breif  target 파일 또는 디렉토리의 permission 체크.
      r, w, x 각 권한을 체크하며 permission변수로받은 모든 권한이 있을경우에만
      true, 하나라도 권한이 없는경우 false
*/
#if 0
//! stat 함수를 이용하여 권한 체크시. 일반계정으로 실행시에도 root 접근권한만 체크하므로 오류가있음.
bool CmdExecute::CheckPermission(const char* target, const char* permission)
{
  if(!target || !permission)  return false;
  struct stat result;
  stat(target, &result);
  for(int i=0; i < strlen(permission) ; i++){
    if(permission[i] == 'r'){
      if(!(result.st_mode & S_IRUSR))  return false;
    }else if(permission[i] == 'w'){
      if(!(result.st_mode & S_IWUSR))  return false;
    }else if(permission[i] == 'x'){
      if(!(result.st_mode & S_IXUSR))  return false;
    }else{
      //fprintf(stderr, "permission option error (only rwx : %s)\n", permission);
      return false;
    }
  }
  return true;
}
#else
bool CmdExecute::IsFile(const char* target)
{
  if (!target)  return false;
  return !access(target, F_OK) ? true:false;
}
//! access 함수로 체크하면 실행 user 권한을 체크함.
bool CmdExecute::CheckPermission(const char* target, const char* permission)
{
  if (!target || !permission)  return false;
  for (int i=0; i < strlen(permission) ; i++) {
    if (permission[i] == 'r') {
      if(access(target, R_OK)<0)  return false;
    } else if(permission[i] == 'w') {
      if(access(target, W_OK)<0)  return false;
    } else if(permission[i] == 'x') {
      if(access(target, X_OK))  return false;
    } else {
      //fprintf(stderr, "permission option error (only rwx : %s)\n", permission);
      return false;
    }
  }
  return true;
}
#endif

} // namespace util
} // namespace gstd
