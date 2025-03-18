cmake_minimum_required(VERSION 3.4)

project(ClientBL)

add_definitions(-std=c++11 -DCPP11_SUPPORT -DMQTT_SUPPORT)

if (FPIC)
add_definitions(-fPIC)
endif()

if (RELEASE_BINARY)
add_definitions(-g0)
endif()

include_directories(
    include
    include/snf
    src
    ${Utils_INCLUDE_DIRS}
    ${Event_INCLUDE_DIRS}
    ${Core_INCLUDE_DIRS}
    ${libmosquitto_INCLUDE_DIRS}
    ../libNetwork/include/http
    ../libNetwork/include/mqtt
    ../libNetwork/include/
    ../libAuto/src
	../libAuto/include
    /opt/local/include
    /usr/local/include
)

file(
    GLOB_RECURSE CPP_FILES src/*.cpp
)

if(DEFINED IC_UNIT_TEST)
	if(${IC_UNIT_TEST} EQUAL 1)
		file(
			GLOB_RECURSE TEST_CPP_FILES unittest/*.cpp
		)
	endif()
endif()

add_library(ClientBL
    ${CPP_FILES}
    ${TEST_CPP_FILES}
)

find_library(libz
    z
)
find_library(libcrypto
    crypto
)
find_library(libssl
    ssl
)
find_library(libcurl
    curl
)
find_library(libsqlite3
    sqlite3
)
find_library(libpthread
    pthread
)

target_link_libraries(ClientBL
    Utils
    Event
    Core
	Auto
    libmosquitto
    Network
    ${libz}
    ${libcrypto}
    ${libssl}
    ${libcurl}
    ${libsqlite3}
    ${libpthread}
)

if(DEFINED IC_UNIT_TEST)
	if(${IC_UNIT_TEST} EQUAL 1)

		message([WARNING] "Adding Test Dependencies " )

		find_library(libgtest
				gtest
				)
		find_library(libgmock
				gmock
				)
		include_directories(
				${GTEST_INC}
		)
		target_link_libraries(${PROJECT_NAME}
				gtest
				gmock
				)
	endif ()
endif()
# Expose public includes to other subprojects through cache variable
set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)
