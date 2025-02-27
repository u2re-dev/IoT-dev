cmake_minimum_required(VERSION 3.16.0)

#
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)
set(CXX_COMPILE_OPTIONS "-std=gnu++23;-DCHIP_HAVE_CONFIG_H;-Os" APPEND)

#
find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO REQUIRED IMPORTED_TARGET "gio-2.0")
#link_libraries(PkgConfig::GIO)

#
set(ESP_MATTER_PATH $ENV{ESP_MATTER_PATH})
set(MATTER_SDK_PATH ${ESP_MATTER_PATH}/connectedhomeip/connectedhomeip)

# This should be done before using the IDF_TARGET variable.
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
include(${ESP_MATTER_PATH}/examples/common/cmake_common/components_include.cmake)
include(${ESP_MATTER_PATH}/generate-include-files.cmake)

#
include_directories("./include")

#
#include_directories("${MATTER_SDK_PATH}")
#include_directories("${MATTER_SDK_PATH}/src")
#include_directories("${MATTER_SDK_PATH}/src/app")
#include_directories("${MATTER_SDK_PATH}/src/include")
#include_directories("${MATTER_SDK_PATH}/zzz_generated/app-common")
#include_directories("${MATTER_SDK_PATH}/third_party/nlassert/repo/include")
#include_directories("${MATTER_SDK_PATH}/third_party/nlio/repo/include")
#include_directories("${MATTER_SDK_PATH}/third_party/jsoncpp/repo/include")
#include_directories("${MATTER_SDK_PATH}/third_party/pigweed/repo")
#include_directories("${MATTER_SDK_PATH}/third_party/pigweed/repo/pw_fuzzer/public")
#include_directories("${MATTER_SDK_PATH}/third_party/pigweed/repo/pw_containers/public")
#include_directories("${MATTER_SDK_PATH}/third_party/pigweed/repo/pw_assert/public")
#include_directories("${MATTER_SDK_PATH}/third_party/pigweed/repo/pw_assert_backend/public")
#include_directories("${ESP_MATTER_PATH}/components/esp_matter")
include_directories("${PROJECT_SOURCE_DIR}/main")
include_directories("${PROJECT_SOURCE_DIR}/main/general")


#
set(EXTRA_COMPONENT_DIRS
    "~/esp-matter/"
    "${ESP_MATTER_PATH}"
    #"${MATTER_SDK_PATH}"
    "${MATTER_SDK_PATH}/config/esp32/components"
    "${ESP_MATTER_PATH}/components"
    ${extra_components_dirs_append})

#
project(IoT-dev)

#
idf_build_set_property(CXX_COMPILE_OPTIONS "-std=gnu++23;-DCHIP_HAVE_CONFIG_H;-Os;-Wno-overloaded-virtual" APPEND)
idf_build_set_property(C_COMPILE_OPTIONS "-Os" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-Wno-format-nonliteral;-Wno-format-security" APPEND)
