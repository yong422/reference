# reference 라이브러리 파일 cmake file
# CentOS 5 에서 기본지원되는 cmake version 2.6 의 경우 external project 를 지원하지 않는다.
# 최소 external project 를 사용하기 위해서는 version 2.8 이상의 사용이 필요하다.
# CentOS 5 에서는 source build 를 통해 3.X 초반대 설치가 가능하다.
CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

# 프로젝트명 설정
PROJECT ( reference )

FIND_PACKAGE ( PythonInterp )
# conan cmake 설치, conan 을 이용한 라이브러리 설치
IF ( NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake" )
  MESSAGE ( STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan" )
  #FILE ( DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.13/conan.cmake"
  FILE ( DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
                  "${CMAKE_BINARY_DIR}/conan.cmake")
ENDIF ()

INCLUDE ( ${CMAKE_BINARY_DIR}/conan.cmake )

# 개발서버 기준.
# python 2.7.13, conan 1.17.2 환경에서 개발.
#conan_check ( VERSION 1.17.2 REQUIRED )

# 추가로 사용하는 conan remote repo 에 대하여 추가
conan_add_remote(NAME mpusz
                URL https://api.bintray.com/conan/mpusz/conan-mpusz)

# maxminddb conan repo
conan_add_remote(NAME monkeber-maxminddb
                 URL https://api.bintray.com/conan/monkeber/monkeber )

conan_add_remote(NAME ykjo 
                 URL https://api.bintray.com/conan/ykjo/conan-cpp )

#  conan_cmake_run ( CONANFILE conanfile.py BASIC_SETUP NO_OUTPUT_DIRS BUILD missing BUILD_TYPE Release VERBOSE)
# BASIC_SETUP 옵션을 줄경우 conan_cmake_run 내부에서 conan_basic_setup 이 실행된다.
#  conan_basic_setup 의 경우 output dirs 등의 전역 변수에 대한 설정이 실행되므로 
# BASIC_SETUP 의 경우 별도의 conan_basic_setup 으로 처리한다.
conan_cmake_run ( CONANFILE conanfile.py BUILD missing BUILD_TYPE Release VERBOSE)

# conan_cmake_run 내부 함수에서 conan_load_buildinfo() 함수를 호출하여 conan build 로 생성되는 
# 프로젝트 매크로 파일을 자동으로 load 한다.
# 따라서 별도의 include 가 필요 없다.
# INCLUDE ( ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake )

# conan 을 이용하여 빌드된 라이브러리의 타겟 매크로 설정
# ARGUMENTS_NO_OUTPUT_DIRS 
#   설정을 하지 않을 경우 conan_cmake_run 내부에서 output 경로를 빌드되는 디렉토리 기준으로 전역 설정한다.
#   따라서 타겟별로 설정된 output 이 적용되지 않으므로, 타겟별로 output 을 별도 설정 한 경우 해당 옵션을 추가해야 한다.
conan_basic_setup ( TARGETS NO_OUTPUT_DIRS )

# conan 을 이용하여 설치한 라이브러리의 헤더경로 추가
INCLUDE_DIRECTORIES ( ${CONAN_INCLUDE_DIRS} )

ENABLE_TESTING()


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

# library 출력경로 설정
# shared 는 LIBRARY_OUTPUT_DIRECTORY, static library 는 ARCHIVE_OUTPUT_DIRECTORY 설정
SET_TARGET_PROPERTIES ( reference
                        PROPERTIES
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
ADD_SUBDIRECTORY ( test )

