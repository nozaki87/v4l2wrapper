# ----------------------------------------------------------------------------
#  CMAKE_TOOLCHAIN_FILE for Android NDK r5b
#  Hot to use this file
#  export ANDROID_NDK= <your android NDK installed directory>
#  cmake -DCMAKE_TOOLCHAIN_FILE=android-ndk-r5b-toolchain.cmake
# ----------------------------------------------------------------------------
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)

# set path for codesourcery toolchain
set(ARM_CODESOURCERY_ROOT "$ENV{ARM_CODESOURCERY_ROOT}")

if(NOT EXISTS ${ARM_CODESOURCERY_ROOT})
  message(FATAL_ERROR " ${ARM_CODESOURCERY_ROOT} does not exist!")
endif()

# specify the cross compiler
include(CMakeForceCompiler)
CMAKE_FORCE_C_COMPILER(${ARM_CODESOURCERY_ROOT}/bin/arm-none-linux-gnueabi-gcc GNU)
CMAKE_FORCE_CXX_COMPILER(${ARM_CODESOURCERY_ROOT}/bin/arm-none-linux-gnueabi-g++ GNU)

# where is the target environment 
set(CMAKE_FIND_ROOT_PATH ${ARM_CODESOURCERY_ROOT})

# only search for programs, libraries and includes
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

#set these global flags for cmake client scripts to change behavior
set(ARM Y)
set(TS_CODESOURCERY_TOOLCHAIN Y)
