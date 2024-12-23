cmake_minimum_required(VERSION 3.30.0)

#
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/)
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/components/)

#
project(CPT CPP)

#
set(IDF_PROJECT_EXECUTABLE cpt_fw)
add_executable(${IDF_PROJECT_EXECUTABLE}.elf "src/main.cpp" "src/loop.cpp")
target_link_libraries(${IDF_PROJECT_EXECUTABLE}.elf idf::newlib)
target_include_directories(${IDF_PROJECT_EXECUTABLE}.elf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
idf_build_executable(${IDF_PROJECT_EXECUTABLE}.elf)
