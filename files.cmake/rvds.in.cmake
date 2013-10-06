message(STATUS "use ARM RealView Dev. Suite")
message(STATUS "building config.xml by armcc for codesourcery toolchains")
execute_process(COMMAND armcc --arm_linux_configure --arm_linux_config_file=config.xml armcc  --configure_gcc=${ARM_CODESOURCERY_ROOT}/bin/arm-none-linux-gnueabi-gcc --configure_gld=${ARM_CODESOURCERY_ROOT}/bin/arm-none-linux-gnueabi-ld --configure_gld=${ARM_CODESOURCERY_ROOT}/bin/arm-none-linux-gnueabi-ld --configure_extra_includes=${ARM_CODESOURCERY_ROOT}/arm-none-linux-gnueabi/include/c++/4.5.2)
set(CMAKE_C_COMPILER "armcc")
set(CMAKE_CXX_COMPILER "armcc")
set(CMAKE_C_FLAGS "")
set(CMAKE_CXX_FLAGS "")
set(CMAKE_SHARED_LIBRARY_C_FLAGS "") # remove -fPIC
set(CMAKE_SHARED_LIBRARY_CXX_FLAGS "") # remove -fPIC
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "") # remove -rdynamic
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "") # remove -rdynamic
set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "--shared")
set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "--shared")
set(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG "-o")
set(CMAKE_SHARED_LIBRARY_SONAME_CXX_FLAG "-o")
set(CMAKE_EXE_LINKER_FLAGS "")





#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --arm_linux_paths --arm_linux_config_file=${CMAKE_BINARY_DIR}/config.xml --cpu=Cortex-A9.no_neon")
#set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} --arm_linux_paths --arm_linux_config_file=${CMAKE_BINARY_DIR}/config.xml --cpu=Cortex-A9.no_neon --cpp")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --translate_gcc --arm_linux_paths --arm_linux_config_file=${CMAKE_BINARY_DIR}/config.xml -march=armv7-a -mfloat-abi=softfp")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} --translate_gcc --arm_linux_paths --arm_linux_config_file=${CMAKE_BINARY_DIR}/config.xml -march=armv7-a -mfloat-abi=softfp")

set(CMAKE_EXECUTABLE_SUFFIX ".axf")
