# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/nokia/sib9/slog

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/nokia/sib9/build

# Include any dependencies generated for this target.
include CMakeFiles/slog-sample.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/slog-sample.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/slog-sample.dir/flags.make

CMakeFiles/slog-sample.dir/example/example.c.o: CMakeFiles/slog-sample.dir/flags.make
CMakeFiles/slog-sample.dir/example/example.c.o: /home/nokia/sib9/slog/example/example.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/nokia/sib9/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/slog-sample.dir/example/example.c.o"
	/usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/slog-sample.dir/example/example.c.o   -c /home/nokia/sib9/slog/example/example.c

CMakeFiles/slog-sample.dir/example/example.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/slog-sample.dir/example/example.c.i"
	/usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/nokia/sib9/slog/example/example.c > CMakeFiles/slog-sample.dir/example/example.c.i

CMakeFiles/slog-sample.dir/example/example.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/slog-sample.dir/example/example.c.s"
	/usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/nokia/sib9/slog/example/example.c -o CMakeFiles/slog-sample.dir/example/example.c.s

# Object files for target slog-sample
slog__sample_OBJECTS = \
"CMakeFiles/slog-sample.dir/example/example.c.o"

# External object files for target slog-sample
slog__sample_EXTERNAL_OBJECTS =

slog-sample: CMakeFiles/slog-sample.dir/example/example.c.o
slog-sample: CMakeFiles/slog-sample.dir/build.make
slog-sample: libslog.a
slog-sample: CMakeFiles/slog-sample.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/nokia/sib9/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable slog-sample"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/slog-sample.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/slog-sample.dir/build: slog-sample

.PHONY : CMakeFiles/slog-sample.dir/build

CMakeFiles/slog-sample.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/slog-sample.dir/cmake_clean.cmake
.PHONY : CMakeFiles/slog-sample.dir/clean

CMakeFiles/slog-sample.dir/depend:
	cd /home/nokia/sib9/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/nokia/sib9/slog /home/nokia/sib9/slog /home/nokia/sib9/build /home/nokia/sib9/build /home/nokia/sib9/build/CMakeFiles/slog-sample.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/slog-sample.dir/depend
