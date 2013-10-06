message(STATUS "Android NDK toolchain ${ANDROID_NDK_ROOT}")

# specify the cross compiler
set(CMAKE_C_FLAGS "")
set(CMAKE_CXX_FLAGS "")
set(CMAKE_SHARED_LIBRARY_C_FLAGS "") 
set(CMAKE_SHARED_LIBRARY_CXX_FLAGS "")
#set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
#set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

#setup build targets, mutually exclusive
set(PossibleArmTargets "armeabi;armeabi-v7a")
set(ARM_TARGETS "armeabi-v7a" CACHE STRING "the arm targets for android, recommend armeabi-v7a for floating point support and NEON.")
#set_property(CACHE ARM_TARGETS PROPERTY STRINGS ${PossibleArmTargets} )
#set_property(GLOBAL ARM_TARGETS PROPERTY STRINGS ${PossibleArmTargets} )

#set these flags for client use
if(ARM_TARGETS STREQUAL "armeabi")
  set(ARMEABI true)
else()
  set(ARMEABI_V7A true)
endif()

set(SYSROOT "${ANDROID_NDK_ROOT}/platforms/android-8/arch-arm")

set(CMAKE_C_FLAGS   "")
set(CMAKE_CXX_FLAGS "")

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   --sysroot=${SYSROOT}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --sysroot=${SYSROOT}")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -mandroid -ffast-math -Wl,--dynamic-linker=/system/bin/linker -Wl,-rpath=/system/lib")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mandroid -ffast-math -Wl,--dynamic-linker=/system/bin/linker -Wl,-rpath=/system/lib")


#It is recommended to use the -mthumb compiler flag to force the generation of 16-bit Thumb-1 instructions (the default being 32-bit ARM ones).
option(ARM_ENABLE_THUMB "enable thumb instruction set" false)
if(ARM_ENABLE_THUMB)
  set(CMAKE_C_FLAGS "-mthumb")
  set(CMAKE_CXX_FLAGS "-mthumb")
endif(ARM_ENABLE_THUMB)

if(ARMEABI_V7A)
  option(ARM_ENABLE_NEON "enable NEON instruction set" false)

  #these are required flags for android armv7-a
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -march=armv7-a -mfloat-abi=softfp")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv7-a -mfloat-abi=softfp")
  if(ARM_ENABLE_NEON)
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpu=neon")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon")
  else(ARM_ENABLE_NEON)
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpu=vfp")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -mfpu=vfp")
  endif(ARM_ENABLE_NEON)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${ANDROID_NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/include")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${ANDROID_NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/libs/armeabi-v7a/include")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${ANDROID_NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/libs/armeabi-v7a")
else(ARMEABI_V7A)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv5te -mfloat-abi=softfp")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv5te -mfloat-abi=softfp")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${ANDROID_NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/include")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${ANDROID_NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/libs/armeabi/include")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${ANDROID_NDK_ROOT}/sources/cxx-stl/gnu-libstdc++/libs/armeabi")
endif(ARMEABI_V7A)

#Also, is is *required* to use the following linker flags that routes around
#a CPU bug in some Cortex-A8 implementations:
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--fix-cortex-a8")
set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--fix-cortex-a8")
