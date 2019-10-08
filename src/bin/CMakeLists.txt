# bin/iplookup cmake

SET ( BIN_NAME
      iplookup )

#FILE (GLOB SRCS *.cpp) 
SET ( SRCS iplookup.cpp )

ADD_EXECUTABLE(${BIN_NAME} ${SRCS})

SET_TARGET_PROPERTIES ( ${BIN_NAME}
                        PROPERTIES
                        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin )

TARGET_LINK_LIBRARIES (${BIN_NAME}
                       reference
                       pthread
                       libmaxminddb
)
