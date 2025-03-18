cmake_minimum_required(VERSION 3.4)

project(Network)

add_definitions(-std=c++11 -DCPP11_SUPPORT)

if (FPIC)
add_definitions(-fPIC)
endif()

if (RELEASE_BINARY)
add_definitions(-g0)
endif()

add_definitions(-DCURL_ENABLED)

include_directories(
    include/http
    include/mqtt
    include/igniteHTTP
    ${Core_INCLUDE_DIRS}
    include
    src
    ${Utils_INCLUDE_DIRS}
    ${libmosquitto_INCLUDE_DIRS}
    /opt/local/include
    /usr/local/include
    /opt/local/lib
)

file(
	GLOB_RECURSE CPP_FILES src/http/*.cpp
	GLOB_RECURSE CPP_FILES src/mqtt/*.cpp
	GLOB_RECURSE CPP_FILES src/igniteHTTP/*.cpp
)

if(DEFINED IC_UNIT_TEST)
    if(${IC_UNIT_TEST} EQUAL 1)
		file(
			GLOB_RECURSE TEST_CPP_FILES unittest/*.cpp
		)
        include_directories(
            unittest
        )    
	endif()
endif()

add_library(Network
        ${CPP_FILES}
        ${TEST_CPP_FILES}
    )

find_library(libcrypto
        crypto
        )
find_library(libssl
        ssl
        )

target_link_libraries(Network
    Utils
    libmosquitto
    ${libcrypto}
    ${libssl}
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
