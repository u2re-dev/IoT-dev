# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

#
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)
set(CXX_COMPILE_OPTIONS "-std=gnu++23;-Os" APPEND)

#
#idf_build_set_property(CXX_COMPILE_OPTIONS "-std=gnu++23;-Os" APPEND)
idf_component_register(SRCS "main.cpp" "./main.cpp" INCLUDE_DIRS ".")
