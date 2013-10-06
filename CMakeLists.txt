# cmake file

project(eyetracker_top)

cmake_minimum_required(VERSION 2.6)

if(TS_CODESOURCERY_TOOLCHAIN)
   include("codesourcery.in.cmake")
endif(TS_CODESOURCERY_TOOLCHAIN)
if(TS_ANDROID_NDK_TOOLCHAIN)
   include("android.in.cmake")
endif(TS_ANDROID_NDK_TOOLCHAIN)

SET(ENABLE_DEBUG OFF CACHE BOOL "enable debug support")
SET(ENABLE_PROFILING OFF CACHE BOOL "enable profiling support by gprof")
SET(ENABLE_COVERAGE OFF CACHE BOOL "enable coverage support by gcov")
SET(OPT_LEVEL 3 CACHE STRING "C compiler optimize level")
SET(DEBUGLOG OFF CACHE BOOL "enable log")

# for Dictionary binary
if (ARM)
SET(OBJCOPY_TERGET -O elf32-littlearm -B arm)
else(ARM)
 if ($ENV{MACHTYPE} MATCHES "64")
 SET(OBJCOPY_TERGET -O elf64-x86-64 -B i386)
 else ($ENV{MACHTYPE} MATCHES "64")
 SET(OBJCOPY_TERGET -O elf32-$ENV{MACHTYPE} -B i386)
 endif ($ENV{MACHTYPE} MATCHES "64")
#SET(OBJCOPY_TERGET -O elf32-i386 -B i386)
endif(ARM)

# Optimize level: default 2
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O${OPT_LEVEL}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O${OPT_LEVEL}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg -g -msse2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -g -msse2")

# Profiling?
if(ENABLE_PROFILING)
  message(STATUS "-pg option is added for gprof")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg -g")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -g")
endif()

# Coverage
if(ENABLE_COVERAGE)
  SET(ENABLE_DEBUG "Y")
  message(STATUS "coverage options are added")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
endif() # ENABLE_COVERAGE

# Debug ?
if(ENABLE_DEBUG)
  message(STATUS "-g option is added for debugging")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

add_subdirectory(library)
add_subdirectory(test)

