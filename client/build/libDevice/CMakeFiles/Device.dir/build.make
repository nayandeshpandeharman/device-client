# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /media/sf_GitHub/device-client/client

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /media/sf_GitHub/device-client/client/build

# Include any dependencies generated for this target.
include libDevice/CMakeFiles/Device.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include libDevice/CMakeFiles/Device.dir/compiler_depend.make

# Include the progress variables for this target.
include libDevice/CMakeFiles/Device.dir/progress.make

# Include the compile flags for this target's objects.
include libDevice/CMakeFiles/Device.dir/flags.make

libDevice/CMakeFiles/Device.dir/src/CClientMessageDispatcherImpl.cpp.o: libDevice/CMakeFiles/Device.dir/flags.make
libDevice/CMakeFiles/Device.dir/src/CClientMessageDispatcherImpl.cpp.o: ../libDevice/src/CClientMessageDispatcherImpl.cpp
libDevice/CMakeFiles/Device.dir/src/CClientMessageDispatcherImpl.cpp.o: libDevice/CMakeFiles/Device.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/media/sf_GitHub/device-client/client/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object libDevice/CMakeFiles/Device.dir/src/CClientMessageDispatcherImpl.cpp.o"
	cd /media/sf_GitHub/device-client/client/build/libDevice && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT libDevice/CMakeFiles/Device.dir/src/CClientMessageDispatcherImpl.cpp.o -MF CMakeFiles/Device.dir/src/CClientMessageDispatcherImpl.cpp.o.d -o CMakeFiles/Device.dir/src/CClientMessageDispatcherImpl.cpp.o -c /media/sf_GitHub/device-client/client/libDevice/src/CClientMessageDispatcherImpl.cpp

libDevice/CMakeFiles/Device.dir/src/CClientMessageDispatcherImpl.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Device.dir/src/CClientMessageDispatcherImpl.cpp.i"
	cd /media/sf_GitHub/device-client/client/build/libDevice && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /media/sf_GitHub/device-client/client/libDevice/src/CClientMessageDispatcherImpl.cpp > CMakeFiles/Device.dir/src/CClientMessageDispatcherImpl.cpp.i

libDevice/CMakeFiles/Device.dir/src/CClientMessageDispatcherImpl.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Device.dir/src/CClientMessageDispatcherImpl.cpp.s"
	cd /media/sf_GitHub/device-client/client/build/libDevice && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /media/sf_GitHub/device-client/client/libDevice/src/CClientMessageDispatcherImpl.cpp -o CMakeFiles/Device.dir/src/CClientMessageDispatcherImpl.cpp.s

libDevice/CMakeFiles/Device.dir/src/CCpuLoad.cpp.o: libDevice/CMakeFiles/Device.dir/flags.make
libDevice/CMakeFiles/Device.dir/src/CCpuLoad.cpp.o: ../libDevice/src/CCpuLoad.cpp
libDevice/CMakeFiles/Device.dir/src/CCpuLoad.cpp.o: libDevice/CMakeFiles/Device.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/media/sf_GitHub/device-client/client/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object libDevice/CMakeFiles/Device.dir/src/CCpuLoad.cpp.o"
	cd /media/sf_GitHub/device-client/client/build/libDevice && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT libDevice/CMakeFiles/Device.dir/src/CCpuLoad.cpp.o -MF CMakeFiles/Device.dir/src/CCpuLoad.cpp.o.d -o CMakeFiles/Device.dir/src/CCpuLoad.cpp.o -c /media/sf_GitHub/device-client/client/libDevice/src/CCpuLoad.cpp

libDevice/CMakeFiles/Device.dir/src/CCpuLoad.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Device.dir/src/CCpuLoad.cpp.i"
	cd /media/sf_GitHub/device-client/client/build/libDevice && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /media/sf_GitHub/device-client/client/libDevice/src/CCpuLoad.cpp > CMakeFiles/Device.dir/src/CCpuLoad.cpp.i

libDevice/CMakeFiles/Device.dir/src/CCpuLoad.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Device.dir/src/CCpuLoad.cpp.s"
	cd /media/sf_GitHub/device-client/client/build/libDevice && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /media/sf_GitHub/device-client/client/libDevice/src/CCpuLoad.cpp -o CMakeFiles/Device.dir/src/CCpuLoad.cpp.s

