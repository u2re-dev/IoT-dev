cmake_minimum_required(VERSION 3.16.0)

#
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)
set(CXX_COMPILE_OPTIONS "-std=gnu++23;-Os" APPEND)
#set(EXTRA_COMPONENT_DIRS "~/esp-matter/" "~/esp-matter/connectedhomeip/connectedhomeip/config/esp32/")

#
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
include(~/esp-matter/generate-include-files.cmake)
project(IoT-dev)
