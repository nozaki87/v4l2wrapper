message(STATUS "Codesourcery toolchain ${ARM_CODESOURCERY_ROOT}")
# specify the cross compiler
set(CMAKE_C_FLAGS "")
set(CMAKE_CXX_FLAGS "")
set(CMAKE_SHARED_LIBRARY_C_FLAGS "") 
set(CMAKE_SHARED_LIBRARY_CXX_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS "")

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

set(SYSROOT "${ARM_CODESOURCERY_ROOT}/arm-none-linux-gnueabi/libc")

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   --sysroot=${SYSROOT}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --sysroot=${SYSROOT}")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fPIC -ffast-math") # -mbionic -fno-exceptions -fno-rtti 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -ffast-math") # -mbionic  -fno-exceptions -fno-rtti 
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wl,-rpath -Wl,.")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-rpath -Wl,.")


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
else(ARMEABI_V7A)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv5te -mfloat-abi=softfp")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv5te -mfloat-abi=softfp")
endif(ARMEABI_V7A)

message(STATUS "add -static for codesourcery toolchain")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
