cmake_minimum_required(VERSION 3.4)

project(Event)

add_definitions(-std=c++11)

if (FPIC)
add_definitions(-fPIC)
endif()

if (RELEASE_BINARY)
add_definitions(-g0)
endif()

file(
    GLOB_RECURSE CPP_FILES src/*.cpp
)

include_directories(
    ${Utils_INCLUDE_DIRS}
    ./include
)

if(DEFINED IC_UNIT_TEST)
    if(${IC_UNIT_TEST} EQUAL 1)
    file(
        GLOB_RECURSE TEST_CPP_FILES unittest/*.cpp
    )
	add_library(${PROJECT_NAME}
	    SHARED
	    ${CPP_FILES}
	    ${TEST_CPP_FILES}
	)
	endif()
else()
    add_library(${PROJECT_NAME}
        SHARED
        ${CPP_FILES}
    )
endif()



find_library(libz
    z
)

set_target_properties(
    ${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(${PROJECT_NAME}
    Utils
    ${libz}
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


