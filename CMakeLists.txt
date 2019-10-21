# reference 라이브러리 파일 cmake file
# CentOS 5 에서 기본지원되는 cmake version 2.6 의 경우 external project 를 지원하지 않는다.
# 최소 external project 를 사용하기 위해서는 version 2.8 이상의 사용이 필요하다.
# CentOS 5 에서는 source build 를 통해 3.X 초반대 설치가 가능하다.
CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

# 프로젝트명 설정
PROJECT ( reference )

INCLUDE (ExternalProject)

# Openssl library build 를 포함 시킬경우
OPTION (WITH_OPENSSL_BUILD "openssl build" ON)
# libcurl build 를 포함 시킬 경우
OPTION (WITH_LIBCURL_BUILD "libcurl build" ON)
# http client build 를 포함 시킬 경우
# 해당 옵션이 ON 인 경우 libcurl, openssl 는 자동으로 external 추가
OPTION (WITH_HTTP_CLIENT_BUILD "gstd::client::http build" ON)
# TEST build 적용
OPTION (WITH_REFERENCE_TEST "reference make test" ON)
# ExternalProject 에 등록된 라이브러리를 빌드하지 않는다.
# 개발 및 테스트 진행을 위해 최초빌드는 필요하므로 default ON
# 이후 개발진행시 추가적으로 external project 체크가 필요없으므로 OFF 하여 개발
OPTION ( PASS_EXTERNAL_BUILD "Do not build external project" OFF )

IF (WITH_REFERENCE_TEST)
MESSAGE ( "enable test" )
# 테스트 설정
ENABLE_TESTING()
ENDIF (WITH_REFERENCE_TEST)

# PREFIX 기준으로 별도로 설정하지 않을 경우의 경로
# DOWNLOAD = <PREFIX>/src
# SOURCE_DIR = <PREFIX>/src/<name>
# BINARY_DIR = <PREFIX>/src/<name>-build
# INSTALL_DIR = <PREFIX>