libDevice/CMakeFiles/Device.dir/src/CDefaultProductImpl.cpp.o: libDevice/CMakeFiles/Device.dir/flags.make
libDevice/CMakeFiles/Device.dir/src/CDefaultProductImpl.cpp.o: ../libDevice/src/CDefaultProductImpl.cpp
libDevice/CMakeFiles/Device.dir/src/CDefaultProductImpl.cpp.o: libDevice/CMakeFiles/Device.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/media/sf_GitHub/device-client/client/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object libDevice/CMakeFiles/Device.dir/src/CDefaultProductImpl.cpp.o"
	cd /media/sf_GitHub/device-client/client/build/libDevice && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT libDevice/CMakeFiles/Device.dir/src/CDefaultProductImpl.cpp.o -MF CMakeFiles/Device.dir/src/CDefaultProductImpl.cpp.o.d -o CMakeFiles/Device.dir/src/CDefaultProductImpl.cpp.o -c /media/sf_GitHub/device-client/client/libDevice/src/CDefaultProductImpl.cpp

libDevice/CMakeFiles/Device.dir/src/CDefaultProductImpl.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Device.dir/src/CDefaultProductImpl.cpp.i"
	cd /media/sf_GitHub/device-client/client/build/libDevice && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /media/sf_GitHub/device-client/client/libDevice/src/CDefaultProductImpl.cpp > CMakeFiles/Device.dir/src/CDefaultProductImpl.cpp.i

libDevice/CMakeFiles/Device.dir/src/CDefaultProductImpl.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Device.dir/src/CDefaultProductImpl.cpp.s"
	cd /media/sf_GitHub/device-client/client/build/libDevice && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /media/sf_GitHub/device-client/client/libDevice/src/CDefaultProductImpl.cpp -o CMakeFiles/Device.dir/src/CDefaultProductImpl.cpp.s

libDevice/CMakeFiles/Device.dir/src/CDeviceCommandHandlerImpl.cpp.o: libDevice/CMakeFiles/Device.dir/flags.make
libDevice/CMakeFiles/Device.dir/src/CDeviceCommandHandlerImpl.cpp.o: ../libDevice/src/CDeviceCommandHandlerImpl.cpp
libDevice/CMakeFiles/Device.dir/src/CDeviceCommandHandlerImpl.cpp.o: libDevice/CMakeFiles/Device.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/media/sf_GitHub/device-client/client/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object libDevice/CMakeFiles/Device.dir/src/CDeviceCommandHandlerImpl.cpp.o"
	cd /media/sf_GitHub/device-client/client/build/libDevice && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT libDevice/CMakeFiles/Device.dir/src/CDeviceCommandHandlerImpl.cpp.o -MF CMakeFiles/Device.dir/src/CDeviceCommandHandlerImpl.cpp.o.d -o CMakeFiles/Device.dir/src/CDeviceCommandHandlerImpl.cpp.o -c /media/sf_GitHub/device-client/client/libDevice/src/CDeviceCommandHandlerImpl.cpp

libDevice/CMakeFiles/Device.dir/src/CDeviceCommandHandlerImpl.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Device.dir/src/CDeviceCommandHandlerImpl.cpp.i"
	cd /media/sf_GitHub/device-client/client/build/libDevice && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /media/sf_GitHub/device-client/client/libDevice/src/CDeviceCommandHandlerImpl.cpp > CMakeFiles/Device.dir/src/CDeviceCommandHandlerImpl.cpp.i

libDevice/CMakeFiles/Device.dir/src/CDeviceCommandHandlerImpl.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Device.dir/src/CDeviceCommandHandlerImpl.cpp.s"
	cd /media/sf_GitHub/device-client/client/build/libDevice && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /media/sf_GitHub/device-client/client/libDevice/src/CDeviceCommandHandlerImpl.cpp -o CMakeFiles/Device.dir/src/CDeviceCommandHandlerImpl.cpp.s

libDevice/CMakeFiles/Device.dir/src/CDeviceInfo.cpp.o: libDevice/CMakeFiles/Device.dir/flags.make
libDevice/CMakeFiles/Device.dir/src/CDeviceInfo.cpp.o: ../libDevice/src/CDeviceInfo.cpp
libDevice/CMakeFiles/Device.dir/src/CDeviceInfo.cpp.o: libDevice/CMakeFiles/Device.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/media/sf_GitHub/device-client/client/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object libDevice/CMakeFiles/Device.dir/src/CDeviceInfo.cpp.o"
	cd /media/sf_GitHub/device-client/client/build/libDevice && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT libDevice/CMakeFiles/Device.dir/src/CDeviceInfo.cpp.o -MF CMakeFiles/Device.dir/src/CDeviceInfo.cpp.o.d -o CMakeFiles/Device.dir/src/CDeviceInfo.cpp.o -c /media/sf_GitHub/device-client/client/libDevice/src/CDeviceInfo.cpp

libDevice/CMakeFiles/Device.dir/src/CDeviceInfo.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Device.dir/src/CDeviceInfo.cpp.i"
	cd /media/sf_GitHub/device-client/client/build/libDevice && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /media/sf_GitHub/device-client/client/libDevice/src/CDeviceInfo.cpp > CMakeFiles/Device.dir/src/CDeviceInfo.cpp.i

