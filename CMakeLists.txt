cmake_minimum_required(VERSION 3.5)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
include(spiffsgen/spiffsgen.cmake)
project(Interlock3 C ASM)

add_compile_options(-Wall -Werror -Wpedantic -Werror=vla)