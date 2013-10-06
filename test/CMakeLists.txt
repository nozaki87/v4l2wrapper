# test
project(v4l2w_test)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/files.cmake/modules")

include_directories(
  ${v4l2w_SOURCE_DIR}/include
)

include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# test target
link_libraries(v4l2w_static)

# external libraries
find_package(OpenCV REQUIRED)
find_package(LIBJPEG REQUIRED)
if(OpenCV_FOUND)
  message(STATUS "Found library: OpenCV")
  include_directories(${OpenCV_INCLUDE_DIRS})
  link_libraries(${OpenCV_LIBS})
endif(OpenCV_FOUND)
if(LIBJPEG_FOUND)
  message(STATUS "Found library: libjpeg")
  include_directories(${LIBJPEG_INCLUDE_DIRS})
  link_libraries(${LIBJPEG_LIBRARIES})
endif(LIBJPEG_FOUND)

add_executable(vga_raw src/vga_raw.cpp)
add_executable(fHD_jpeg src/fHD_jpeg.cpp)
add_executable(fHD_h264_stdout src/fHD_h264_stdout.cpp)