libDevice/CMakeFiles/Device.dir/src/CDeviceInfo.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Device.dir/src/CDeviceInfo.cpp.s"
	cd /media/sf_GitHub/device-client/client/build/libDevice && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /media/sf_GitHub/device-client/client/libDevice/src/CDeviceInfo.cpp -o CMakeFiles/Device.dir/src/CDeviceInfo.cpp.s

libDevice/CMakeFiles/Device.dir/src/CZMQClient.cpp.o: libDevice/CMakeFiles/Device.dir/flags.make
libDevice/CMakeFiles/Device.dir/src/CZMQClient.cpp.o: ../libDevice/src/CZMQClient.cpp
libDevice/CMakeFiles/Device.dir/src/CZMQClient.cpp.o: libDevice/CMakeFiles/Device.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/media/sf_GitHub/device-client/client/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object libDevice/CMakeFiles/Device.dir/src/CZMQClient.cpp.o"
	cd /media/sf_GitHub/device-client/client/build/libDevice && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT libDevice/CMakeFiles/Device.dir/src/CZMQClient.cpp.o -MF CMakeFiles/Device.dir/src/CZMQClient.cpp.o.d -o CMakeFiles/Device.dir/src/CZMQClient.cpp.o -c /media/sf_GitHub/device-client/client/libDevice/src/CZMQClient.cpp

libDevice/CMakeFiles/Device.dir/src/CZMQClient.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Device.dir/src/CZMQClient.cpp.i"
	cd /media/sf_GitHub/device-client/client/build/libDevice && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /media/sf_GitHub/device-client/client/libDevice/src/CZMQClient.cpp > CMakeFiles/Device.dir/src/CZMQClient.cpp.i

libDevice/CMakeFiles/Device.dir/src/CZMQClient.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Device.dir/src/CZMQClient.cpp.s"
	cd /media/sf_GitHub/device-client/client/build/libDevice && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /media/sf_GitHub/device-client/client/libDevice/src/CZMQClient.cpp -o CMakeFiles/Device.dir/src/CZMQClient.cpp.s

libDevice/CMakeFiles/Device.dir/src/CZMQDeviceMessageSenderImpl.cpp.o: libDevice/CMakeFiles/Device.dir/flags.make
libDevice/CMakeFiles/Device.dir/src/CZMQDeviceMessageSenderImpl.cpp.o: ../libDevice/src/CZMQDeviceMessageSenderImpl.cpp
libDevice/CMakeFiles/Device.dir/src/CZMQDeviceMessageSenderImpl.cpp.o: libDevice/CMakeFiles/Device.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/media/sf_GitHub/device-client/client/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object libDevice/CMakeFiles/Device.dir/src/CZMQDeviceMessageSenderImpl.cpp.o"
	cd /media/sf_GitHub/device-client/client/build/libDevice && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT libDevice/CMakeFiles/Device.dir/src/CZMQDeviceMessageSenderImpl.cpp.o -MF CMakeFiles/Device.dir/src/CZMQDeviceMessageSenderImpl.cpp.o.d -o CMakeFiles/Device.dir/src/CZMQDeviceMessageSenderImpl.cpp.o -c /media/sf_GitHub/device-client/client/libDevice/src/CZMQDeviceMessageSenderImpl.cpp

libDevice/CMakeFiles/Device.dir/src/CZMQDeviceMessageSenderImpl.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Device.dir/src/CZMQDeviceMessageSenderImpl.cpp.i"
	cd /media/sf_GitHub/device-client/client/build/libDevice && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /media/sf_GitHub/device-client/client/libDevice/src/CZMQDeviceMessageSenderImpl.cpp > CMakeFiles/Device.dir/src/CZMQDeviceMessageSenderImpl.cpp.i

libDevice/CMakeFiles/Device.dir/src/CZMQDeviceMessageSenderImpl.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Device.dir/src/CZMQDeviceMessageSenderImpl.cpp.s"
	cd /media/sf_GitHub/device-client/client/build/libDevice && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /media/sf_GitHub/device-client/client/libDevice/src/CZMQDeviceMessageSenderImpl.cpp -o CMakeFiles/Device.dir/src/CZMQDeviceMessageSenderImpl.cpp.s

