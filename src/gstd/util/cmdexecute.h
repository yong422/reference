#ifndef GSTD_UTIL_CMD_EXECUTE_H
#define GSTD_UTIL_CMD_EXECUTE_H

#if !defined(_WIN32)
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <map>

#ifndef DEFAULT_CMDEXEC_BUFSIZE
  #define DEFAULT_CMDEXEC_BUFSIZE 4096
#endif

namespace gstd {
namespace util {

//  @brief  linux command line 명령 실행을 이용한 결과를 받거나
//          파일의 내용을 읽어올때 사용할 클래스
// 
//  @수정
//          popen 만 사용시 file 접근시 permission denied 가 발생.
//          파일접근시 옵션으로 fopen, fclose 사용하도록 수정
class CmdExecute{
public:
  CmdExecute(int bufsize=DEFAULT_CMDEXEC_BUFSIZE);
  ~CmdExecute();
  
  int GetStringList(const char* target, std::list<std::string> *plist, bool file=false);
  int GetStringVector(const char* target, std::vector<std::string> *pvec, bool file=false);
  //! target 파일 또는 폴더의 (rwx)권한 체크.
  bool CheckPermission(const char* target, const char* permission="x");
  bool IsFile(const char* target);
  bool IsDir(const char* dir){return IsFile(dir);}
private:
  bool _Open(const char* target, bool file=false);
  bool _Close(bool file=false);
  bool _ReadLine();

private:
  FILE*   _file;
  char*   _buf;
  int     _len;
};

} // namespace util
} // namespace gstd
#endif // __GSTD_UTIL_CMD_EXECUTE_H__
