# reference project

## Description

- 해당 프로젝트는 모니터링 개발에서 공통으로 사용할만한 모듈들을 추가한 라이브러리 프로젝트 이다.
- 외부라이브러리를 참조하여 개발한 모듈 (redis-client, http-client, geoip client) 들은 해당 프로젝트에 추가.
- 프로젝트의 빌드방식은 CMAKE 를 사용한다.
- 모듈별 테스트에는 gtest 를 사용한다.

## directory

| name     | description                                                  |
| -------- | ------------------------------------------------------------ |
| bin      | reference 모듈을 사용하여 기능을 수행하는 command tool       |
|          | ex) iplookup(maxminddb)                                      |
| doc      | 프로젝트 관련 문서파일                                       |
| external | 참조할만한 외부 프로젝트의 서브모듈 등록 directory           |
| src      | source code directory                                        |
| test     | unit test 또는 전체 기능 테스트를 위한 테스트 코드 directory |

## History

### v1.5.0

**Implemented enhancements :**

- redis client 기능 추가

  - MultipleSet (MSET, MSETNX)
  - MultipleGet (MGET)
  - GetSet
  - uint64_t get, set
  - CheckAndSet, Watch, Unwatch, Multi, Exec

- uuid generator 추가

### v1.4.0

**Implemented enhancements :**

- conan package manager 적용

### v1.3.1

**Implemented enhancements :**

- dictionary(key value) 를 이용한 string replace 기능 추가

### v1.3.0

**Implemented enhancements :**

- datetime string 사용을 위한 DateTime class

  - iso8601, cpp standard, syslog format 등에 대한 getter 추가
  - cpp Conversion specifier 사용

### v1.2.2

**Implemented enhancements :**

- blocking advancedqueue close 기능 추가

  - 외부에서 blocking advancedqueue close 호출시 block 되어 대기중인 모든 스레드 대기 해제.
  - Pop, Push 에 대한 데이터 접근에 대한 disable 처리.

**Fixed bug :**

- blocking advancedqueue 스레드별 동기화 오류로 비정상 데이터 접근 버그 수정
  
  - queue data 타입이 shared_ptr 일 경우 shared_ptr 의 destructor 에서 segmentation fault 발생

### v1.2.0

**Implemented enhancements :**

- std::string printf 기능 추가
- redis hash store 사용을 위한 redis_client 기능 추가
  - hmget, hmset, hincrby, hdel

### v1.1.0

**Implemented enhancements :**

- blocking advancedqueue 추가
- advancedqueue 기본형을 base_advancedqueue 로 분리후 상속하여 사용

### v1.0.4

**Implemented enhancements :**

- client 공통 에러 코드 추가
- http-client tcp keepalive 설정 추가
- http-client curl 에러 발생시 오류 설정 추가
- numeric 체크 추가

### v1.0.3

**Implemented enhancements :**

- windows directory 체크 및 삭제 추가

**Fixed bug :**

- filetool 윈도우즈 오류 수정

### v1.0.2

**Implemented enhancements :**

- cycle checker 추가
  - unit test code 추가

### v1.0.1

**Implemented enhancements :**

- ip 관련 모듈 통합 gstd::net::Ip
  - ip address <-> ip number 변환
  - check special purpose ip
  - ip netmask calculator

### v1.0.0

**Implemented enhancements :**

- directory 구조 변경 및 네임스페이스 추가.
  - 모든 코드를 src/gstd 하위로 변경.
- 프로젝트에서 기본적으로 필요한 외부 submodule 은 cmake external project 로 변경하여 빌드
  - maxminddb, hiredis, gtest

**Fixed bug :**

### v0.9.1

**Implemented enhancements :**

- service_control 의 service stop & start 기능 추가
- version string check class 추가
- strutil > windows error string function 추가

**Fixed bug :**

### v0.9.0

- reference 의 라이브러리(공통모듈)화 및 사용되는 기타 프로젝트에서의 버전관리를 위한 버전관리 추가

## build

