# reference test 빌드를 위한 CMakeLists 파일
# 

SET ( TESTER_NAME reference-test )

INCLUDE (ExternalProject)
# test 를 위한 gtest 프로젝트 추가
# v1.6.0 -> v1.8.0 으로 변경
# googletest 와 googlemock 프로젝트가 1.8.0 이후부터 병합
ExternalProject_Add(
  gtest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.8.0
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
    INSTALL_COMMAND ""
)

# gtest 의 환경정보
# source_dir = 소스다운로드 dir
# binary_dir = library 빌드된 dir
ExternalProject_Get_Property(gtest source_dir binary_dir)

# Create a libgtest target to be used as a dependency by test programs
ADD_LIBRARY(libgtest STATIC IMPORTED GLOBAL)
ADD_DEPENDENCIES(libgtest gtest)

# Set libgtest properties
SET_TARGET_PROPERTIES(libgtest
                      PROPERTIES
                      IMPORTED_LOCATION "${binary_dir}/googlemock/gtest/libgtest.a"
                      IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}"
)

# Create a libgmock target to be used as a dependency by test programs
ADD_LIBRARY(libgmock IMPORTED STATIC GLOBAL)
ADD_DEPENDENCIES(libgmock gtest)

# Set libgmock properties
SET_TARGET_PROPERTIES(libgmock
                      PROPERTIES
                      IMPORTED_LOCATION "${binary_dir}/googlemock/libgmock.a"
                      IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}"
)

# gtest include 추가
INCLUDE_DIRECTORIES( ${source_dir}/googletest/include )
INCLUDE_DIRECTORIES( ${source_dir}/googlemock/include )

INCLUDE_DIRECTORIES( /usr/include )

SET ( SRCS 
      dictionary/dictionary-test.cpp 
      advancedqueue/test_advancedqueue.cpp 
      check/test_check.cpp
      http_client/test_http_client.cpp
      ipchecker/test_ipchecker.cpp
      logger/logger-test.cpp
      numeric/test_numeric.cpp
      redis_client/test_redis_client.cpp
      strtool/strtool-test.cpp
      version/test_version.cpp
      net/test_ip.cpp
      net/test_hostaddr.cpp
      util/test_datetime.cpp
      main.cpp )

ADD_EXECUTABLE ( ${TESTER_NAME} ${SRCS} )

TARGET_LINK_LIBRARIES ( ${TESTER_NAME}
                        reference
                        libcurl
                        libssl
                        libcrypto
                        zlib
                        libgtest
                        libgmock
                        libhiredis
                        libmaxminddb
                        pthread
                        dl
)

ADD_TEST (NAME ${TESTER_NAME}
          COMMAND ${TESTER_NAME})