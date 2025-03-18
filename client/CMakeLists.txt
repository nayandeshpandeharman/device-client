cmake_minimum_required(VERSION 3.4)

#set(IC_UNIT_TEST 1)
#set(GTEST_INC googletest/include)
#set(GTEST_LIB /usr/src/gtest)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_LIBRARY_PATH /opt/local/lib ${CMAKE_LIBRARY_PATH})

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)

#For Linux builds, using UNIX socket as INET_SOCKET doesn't work well
#add_definitions(-DINET_SOCKET=1)
add_definitions(-DSHORT_LOG_MSG)
add_definitions(-DEVENT_LOG_INTERVAL_CNT=500)
add_definitions(-DENABLE_ZMQ)
add_definitions(-DINCLUDE_IGNITE_AUTO)

if(IC_UNIT_TEST EQUAL 1)
	add_definitions(-DIC_UNIT_TEST=1)
endif ()

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(SEND_ERROR "In-source builds are not allowed.")
endif ()

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE   ON)

project(deviceclient)

if(IC_UNIT_TEST EQUAL 1)
	add_subdirectory(googletest)
	
endif ()

add_subdirectory(libUtils)
add_subdirectory(libEvent)
add_subdirectory(libCore)
add_subdirectory(libmosquitto)
add_subdirectory(libClientBL)
add_subdirectory(libNetwork)
add_subdirectory(libDevice)
add_subdirectory(libAuto)
add_subdirectory(deviceclient)