참조하는 maxminddb 의 기본 빌드를 위해서는 다음의 요구사항이 충족되어야 한다.

> CentOS 6 이상에서 빌드시 문제없으나, CentOS 5 에서 빌드시 다음의 GNU tool 업그레이드가 필요하다.  
> 관련 설치파일은 ftp://ftp.gnu.org/gnu  
> automake 1.14 이후 버전의 경우 CentOS5 빌드가 안되므로 최소 필요버전인 1.14 로 설치  
> TODO: 이후 정확한 설치해야할 최소버전 명시 예정

- autoconf > 2.60
- automake > 1.14
- libtool > CentOS 5 설치가능한 최신버전

상세 빌드 방법

```bash
$ git clone https://gitlab.gabia.com/monitoring/common/cpp/reference.git
$ cd  reference
$ git submodule update --init --recursive
$
$ mkdir -p build
$ cd build
$ cmake ../
$ make run-test
```

출력 폴더

- bin : iplookup 등 바이너리 파일
- lib : libreference.a static library 파일
- build/test : 테스트 바이너리 파일

## module description

### Logger

- TODO: 현재 syslog 사용만 가능하며 추가적으로 기본 로그도 사용가능하도록 추가 할 예정.

  ```cpp
    // 자세한 enum, macro 등은 logger.h 참조

    // syslog 를 오픈한다.
    // process name, minimum log severity, syslog facility 를 설정한다.
    gstd::Logger::InitSyslogInstance("test", gstd::genum::logger::kDebug, gstd::genum::logger::kLocal5);

    // syslog 기본 매크로
    // severity, 와 message 를 syslog 에 작성한다.
    _GSTD_SYSLOG(gstd::genum::logger::kWarn, "write %s", "syslog");

    // severity 별 매크로
    _GSTD_WARN_SYSLOG("Test Warning %s", "OK");
    _GSTD_CRIT_SYSLOG("Test Critical %s", "OK");

    gstd::Logger::ExitSyslogInstance();

  ```

### CSmtp

- SMTP 프로토콜을 사용한 메일 클라이언트.
- 한글 미지원인 코드에 추가로 수정하여 utf-8, base64 인코딩으로 한글사용하도록 수정.
- 소켓 타임아웃 미지정 추가

### initscript

- redhat init.d script 생성

#### example

```cpp
  CInitScript inis("gabia_mond" /*실행파일*/, "/usr/local/gabia_mond/bin"/* 설치경로 */);
  inis.SetDebug();
  inis.SetDescription("Gabia Server monitoring service provided by daemon.");
  //! daemon이 설치된 경로에 데몬.init 스크립트를 생성한다.
  inis.Create();
  //! daemon.init 스크립트를 /etc/init.d/ 와 /etc/rc.d/init.d/ 에 복사한다.
  //! chkconfig 를 이용하여 등록한다.
  if(!strcmp(argv[1], "install"))
    inis.Install();
  else if(!strcmp(argv[1], "uninstall"))
    inis.Uninstall();
```

### ServiceControl

- 서비스 정지

  ```cpp
  int retries = 0;
  gstd::ServiceControl service_control("서비스명", "서비스 설명");
  while (retries++ < 5) {
    if (service_control.IsStoppedService() == _GSTD_SERVICE_CHECK_RUNNING) {
      // 서비스 실행중
      service_control.DoStopService();
    }
  }

  ```

- 서비스 시작

  ```cpp
  int retries = 0;
  gstd::ServiceControl service_control("서비스명", "서비스 설명");
  while (retries++ < 5) {
    if (service_control.DoStartService()) {
      // 시작 성공
    } else {
      if  (service_control.last_error_number() == WAIT_TIMEOUT) {
        // SERVICE_START_PENDING timeout
      } else if (service_control.last_error_number() == _GSTD_SERVICE_CHECK_RUNNING) {
        // 이미 서비스가 시작되어 있는 경우 실패후 error number 가 0
      else {
        // 실패처리.
      }
    }
  }
  ```
  