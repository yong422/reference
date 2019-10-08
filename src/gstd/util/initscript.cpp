#include <errno.h>
/*    ref    */
#include "gstd/util/filetool.h"
#include "gstd/util/initscript.h"
#include "gstd/util/strtool.h"
#include "gstd/util/cmdexecute.h"

#define __BUFLEN 4096

namespace gstd {
namespace util {

InitScript::InitScript(const char* name, const char* installPath, _OSTYPE type)
{
  _vecInitPath.clear();
  _vecRegisterPath.clear();
  _installPath.clear();
  _pidPath.clear();
  _name.clear();
  _buffer.clear();
  _vecInitPath.push_back("/etc/init.d/");
  _vecInitPath.push_back("/etc/rc.d/init.d/");
  _type = type;
  if (type == InitScript::TYPE_REDHAT) {  
    _vecRegisterPath.push_back("/sbin/chkconfig");
    _vecRegisterPath.push_back("/usr/sbin/chkconfig");
  } else if (type == InitScript::TYPE_DEBIAN) {
    _vecRegisterPath.push_back("/usr/sbin/update-rc.d");
    _vecRegisterPath.push_back("/sbin/update-rc.d");
  }

  _installPath.assign(installPath);
  if(_installPath[_installPath.length() - 1] != '/')  _installPath += std::string("/");
  _name.assign(name);
  _isDebug = false;
}

InitScript::InitScript(std::string& name, std::string& installPath)
{
  __CPRINTF_SUCCESS(true, installPath.c_str());
  if(installPath[installPath.length() - 1] != '/'){
    installPath += std::string("/");
    __CPRINTF_SUCCESS(true, installPath.c_str());
  }
  InitScript(name.c_str(), installPath.c_str());
}

InitScript::~InitScript()
{

}

//  @brief  생성된 init.d 스크립트를 설치 및 등록.
//          Redhat 계열 : chkconfig
//          Debian 계열 :  
int InitScript::Install()
{
  int ret=0;
  CmdExecute cmd;
  for (int i=0; i < _vecInitPath.size(); i++) {
    //! init path 체크.
    if (cmd.IsFile(_vecInitPath[i].c_str())) {
      __CPRINTF_SUCCESS(_isDebug, "init path check");
      __CPRINTF_SUCCESS(_isDebug, _vecInitPath[i].c_str());
      if (cmd.CheckPermission(_vecInitPath[i].c_str(), "wrx")) {
        std::string fullPathFile = _vecInitPath[i] + _name;
        std::string createFile = _installPath + _name + std::string(".init");
        __CPRINTF_SUCCESS(_isDebug, fullPathFile.c_str());
        if (_Copy(createFile.c_str(), fullPathFile.c_str()) > 0) {
          __CPRINTF_SUCCESS(_isDebug, "copy initd script");
          if (_Register() >= 0) {
            __CPRINTF_SUCCESS(_isDebug, "chkconfig register");
          } else {
            __CPRINTF_ERROR(_isDebug, "chkconfig register");
          }
        } else {
          __CPRINTF_ERROR(_isDebug, "copy initd script");
        }
      } else {
        __CPRINTF_ERROR(_isDebug, "permission denied");
      }
    }  
  }
  return ret;
}

//  @brief  os 에 등록된 실행 스크립트를 해제한다.
//          chkconfig or  으로 등록 해제후 initd 스크립트를 삭제한다.
int InitScript::Uninstall()
{
  int ret=1;
  CmdExecute cmd;
  if (_Register(true) > 0) {
    __CPRINTF_SUCCESS(_isDebug, "uninstall initd");
  }
  for (int i=0; i < _vecInitPath.size(); i++) {
    if (cmd.IsFile(_vecInitPath[i].c_str())) {
      std::string fullPathFile = _vecInitPath[i] + _name;
      if (!unlink(fullPathFile.c_str())) {
        __CPRINTF_SUCCESS(_isDebug, "delete initd script");
        __CPRINTF_SUCCESS(_isDebug, fullPathFile.c_str());
      } else {
        __CPRINTF_ERROR(_isDebug, "delete initd script");
        __CPRINTF_ERROR(_isDebug, fullPathFile.c_str());
      }
    } else {
        __CPRINTF_SUCCESS(_isDebug, "not exist initd script");
    }
  }
  return ret;
}

//  @brief    process name, install path 에 맞는 init 스크립트 생성
//  @params
//  @return
int InitScript::Create()
{
  int ret = -1;
  std::string fullPath = _installPath + _name + std::string(".init");
  __CPRINTF_SUCCESS(true, fullPath.c_str());
  FileTool* tool = new FileTool();
  if (tool->Open(fullPath.c_str(), "wb")) {
    tool->SetPermission(0755);
    if (_WriteScriptHeader() > 0) {
      tool->Write(_buffer.c_str(), _buffer.length());
    }
    if (_WriteScriptArgs() > 0) {
      tool->Write(_buffer.c_str(), _buffer.length());
    }
    if (_WriteScriptStartFunction() > 0) {
      tool->Write(_buffer.c_str(), _buffer.length());
    }    
    if (_WriteScriptStopFunction() > 0) {
      tool->Write(_buffer.c_str(), _buffer.length());
    }      
    if (_WriteScriptRestartFunction() > 0) {
      tool->Write(_buffer.c_str(), _buffer.length());
    }  
    if (_WriteScriptTail() > 0) {
      tool->Write(_buffer.c_str(), _buffer.length());
    }
  }
  tool->Close();
  delete tool;
  return ret;
}

int InitScript::_Copy(const char* from, const char* to)
{
  int ret = 0;
  FileTool fromFile;
  FileTool toFile;
  if (fromFile.Open(from, "rb")) {
    if (toFile.Open(to, "wb")) {
      toFile.SetPermission(0755);
      while (fromFile.Read() > 0) {
        toFile.Write(fromFile.GetBuffer(), fromFile.GetReadSize());
      }
      ret=1;
      toFile.Close();
    } else {
      ret=-1;
      __CPRINTF_ERROR(_isDebug, "open failed copy file");
    }
    fromFile.Close();
  } else {
    ret=-1;
    __CPRINTF_ERROR(_isDebug, "not exist initd script");
  }
  return ret;
}

int InitScript::_Register(bool flagDel)
{  
  int ret = 0;
  CmdExecute cmd;
  std::string command;
  for (int i=0; i < _vecRegisterPath.size(); i++) {
    if (cmd.IsFile(_vecRegisterPath[i].c_str())) {
      __CPRINTF_SUCCESS(_isDebug, "register file exist");
      __CPRINTF_SUCCESS(_isDebug, _vecRegisterPath[i].c_str());
      if (cmd.CheckPermission(_vecRegisterPath[i].c_str(), "rx")) {
        __CPRINTF_SUCCESS(_isDebug, "permission check");
        if (flagDel) {
          if (_type == InitScript::TYPE_REDHAT)
            command = _vecRegisterPath[i] + " --del " + _name;
          else if (_type == InitScript::TYPE_DEBIAN)
            command = _vecRegisterPath[i] + " -f " + _name + " remove";
        } else {
          if (_type == InitScript::TYPE_REDHAT)
            command = _vecRegisterPath[i] + " --add " + _name;
          else
            command = _vecRegisterPath[i] + " " + _name + " defaults";
          //command = _vecRegisterPath[i];
        }
        break;
      } else {
        __CPRINTF_ERROR(_isDebug, "permission denied");
      }
    } else {
      __CPRINTF_ERROR(_isDebug, "register file not exist");
    }
  }  
  if (!command.empty()) {
    ret = system( command.c_str() );
    if (ret < 0) {
      __CPRINTF_ERROR(_isDebug, "command error");
      fprintf(stderr, "ret : %d errno : %d\n", ret, errno);
    }
  }
  return ret;
}


int InitScript::_WriteScriptHeader()
{
  _buffer.clear();
  char* buf = new char[__BUFLEN];
  if (buf) {
    memset(buf, 0x00, __BUFLEN);
    snprintf(buf, __BUFLEN-1,
        "#!/bin/bash\n\n"
        "### BEGIN INIT INFO\n"
        "# Provides: %s\n"
        "# Required-Start: $local_fs $remote_fs $network $syslog $time\n"
        "# Required-Stop: $local_fs $remote_fs $network $syslog $time\n"
        "# Default-Start:  2 3 4 5\n"
//        "# Default-Stop:   0 1 2 6\n"
        "# Default-Stop:   0 1 6\n"
        "# Short-Description: start and stop %s\n"
        "%s%s"
        "### END INIT INFO\n\n"
        "# chkconfig: 2345 99 10\n"
        "# probe: true\n\n",
        _name.c_str(), _name.c_str(), _description.empty() ? "" : "# Description: ",
        _description.empty() ? "" : std::string(_description + std::string("\n")).c_str());
    _buffer.assign(buf);
    delete[] buf;
  } else {
    return -1;
  }
  return 1;
}

int InitScript::_WriteScriptArgs()
{
  _buffer.clear();
  char* buf = new char[__BUFLEN];
  if (buf) {
    memset(buf, 0x00, __BUFLEN);
    snprintf(buf, __BUFLEN-1,
        "\n"
        "# Process name ( For display )\n"
        "NAME=%s\n"
        "# Daemon name, where is the actual executable\n"
        "DAEMON=%s%s\n"
        "# pid file for the daemon\n"
        "PIDFILE=%s%s.pid\n\n\n"
        "# If the daemon is not there, then exit\n"
        "[ -f $DAEMON ] || exit 5\n"
        "\n\n",
        _name.c_str(), _installPath.c_str(), _name.c_str(),
        _pidPath.c_str(), _name.c_str());
    _buffer.assign(buf);
    delete[] buf;
  } else {
    return -1;
  }
  return 1;
}

int InitScript::_WriteScriptStartFunction()
{
  _buffer.clear();
  char* buf = new char[__BUFLEN];
  if (buf) {
    memset(buf, 0x00, __BUFLEN);
    snprintf(buf, __BUFLEN-1,
        "\nstart(){\n"
        "    echo -n \"Starting $NAME :\"\n"
        "    $DAEMON --start\n"
        "    echo\n"
        "}\n\n\n"
        );
    _buffer.assign(buf);
    delete[] buf;
  } else {
    return -1;
  }
  return 1;
}

int InitScript::_WriteScriptStopFunction()
{
  _buffer.clear();
  char* buf = new char[__BUFLEN];
  if (buf) {
    memset(buf, 0x00, __BUFLEN);
    snprintf(buf, __BUFLEN-1,
        "\nstop(){\n"
        "    echo -n \"Shutting down $NAME :\"\n"
        "    $DAEMON --stop\n"
        "    sleep 1\n"
        "    $DAEMON --kill\n"
        "    echo\n"
        "}\n\n\n"
        );
    _buffer.assign(buf);
    delete[] buf;
  } else {
    return -1;
  }
  return 1;
}

int InitScript::_WriteScriptRestartFunction()
{
  _buffer.clear();
  char* buf = new char[__BUFLEN];
  if (buf) {
    memset(buf, 0x00, __BUFLEN);
    snprintf(buf, __BUFLEN-1,
        "\nrestart(){\n"
        "    stop\n"
        "    sleep 1\n"
        "    start\n"
        "}\n\n\n"
        );
    _buffer.assign(buf);
    delete[] buf;
  } else {
    return -1;
  }
  return 1;
}

int InitScript::_WriteScriptTail()
{
  _buffer.clear();
  char* buf = new char[__BUFLEN];
  if (buf) {
    memset(buf, 0x00, __BUFLEN);
    snprintf(buf, __BUFLEN-1,
        "case \"$1\" in\n"
        "    start)\n"
        "        start\n"
        "        ;;\n"
        "    stop)\n"
        "        stop\n"
        "        ;;\n"
        "    restart)\n"
        "        restart\n"
        "        ;;\n"
        "    *)\n"
        "        echo \"Usage: $NAME {start:stop:restart}\"\n"
        "        exit 2\n"
                "esac\n"
        "\n\n"
        "exit 0\n"
        );
    _buffer.assign(buf);
    delete[] buf;
  } else {
    return -1;
  }
  return 1;
}

} // namespace util
} // namespace gstd

#if defined(_TEST_INITSCRIPT)
int main(int argc, char** argv)
{
  char buf[1024] = {0,};
  getcwd(buf, 1024);
  fprintf(stdout, "ab path : %s\n", buf);
  std::string file = std::string(argv[0]);
  //! 파일명, 절대경로 설정
  //InitScript inis(file.c_str(), buf);
  InitScript inis("gabia_mond_test", "/usr/local/gabia_mond_test/bin/");
  inis.SetDebug();
  inis.SetDescription("Gabia Server monitoring service provided by daemon.");
  //! daemon이 설치된 경로에 데몬.init 스크립트를 생성한다.
  inis.Create();
  //! daemon.init 스크립트를 /etc/init.d/ 와 /etc/rc.d/init.d/ 에 복사한다.
  if(!strcmp(argv[1], "install"))
    inis.Install();
  else if(!strcmp(argv[1], "uninstall"))
    inis.Uninstall();
  return 1;
}

#endif
