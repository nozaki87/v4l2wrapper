# cmake file

project(v4l2w_top)

cmake_minimum_required(VERSION 2.6)

if(TS_CODESOURCERY_TOOLCHAIN)
   include("codesourcery.in.cmake")
endif(TS_CODESOURCERY_TOOLCHAIN)
if(TS_ANDROID_NDK_TOOLCHAIN)
   include("android.in.cmake")
endif(TS_ANDROID_NDK_TOOLCHAIN)

if (NOT CMAKE_BUILD_TYPE)
 set(CMAKE_BUILD_TYPE Debug)
endif (NOT CMAKE_BUILD_TYPE)
set(ENABLE_COVERAGE OFF CACHE BOOL "enable coverage support by gcov")

message(STATUS "Build Type: " ${CMAKE_BUILD_TYPE})

# for Dictionary binary
if (ARM)
SET(OBJCOPY_TERGET -O elf32-littlearm -B arm)
else(ARM)
 if (CMAKE_SIZEOF_VOID_P EQUAL 8)
 SET(OBJCOPY_TERGET -O elf64-x86-64 -B i386)
 else (CMAKE_SIZEOF_VOID_P EQUAL 8)
 SET(OBJCOPY_TERGET -O elf32-i386 -B i386)
 endif (CMAKE_SIZEOF_VOID_P EQUAL 8)
endif(ARM)

# BUILD_TYPE
if (UNIX)
  set(CMAKE_C_FLAGS_RELEASE   "-O3 -DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
  set(CMAKE_C_FLAGS_DEBUG     "-O2 -g -pg")
  set(CMAKE_CXX_FLAGS_DEBUG   "-O2 -g -pg")
  set(CMAKE_C_FLAGS           "-msse2")
  set(CMAKE_CXX_FLAGS         "-msse2")
endif(UNIX)

# Coverage
if(ENABLE_COVERAGE)
  set(CMAKE_BUILD_TYPE Debug FORCE)
  message(STATUS "coverage options are added")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
endif(ENABLE_COVERAGE)

add_subdirectory(library)
add_subdirectory(test)

