cmake_minimum_required(VERSION 3.4)

project(Core)

add_definitions(-std=c++11 -DCPP11_SUPPORT -DMQTT_SUPPORT)

if (FPIC)
add_definitions(-fPIC)
endif()

if (RELEASE_BINARY)
add_definitions(-g0)
endif()

include_directories(
    include
    src
    ${Utils_INCLUDE_DIRS}
    ${Event_INCLUDE_DIRS}
    ../libNetwork/include/http
    ../libNetwork/include
    /opt/local/include
    /usr/local/include
)

file(
    GLOB_RECURSE CPP_FILES src/*.cpp
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


if(DEFINED IC_UNIT_TEST)
    if (${IC_UNIT_TEST} EQUAL 1)
        file(
            GLOB_RECURSE TEST_CPP_FILES unittest/*.cpp
        )
        add_library(Core
            ${CPP_FILES}
            ${TEST_CPP_FILES}
        )
    else()
        add_library(Core
            ${CPP_FILES}
        )
    endif ()
else()
add_library(Core
    ${CPP_FILES}
)   
endif ()



target_link_libraries(Core
    Utils
    Event
    Network
    ${libz}
    ${libcrypto}
    ${libssl}
    ${libcurl}
    ${libsqlite3}
)


if(DEFINED IC_UNIT_TEST)
    if(${IC_UNIT_TEST} EQUAL 1)

        message([WARNING] "[libCore] Adding Test Dependencies " )

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
