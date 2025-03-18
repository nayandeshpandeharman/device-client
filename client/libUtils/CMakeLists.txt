cmake_minimum_required(VERSION 3.4)

project(Utils)

add_definitions(-std=c++11 -DCPP11_SUPPORT -pthread)

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
    ./include
	./include/jsoncpp
)

find_library(libpthread
    pthread
)

find_library(libz
    z
)

if(DEFINED IC_UNIT_TEST)
	if (${IC_UNIT_TEST} EQUAL 1)
		file(
			GLOB_RECURSE TEST_CPP_FILES unittest/*.cpp
		)
	endif ()
endif ()

add_library(${PROJECT_NAME}
	${CPP_FILES}
	${TEST_CPP_FILES}
)

set_target_properties(
    ${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON
)


target_link_libraries(${PROJECT_NAME}
    ${libpthread}
	${libz}
)

if(DEFINED IC_UNIT_TEST)
	if(${IC_UNIT_TEST} EQUAL 1)

		message([WARNING] "Adding Test Dependencies " )

		include_directories(
			${GTEST_INC}
		)
		find_library(libgtest
			gtest
		)

		target_link_libraries(${PROJECT_NAME}
				gtest
		)
	endif ()
endif()

# Expose public includes to other subprojects through cache variable
set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)