libDevice/CMakeFiles/Device.dir/src/CZMQReceiveMessage.cpp.o: libDevice/CMakeFiles/Device.dir/flags.make
libDevice/CMakeFiles/Device.dir/src/CZMQReceiveMessage.cpp.o: ../libDevice/src/CZMQReceiveMessage.cpp
libDevice/CMakeFiles/Device.dir/src/CZMQReceiveMessage.cpp.o: libDevice/CMakeFiles/Device.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/media/sf_GitHub/device-client/client/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object libDevice/CMakeFiles/Device.dir/src/CZMQReceiveMessage.cpp.o"
	cd /media/sf_GitHub/device-client/client/build/libDevice && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT libDevice/CMakeFiles/Device.dir/src/CZMQReceiveMessage.cpp.o -MF CMakeFiles/Device.dir/src/CZMQReceiveMessage.cpp.o.d -o CMakeFiles/Device.dir/src/CZMQReceiveMessage.cpp.o -c /media/sf_GitHub/device-client/client/libDevice/src/CZMQReceiveMessage.cpp

libDevice/CMakeFiles/Device.dir/src/CZMQReceiveMessage.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Device.dir/src/CZMQReceiveMessage.cpp.i"
	cd /media/sf_GitHub/device-client/client/build/libDevice && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /media/sf_GitHub/device-client/client/libDevice/src/CZMQReceiveMessage.cpp > CMakeFiles/Device.dir/src/CZMQReceiveMessage.cpp.i

libDevice/CMakeFiles/Device.dir/src/CZMQReceiveMessage.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Device.dir/src/CZMQReceiveMessage.cpp.s"
	cd /media/sf_GitHub/device-client/client/build/libDevice && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /media/sf_GitHub/device-client/client/libDevice/src/CZMQReceiveMessage.cpp -o CMakeFiles/Device.dir/src/CZMQReceiveMessage.cpp.s

# Object files for target Device
Device_OBJECTS = \
"CMakeFiles/Device.dir/src/CClientMessageDispatcherImpl.cpp.o" \
"CMakeFiles/Device.dir/src/CCpuLoad.cpp.o" \
"CMakeFiles/Device.dir/src/CDefaultProductImpl.cpp.o" \
"CMakeFiles/Device.dir/src/CDeviceCommandHandlerImpl.cpp.o" \
"CMakeFiles/Device.dir/src/CDeviceInfo.cpp.o" \
"CMakeFiles/Device.dir/src/CZMQClient.cpp.o" \
"CMakeFiles/Device.dir/src/CZMQDeviceMessageSenderImpl.cpp.o" \
"CMakeFiles/Device.dir/src/CZMQReceiveMessage.cpp.o"

# External object files for target Device
Device_EXTERNAL_OBJECTS =

lib/libDevice.a: libDevice/CMakeFiles/Device.dir/src/CClientMessageDispatcherImpl.cpp.o
lib/libDevice.a: libDevice/CMakeFiles/Device.dir/src/CCpuLoad.cpp.o
lib/libDevice.a: libDevice/CMakeFiles/Device.dir/src/CDefaultProductImpl.cpp.o
lib/libDevice.a: libDevice/CMakeFiles/Device.dir/src/CDeviceCommandHandlerImpl.cpp.o
lib/libDevice.a: libDevice/CMakeFiles/Device.dir/src/CDeviceInfo.cpp.o
lib/libDevice.a: libDevice/CMakeFiles/Device.dir/src/CZMQClient.cpp.o
lib/libDevice.a: libDevice/CMakeFiles/Device.dir/src/CZMQDeviceMessageSenderImpl.cpp.o
lib/libDevice.a: libDevice/CMakeFiles/Device.dir/src/CZMQReceiveMessage.cpp.o
lib/libDevice.a: libDevice/CMakeFiles/Device.dir/build.make
lib/libDevice.a: libDevice/CMakeFiles/Device.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/media/sf_GitHub/device-client/client/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Linking CXX static library ../lib/libDevice.a"
	cd /media/sf_GitHub/device-client/client/build/libDevice && $(CMAKE_COMMAND) -P CMakeFiles/Device.dir/cmake_clean_target.cmake
	cd /media/sf_GitHub/device-client/client/build/libDevice && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Device.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
libDevice/CMakeFiles/Device.dir/build: lib/libDevice.a
.PHONY : libDevice/CMakeFiles/Device.dir/build

libDevice/CMakeFiles/Device.dir/clean:
	cd /media/sf_GitHub/device-client/client/build/libDevice && $(CMAKE_COMMAND) -P CMakeFiles/Device.dir/cmake_clean.cmake
.PHONY : libDevice/CMakeFiles/Device.dir/clean

libDevice/CMakeFiles/Device.dir/depend:
	cd /media/sf_GitHub/device-client/client/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /media/sf_GitHub/device-client/client /media/sf_GitHub/device-client/client/libDevice /media/sf_GitHub/device-client/client/build /media/sf_GitHub/device-client/client/build/libDevice /media/sf_GitHub/device-client/client/build/libDevice/CMakeFiles/Device.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : libDevice/CMakeFiles/Device.dir/depend

