# reference test 빌드를 위한 CMakeLists 파일
# 

SET ( TESTER_NAME unit-test )

INCLUDE (ExternalProject)


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
      util/test_string_generator.cpp
      main.cpp )

ADD_EXECUTABLE ( ${TESTER_NAME} ${SRCS} )

TARGET_LINK_LIBRARIES ( ${TESTER_NAME}
                        reference
                        CONAN_PKG::libcurl
                        CONAN_PKG::OpenSSL
                        CONAN_PKG::zlib
                        CONAN_PKG::gtest
                        CONAN_PKG::maxminddb
                        CONAN_PKG::hiredis
                        pthread
                        dl
)

ADD_TEST (NAME ${TESTER_NAME}
          COMMAND ${TESTER_NAME})



ADD_CUSTOM_TARGET ( run-test 
                    COMMAND ${TESTER_NAME}
)
ADD_CUSTOM_COMMAND ( TARGET run-test POST_BUILD
                    COMMAND ;
                    COMMENT "run all unit tests" )
