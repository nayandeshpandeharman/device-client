cmake_minimum_required(VERSION 3.4)

project(Auto)

add_definitions(-std=c++11 -DCPP11_SUPPORT)

if (FPIC)
add_definitions(-fPIC)
endif()

if (RELEASE_BINARY)
add_definitions(-g0)
endif()

include_directories(
    include
    include/analytics
    src
    ${Utils_INCLUDE_DIRS}
    ${Event_INCLUDE_DIRS}
    ${Core_INCLUDE_DIRS}
    ${Network_INCLUDE_DIRS}
    ${Network_INCLUDE_DIRS}/http    
    ../libClientBL/include  
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
        add_library(Auto
            ${CPP_FILES}
            ${TEST_CPP_FILES}
        )
    else()
        add_library(Auto
            ${CPP_FILES}
        )
	endif()
else()
    add_library(Auto
        ${CPP_FILES}
    )
endif()

if(DEFINED IC_UNIT_TEST)
    if(${IC_UNIT_TEST} EQUAL 1)

        message([WARNING] "[libAuto] Adding Test Dependencies " )

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

find_library(libz
    z
)

target_link_libraries(Auto
    Utils
    Event
    Core
)

# Expose public includes to other subprojects through cache variable
set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)
