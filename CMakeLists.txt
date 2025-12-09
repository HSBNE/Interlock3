cmake_minimum_required(VERSION 3.5)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(Interlock3 C ASM)

add_compile_options(-Wall -Werror -Wpedantic -Werror=vla -DconfigSUPPORT_STATIC_ALLOCATION=1)



# Create littlefs image
set(MKLITTLEFS_BIN ${CMAKE_CURRENT_SOURCE_DIR}/tools/mklittlefs)
set(LFS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/littlefs_data)
set(LFS_OUTPUT_BIN ${CMAKE_CURRENT_BINARY_DIR}/littlefs.bin)
add_custom_command(
    OUTPUT ${LFS_OUTPUT_BIN}
    COMMAND ${MKLITTLEFS_BIN} -s 0x20000 -b 4096 -p 256 -c ${LFS_SOURCE_DIR} ${LFS_OUTPUT_BIN}
    COMMENT "Building LittleFS image"
    VERBATIM
)
add_custom_target(build_littlefs ALL
    DEPENDS ${LFS_OUTPUT_BIN}
)