# ----------------------------------------------------------------------------
#  CMAKE_TOOLCHAIN_FILE for Android NDK r5b
#  Hot to use this file
#  export ANDROID_NDK= <your android NDK installed directory>
#  cmake -DCMAKE_TOOLCHAIN_FILE=android-ndk-r5b-toolchain.cmake
# ----------------------------------------------------------------------------
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)

# set path for android toolchain
set(ANDROID_NDK_ROOT "$ENV{ANDROID_NDK}")

if(NOT EXISTS ${ANDROID_NDK_ROOT})
  message(FATAL_ERROR " ${ANDROID_NDK_ROOT} does not exist!")
endif()

# specify the cross compiler
include(CMakeForceCompiler)
CMAKE_FORCE_C_COMPILER("${ANDROID_NDK_ROOT}/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin/arm-linux-androideabi-gcc"  GNU)
CMAKE_FORCE_CXX_COMPILER("${ANDROID_NDK_ROOT}/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin/arm-linux-androideabi-g++"  GNU)

# where is the target environment 
set(CMAKE_FIND_ROOT_PATH ${ANDROID_NDK_ROOT}/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86 ${ANDROID_NDK_ROOT}/platforms/android-8/arch-arm)

# only search for programs, libraries and includes in the ndk toolchain
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


#set these global flags for cmake client scripts to change behavior
set(ARM Y)
set(ANDROID Y)
set(TS_ANDROID_NDK_TOOLCHAIN Y)
