sftp-client
===========

# Description

- libcurl(openssl, libssh2) 를 사용하는 sftp client
- 사용되는 모든 library 를 static link 하여 사용



# build 방법

- include directory 와 library directory 에 각각 dependency 한 library 를 추가
- 헤더에 다음과 같은 라이브러리 추가가 필요

```cpp
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "libcurl.lib")
#pragma comment(lib, "wldap32.lib")
#pragma comment(lib, "libssh2.lib")
#pragma comment(lib, "Crypt32.lib")   // libeay32.lib dependency
#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")
```

- 전처리기에 CURL_STATICLIB 정의 추가


# dependency library (2018.03.05 빌드기준, VC10)

## OpenSSL (1.0.2l)

- openssl-1.0.2l static library 사용
- 상세빌드는 openssl 빌드방법 [참조](링크추가예정)

## libssh2 (1.8.0)

- libssh2-1.8.0 static library 사용
- openssl-1.0.2l static library 참조하여 빌드

### build

- static library 빌드된 openssl 필요
- [다운로드](https://www.libssh2.org/)
- VC++ 6 Project 오픈후 사용중인 버젼의 프로젝트로 변경 (VC10), 솔루션 파일 생성
- 솔루션 구성에서 OpenSSL LIB Release 선택하여 build
  - build 시 openssl header 경로 추가가 필요

## libcurl (7.58.0)

- libcurl-7.58.0 static library 사용
- [다운로드](https://curl.haxx.se/download.html) 에서 source Archives 내 zip 파일 다운로드
- projects\Windows 내에서 사용중인 visual studio 에 맞는 프로젝트 선택.
  - Visual studio 2010 기준으로 VC10
  - projects\Windows\VC10\curl-all 솔루션파일 선택
- 솔루션 구성에서 LIB Release - LIB OpenSSL - LIB libssh2 선택
- openssl, libssh2 header include 
- build 진행
- build 된 libcurl static library는 build\Win32\VC10\LIB Release - LIB OpenSSL - LIB LibSSH2 저장.
- header 파일과 별도로 구성하여 복사하여 사용