cmake_minimum_required(VERSION 3.4)

project(zmq_device_simulator)

add_definitions(-std=c++11)
include_directories()

link_directories()

file(
   GLOB_RECURSE CPP_FILES src/*.cpp
)

add_executable(zmq_device_simulator
	${CPP_FILES}
)

target_link_libraries(zmq_device_simulator
	zmq
)