# openssl, simple-zip 등에서 사용할 zlib
IF ( PASS_EXTERNAL_BUILD )
ExternalProject_Add(
  project_zlib
  GIT_REPOSITORY https://github.com/madler/zlib.git
  GIT_TAG v1.2.11
  PREFIX ${PROJECT_SOURCE_DIR}/libtemp/zlib
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
ELSE ( PASS_EXTERNAL_BUILD )
ExternalProject_Add(
  project_zlib
  GIT_REPOSITORY https://github.com/madler/zlib.git
  GIT_TAG v1.2.11
  PREFIX ${PROJECT_SOURCE_DIR}/libtemp/zlib
  INSTALL_COMMAND ""
)
ENDIF ( PASS_EXTERNAL_BUILD )

ExternalProject_Get_Property(project_zlib source_dir binary_dir)
ADD_LIBRARY(zlib STATIC IMPORTED GLOBAL)
SET_TARGET_PROPERTIES(zlib
                      PROPERTIES
                      "IMPORTED_LOCATION" "${binary_dir}/libz.a"
) 
ADD_DEPENDENCIES(zlib project_zlib)
IF ((WITH_OPENSSL_BUILD) OR (WITH_HTTP_CLIENT_BUILD))
  IF ( PASS_EXTERNAL_BUILD )
    ExternalProject_Add(
      project_openssl
      PREFIX ${PROJECT_SOURCE_DIR}/libtemp/openssl
      BINARY_DIR ${PROJECT_SOURCE_DIR}/libtemp/openssl/src/openssl
      SOURCE_DIR ${PROJECT_SOURCE_DIR}/libtemp/openssl/src/openssl
      INSTALL_DIR ${PROJECT_SOURCE_DIR}/libtemp/openssl/build
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ""
    )
  ELSE ( PASS_EXTERNAL_BUILD )
    MESSAGE ( "Openssl build" )
    ExternalProject_Add(
      project_openssl
      GIT_REPOSITORY https://github.com/openssl/openssl.git
      GIT_TAG OpenSSL_1_0_2p
      DEPENDS project_zlib
      PREFIX ${PROJECT_SOURCE_DIR}/libtemp/openssl
      BINARY_DIR ${PROJECT_SOURCE_DIR}/libtemp/openssl/src/openssl
      SOURCE_DIR ${PROJECT_SOURCE_DIR}/libtemp/openssl/src/openssl
      INSTALL_DIR ${PROJECT_SOURCE_DIR}/libtemp/openssl/build
      CONFIGURE_COMMAND ./config --prefix=<INSTALL_DIR> shared zlib
      BUILD_COMMAND make
    )
  ENDIF ( PASS_EXTERNAL_BUILD )
  ExternalProject_Get_Property(project_openssl source_dir binary_dir install_dir)
  ADD_LIBRARY(libssl STATIC IMPORTED GLOBAL)
  SET_TARGET_PROPERTIES( libssl
                        PROPERTIES
                        IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/libtemp/openssl/build/lib/libssl.a"
  ) 
  ADD_DEPENDENCIES(libssl project_openssl) 
  ADD_LIBRARY(libcrypto STATIC IMPORTED)
  SET_TARGET_PROPERTIES(libcrypto
                      PROPERTIES
                      IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/libtemp/openssl/build/lib/libcrypto.a"
  ) 
  INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/libtemp/openssl/build/include")
  ADD_DEPENDENCIES(libcrypto project_openssl) 
ENDIF ((WITH_OPENSSL_BUILD) OR (WITH_HTTP_CLIENT_BUILD))

# curl 의 cmake 의 경우 버전 3.4 이상을 요구하므로 그이하에서는 에러가 발생.
IF ((WITH_LIBCURL_BUILD) OR (WITH_HTTP_CLIENT_BUILD))
  MESSAGE ( "Curl library build" )

  IF (VERSION VERSION_LESS 3.4)
    IF ( PASS_EXTERNAL_BUILD )
      ExternalProject_Add(
        project_curl
        PREFIX ${PROJECT_SOURCE_DIR}/libtemp/curl
        BINARY_DIR ${PROJECT_SOURCE_DIR}/libtemp/curl/src/curl
        SOURCE_DIR ${PROJECT_SOURCE_DIR}/libtemp/curl/src/curl
        INSTALL_DIR ${PROJECT_SOURCE_DIR}/libtemp/curl/build
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
      )
    ELSE ( PASS_EXTERNAL_BUILD )
      MESSAGE ( "cmake < 3.4 build autoconf" )
      ExternalProject_Add(
        project_curl
        URL https://curl.haxx.se/download/curl-7.61.1.zip
        PREFIX ${PROJECT_SOURCE_DIR}/libtemp/curl
        BINARY_DIR ${PROJECT_SOURCE_DIR}/libtemp/curl/src/curl
        SOURCE_DIR ${PROJECT_SOURCE_DIR}/libtemp/curl/src/curl
        INSTALL_DIR ${PROJECT_SOURCE_DIR}/libtemp/curl/build
        DEPENDS project_openssl
        CONFIGURE_COMMAND ./configure --prefix=<INSTALL_DIR> --with-ssl=${PROJECT_SOURCE_DIR}/libtemp/openssl/build --disable-ldap
        BUILD_COMMAND make
    )
    ENDIF ( PASS_EXTERNAL_BUILD )
  ELSE (VERSION VERSION_LESS 3.4)
    IF ( PASS_EXTERNAL_BUILD )
      ExternalProject_Add(
        project_curl
        PREFIX ${PROJECT_SOURCE_DIR}/libtemp/curl
        BINARY_DIR ${PROJECT_SOURCE_DIR}/libtemp/curl/src/curl
        SOURCE_DIR ${PROJECT_SOURCE_DIR}/libtemp/curl/src/curl
        INSTALL_DIR ${PROJECT_SOURCE_DIR}/libtemp/curl/build
        DEPENDS project_openssl
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
      )
    ELSE ( PASS_EXTERNAL_BUILD )
      MESSAGE ( "cmake > 3.4 build cmake" )
      ExternalProject_Add(
        project_curl
        GIT_REPOSITORY https://github.com/curl/curl.git
        GIT_TAG curl-7_61_1
        DEPENDS project_openssl
        PREFIX ${PROJECT_SOURCE_DIR}/libtemp/curl
        CMAKE_ARGS -DCMAKE_USE_OPENSSL=ON -DOPENSSL_INCLUDE_DIR=${PROJECT_SOURCE_DIR}/libtemp/openssl/build/include
      )
    ENDIF ( PASS_EXTERNAL_BUILD )
  ENDIF (VERSION VERSION_LESS 3.4)
  ExternalProject_Get_Property(project_curl source_dir binary_dir install_dir)
  ADD_LIBRARY(libcurl STATIC IMPORTED GLOBAL)
  SET_TARGET_PROPERTIES(libcurl
                        PROPERTIES
                        IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/libtemp/curl/build/lib/libcurl.a")
  ADD_DEPENDENCIES(libcurl project_curl) 
  INCLUDE_DIRECTORIES ("${PROJECT_SOURCE_DIR}/libtemp/curl/build/include")
  SET ( LIBTEMP_CURL ${PROJECT_SOURCE_DIR}/libtemp/curl/build)
ENDIF ((WITH_LIBCURL_BUILD) OR (WITH_HTTP_CLIENT_BUILD))

# hiredis 의 경우 makefile 하나로 빌드하며 해당 경로에서 결과가 출력
# 별도의 install 을 하지않고 src 디렉토리를 include  와 library 참조에 사용한다.
IF ( PASS_EXTERNAL_BUILD )
  ExternalProject_Add(
    hiredis
    PREFIX ${PROJECT_SOURCE_DIR}/libtemp/hiredis
    BINARY_DIR ${PROJECT_SOURCE_DIR}/libtemp/hiredis/src/hiredis
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/libtemp/hiredis/src/hiredis
    CONFIGURE_COMMAND ""  # configure 실행 하지 않음.
    BUILD_COMMAND ""
    INSTALL_COMMAND ""    # make install 실행 하지 않음.
  )
ELSE ( PASS_EXTERNAL_BUILD )
  ExternalProject_Add(
    hiredis
    GIT_REPOSITORY https://github.com/redis/hiredis.git
    GIT_TAG v0.14.0
    PREFIX ${PROJECT_SOURCE_DIR}/libtemp/hiredis
    BINARY_DIR ${PROJECT_SOURCE_DIR}/libtemp/hiredis/src/hiredis
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/libtemp/hiredis/src/hiredis
    CONFIGURE_COMMAND ""  # configure 실행 하지 않음.
    BUILD_COMMAND make
    INSTALL_COMMAND ""    # make install 실행 하지 않음.
  )
ENDIF ( PASS_EXTERNAL_BUILD )

IF ( PASS_EXTERNAL_BUILD )
  ExternalProject_Add(
    maxminddb
    PREFIX ${PROJECT_SOURCE_DIR}/libtemp/maxminddb
    BINARY_DIR ${PROJECT_SOURCE_DIR}/libtemp/maxminddb/src/maxminddb
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/libtemp/maxminddb/src/maxminddb
    INSTALL_DIR ${PROJECT_SOURCE_DIR}/libtemp/maxminddb/build
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
  )
ELSE ( PASS_EXTERNAL_BUILD )
  ExternalProject_Add(
    maxminddb
    GIT_REPOSITORY https://github.com/maxmind/libmaxminddb.git
    GIT_TAG 1.3.2
    PREFIX ${PROJECT_SOURCE_DIR}/libtemp/maxminddb
    BINARY_DIR ${PROJECT_SOURCE_DIR}/libtemp/maxminddb/src/maxminddb
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/libtemp/maxminddb/src/maxminddb
    INSTALL_DIR ${PROJECT_SOURCE_DIR}/libtemp/maxminddb/build
    CONFIGURE_COMMAND ./configure --prefix=<INSTALL_DIR>
    BUILD_COMMAND make
  )

  ExternalProject_Add_Step(maxminddb
                          bootstrap
                          COMMAND ./bootstrap
                          DEPENDEES download
                          DEPENDERS configure
                          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/libtemp/maxminddb/src/maxminddb)
ENDIF ( PASS_EXTERNAL_BUILD )

ExternalProject_Get_Property(maxminddb source_dir binary_dir install_dir)
ADD_LIBRARY(libmaxminddb STATIC IMPORTED GLOBAL)
ADD_DEPENDENCIES(libmaxminddb maxminddb)

SET_TARGET_PROPERTIES(libmaxminddb
                      PROPERTIES
                      "IMPORTED_LOCATION" "${install_dir}/lib/libmaxminddb.a"
                     )

INCLUDE_DIRECTORIES("${install_dir}/include")
# hiredis
ExternalProject_Get_Property(hiredis source_dir binary_dir install_dir)
ADD_LIBRARY(libhiredis STATIC IMPORTED GLOBAL)
ADD_DEPENDENCIES(libhiredis hiredis)

SET_TARGET_PROPERTIES(libhiredis
                      PROPERTIES
                      "IMPORTED_LOCATION" "${source_dir}/libhiredis.a"
                     )
INCLUDE_DIRECTORIES("${source_dir}")


# 완전한 C++11 standard 제공은 gcc 4.8 이상 부터
# 참조 https://gcc.gnu.org/gcc-4.8/cxx0x_status.html
SET ( CHECK_GCC_STD11_MINIMUM 4.8.1 )

MESSAGE (${CMAKE_CXX_COMPILER_VERSION})
MESSAGE (${CHECK_GCC_STD11_MINIMUM})

IF (${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS ${CHECK_GCC_STD11_MINIMUM} )
SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x" )
ELSE()
# gcc version 체크후 c++11 사용
SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
#add_compile_options(-std=c++11)
ENDIF()
#SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

ADD_DEFINITIONS(-DLINUX)

INCLUDE_DIRECTORIES (src
                    )

SET ( SRC_DIRS
      src/gstd/check
      src/gstd/net
      src/gstd/client
      src/gstd/container
      src/gstd/type
      src/gstd/util
      src/gstd/client/redis
      src/gstd/client/http
    )

FOREACH ( dir ${SRC_DIRS})

  FILE (GLOB TEMP_FILES "${dir}/*.cpp" )

  FOREACH ( item ${TEMP_FILES})

  # asyncsock.cpp 의 경우 xms 소스코드 참조형태로 전체적인 수정전까지 빌드제외
  #MESSAGE(STATUS "${item} == ${PROJECT_SOURCE_DIR}/asyncsock.cpp\n") # 디버깅용 출력
  IF ( ${item} STREQUAL ${PROJECT_SOURCE_DIR}/asyncsock.cpp )
    LIST(REMOVE_ITEM TEMP_FILES "${PROJECT_SOURCE_DIR}/asyncsock.cpp")

  ENDIF()

  ENDFOREACH()
  #MESSAGE (STATUS ${TEMP_FILES})
  SET ( SOURCES ${SOURCES} ${TEMP_FILES} )

ENDFOREACH()

# static library 파일 빌드
ADD_LIBRARY ( reference STATIC ${SOURCES} )

ADD_DEPENDENCIES ( reference project_openssl )
ADD_DEPENDENCIES ( reference project_curl )
ADD_DEPENDENCIES ( reference project_zlib )
ADD_DEPENDENCIES ( reference hiredis )
ADD_DEPENDENCIES ( reference maxminddb )

# library 출력경로 설정
# shared 는 LIBRARY_OUTPUT_DIRECTORY, static library 는 ARCHIVE_OUTPUT_DIRECTORY 설정
SET_TARGET_PROPERTIES ( reference
                        PROPERTIES
                        #LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib )
                        ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib )

# make cppcheck 추가
FIND_PROGRAM ( CMAKE_CXX_CPPCHECK NAMES cppcheck )
IF (CMAKE_CXX_CPPCHECK)
  MESSAGE ( STATUS "${ColorGreen}Found Tool: ${CMAKE_CXX_CPPCHECK}${ColorEnd}")

  # cmake 내에 cppcheck 실행을 위한 custom target 을 추가한다.
  ADD_CUSTOM_TARGET ( cppcheck
                      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src
                      COMMAND cppcheck --enable=warning --std=posix --platform=unix64 --std=c++11 --force ${ALL_DEFS} -I ./ -I ../base/inc ./
                    )
ELSE ()
  MESSAGE ( FATAL_ERROR "${ColorRed}Not found cppcheck${ColorEnd}")
ENDIF ()

#
# src/bin 빌드
ADD_SUBDIRECTORY ( src )

# test 디렉토리 빌드
IF (WITH_REFERENCE_TEST)
ADD_SUBDIRECTORY(test)
ENDIF (WITH_REFERENCE_TEST)
