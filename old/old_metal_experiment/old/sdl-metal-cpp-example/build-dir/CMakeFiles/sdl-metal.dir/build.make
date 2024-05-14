# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.29

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
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.29.1/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.29.1/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/dwrr/Documents/projects/block_game/metal_experiment/metal_cpp_example/sdl-metal-cpp-example

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/dwrr/Documents/projects/block_game/metal_experiment/metal_cpp_example/sdl-metal-cpp-example/build-dir

# Include any dependencies generated for this target.
include CMakeFiles/sdl-metal.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/sdl-metal.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/sdl-metal.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/sdl-metal.dir/flags.make

triangle_metallib.h: triangle.metallib
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold --progress-dir=/Users/dwrr/Documents/projects/block_game/metal_experiment/metal_cpp_example/sdl-metal-cpp-example/build-dir/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating triangle_metallib.h"
	/usr/bin/xxd -i triangle.metallib > /Users/dwrr/Documents/projects/block_game/metal_experiment/metal_cpp_example/sdl-metal-cpp-example/build-dir/triangle_metallib.h

triangle.metallib: /Users/dwrr/Documents/projects/block_game/metal_experiment/metal_cpp_example/sdl-metal-cpp-example/triangle.metal
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold --progress-dir=/Users/dwrr/Documents/projects/block_game/metal_experiment/metal_cpp_example/sdl-metal-cpp-example/build-dir/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Generating triangle.metallib"
	cd /Users/dwrr/Documents/projects/block_game/metal_experiment/metal_cpp_example/sdl-metal-cpp-example && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/metal -std=macos-metal2.2 -o /Users/dwrr/Documents/projects/block_game/metal_experiment/metal_cpp_example/sdl-metal-cpp-example/build-dir/triangle.metallib triangle.metal

CMakeFiles/sdl-metal.dir/main.cpp.o: CMakeFiles/sdl-metal.dir/flags.make
CMakeFiles/sdl-metal.dir/main.cpp.o: /Users/dwrr/Documents/projects/block_game/metal_experiment/metal_cpp_example/sdl-metal-cpp-example/main.cpp
CMakeFiles/sdl-metal.dir/main.cpp.o: CMakeFiles/sdl-metal.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/dwrr/Documents/projects/block_game/metal_experiment/metal_cpp_example/sdl-metal-cpp-example/build-dir/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/sdl-metal.dir/main.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/sdl-metal.dir/main.cpp.o -MF CMakeFiles/sdl-metal.dir/main.cpp.o.d -o CMakeFiles/sdl-metal.dir/main.cpp.o -c /Users/dwrr/Documents/projects/block_game/metal_experiment/metal_cpp_example/sdl-metal-cpp-example/main.cpp

CMakeFiles/sdl-metal.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/sdl-metal.dir/main.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/dwrr/Documents/projects/block_game/metal_experiment/metal_cpp_example/sdl-metal-cpp-example/main.cpp > CMakeFiles/sdl-metal.dir/main.cpp.i

CMakeFiles/sdl-metal.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/sdl-metal.dir/main.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/dwrr/Documents/projects/block_game/metal_experiment/metal_cpp_example/sdl-metal-cpp-example/main.cpp -o CMakeFiles/sdl-metal.dir/main.cpp.s

# Object files for target sdl-metal
sdl__metal_OBJECTS = \
"CMakeFiles/sdl-metal.dir/main.cpp.o"

# External object files for target sdl-metal
sdl__metal_EXTERNAL_OBJECTS =

sdl-metal: CMakeFiles/sdl-metal.dir/main.cpp.o
sdl-metal: CMakeFiles/sdl-metal.dir/build.make
sdl-metal: /opt/homebrew/Cellar/sdl2/2.30.1/lib/libSDL2.dylib
sdl-metal: metal-cpp/libMetalCPP.a
sdl-metal: CMakeFiles/sdl-metal.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/dwrr/Documents/projects/block_game/metal_experiment/metal_cpp_example/sdl-metal-cpp-example/build-dir/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable sdl-metal"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/sdl-metal.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/sdl-metal.dir/build: sdl-metal
.PHONY : CMakeFiles/sdl-metal.dir/build

CMakeFiles/sdl-metal.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/sdl-metal.dir/cmake_clean.cmake
.PHONY : CMakeFiles/sdl-metal.dir/clean

CMakeFiles/sdl-metal.dir/depend: triangle.metallib
CMakeFiles/sdl-metal.dir/depend: triangle_metallib.h
	cd /Users/dwrr/Documents/projects/block_game/metal_experiment/metal_cpp_example/sdl-metal-cpp-example/build-dir && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/dwrr/Documents/projects/block_game/metal_experiment/metal_cpp_example/sdl-metal-cpp-example /Users/dwrr/Documents/projects/block_game/metal_experiment/metal_cpp_example/sdl-metal-cpp-example /Users/dwrr/Documents/projects/block_game/metal_experiment/metal_cpp_example/sdl-metal-cpp-example/build-dir /Users/dwrr/Documents/projects/block_game/metal_experiment/metal_cpp_example/sdl-metal-cpp-example/build-dir /Users/dwrr/Documents/projects/block_game/metal_experiment/metal_cpp_example/sdl-metal-cpp-example/build-dir/CMakeFiles/sdl-metal.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/sdl-metal.dir/depend
