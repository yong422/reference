reference/windows 하위프로젝트
=============================

# Description

- windows only 로 사용되는 프로젝트 
- 추후 linux 에서도 사용가능 하도록 개발될 경우 소스코드 상위로 이동

# example

## registry

### file

- registry.cpp
- registry.h

### example

- 윈도우 레지스트리 키 확인 및 키 생성
- DWORD, string 값 생성 및 가져오기 확인

```cpp
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
```

## service

### file

- service.cpp
- service_control.cpp
- service.h
- service_control.h

### example

- 다음과 같이 class 를 상속하여 사용한다.
- 기본 프로세스 실행은 다음과 같이 작성한다.

```cpp
class MainApp : public gstd::Service{
public:
	MainApp(LPCTSTR service_name, LPCTSTR service_description);
  ~MainApp(){}
  virtual VOID Run();
	virtual BOOL Start();
	virtual VOID End();
  virtual VOID Stop();
};

MainApp::MainApp(LPCTSTR service_name, LPCTSTR service_description)
  : gstd::Service(service_name, service_description)
{

}
VOID MainApp::Stop()
{
  //프로그램 종료 호출시 정의 될 내용
  // 하위스레드 stop 호출 등.
}

VOID MainApp::Run()
{
  // 프로그램의 cycle 마다 실행될 내용.
  // 선행으로 set_run_cycle_milliseconds(DWORD value) 함수로 cycle 설정
  // 내부적으로 프로세스내에서 설정된 milliseconds cycle 로 Run 함수를 호출한다.
}

BOOL MainApp::Start()
{
  //프로그램 실행시 정의될 내용
  // return 이 TRUE 일 경우 정상동작하며 FALSE 인 경우 프로그램은 종료
  return TRUE;
}

VOID MainApp::End()
{
  // 프로그램 종료시 처리 될 내용
  // 메모리 반환, 남은 데이터 처리 등
}

int _tmain(int argc, _TCHAR* argv[])
{
  MainApp app("프로그램명", "프로그램 설명");

  // service mode 로 프로그램을 실행 할 경우.
  // 서비스 리스트에 해당 바이너리가 등록되어 있어야 한다.
  // app.ServiceMain();

  // console mode 로 프로그램을 실행 할 경우.
  // app.ConsoleMain();

  return 0;
}

// watcher 용 프로세스 생성 (서비스)
int _tmain(int argc, _TCHAR* argv[])
{
  MainApp app("프로그램명", "프로그램 설명");
  // watcher 로 관리할 프로세스 설정
  app.set_watcher_process("Test.exe");
  // 해당 프로세스를 서비스로 시작.
  // 사전에 서비스가 해당 실행프로그램으로 등록되어 있어야 한다.
  app.ServiceMain()
}

```

- 서비스 등록 및 삭제 예제

```cpp
// service_control 헤더는 vs 설정에 따라 인클루드 경로가 달라질 수 있음
// 서비스 등록 바이너리를 정상적으로 실행 하기 위해서는
// 해당 바이너리 실행을 관리자 권한으로 실행하거나, 커맨드라인툴을 관리자 권한으로 실행한 후에 명령어로 실행해야 한다.
#include <service_control.h>
#include <plog/Log.h>
int _tmain(int argc, _TCHAR* argv[])
{
  // 서비스 컨트롤러를 생성한다.
  // 변수로 등록될 서비스 명, 서비스의 설명을 저장한다.
  gstd::ServiceControl scon("servicename", "service description");

  std::string buffer;
  if (argc >= 2) {
    buffer.assign(argv[1]);
    std::cout << buffer << std::endl;
  }

  if (!buffer.compare("--install")) {
    // 현재 실행되는 binary(절대경로) 를 서비스에 등록한다.
    // 서비스관련 정보는 변수에 등록된 값으로 진행한다.
    scon.Install();
  } else if (!buffer.compare("--uninstall")) {
    // 변수에 저장된 서비스명의 서비스를 서비스리스트에서 삭제한다.
    scon.Uninstall();
  }
  return 0;
}
```