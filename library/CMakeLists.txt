project(v4l2w)

cmake_minimum_required(VERSION 2.6)

# Generating Rules for Dictionary as Binary.
set(BINODIR ${PROJECT_BINARY_DIR}/CMakeFiles)
MACRO(ADD_BINOBJ_TARGET _file)
  get_filename_component(filename ${_file} NAME)
  get_filename_component(pathname ${_file} PATH)
  set(output_name ${filename}.o)
  set(BINOBJ ${BINOBJ} ${BINODIR}/${output_name})
  ADD_CUSTOM_COMMAND(
    OUTPUT ${BINODIR}/${output_name}
    COMMAND cd ${pathname} && ${CMAKE_OBJCOPY} -I binary ${OBJCOPY_TERGET} ${filename} ${BINODIR}/${output_name}
    DEPENDS ${_file}
    COMMENT "Generating ${BINODIR}/${output_name} from ${_file}"
    )
ENDMACRO(ADD_BINOBJ_TARGET)
#ADD_BINOBJ_TARGET(some binary)

# Sources
file(GLOB srcs
    "src/*.c"
    "src/*.cpp"
)

include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  )

add_definitions(-DFOOBAR)

add_library("v4l2w_static" ${srcs})
add_library("v4l2w" MODULE ${srcs})

