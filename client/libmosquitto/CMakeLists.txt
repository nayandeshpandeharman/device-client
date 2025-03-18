cmake_minimum_required(VERSION 3.4)

project(libmosquitto)

add_definitions(-DWITH_TLS -DWITH_SOCKS -DWITHOUT_ENGINE)

if (FPIC)
add_definitions(-fPIC)
endif()

if (RELEASE_BINARY)
add_definitions(-g0)
endif()

include_directories(
    include
    src
    src/cpp
    /opt/local/include
    /usr/local/include
)

file(
    GLOB_RECURSE CPP_FILES *.cpp
    GLOB_RECURSE C_FILES *.c
)

add_library(${PROJECT_NAME}
    ${CPP_FILES}
    ${C_FILES}
)

target_link_libraries(${PROJECT_NAME}
    ${libcrypto}
)

# Expose public includes to other subprojects through cache variable
set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include;${PROJECT_SOURCE_DIR}/src;${PROJECT_SOURCE_DIR}/src/cpp
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)
