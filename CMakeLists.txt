cmake_minimum_required (VERSION 3.20)
project(arduino-dsmr-test LANGUAGES CXX)

## Download Catch2 test framework
file(DOWNLOAD
  https://github.com/catchorg/Catch2/releases/download/v3.4.0/catch_amalgamated.hpp
  ${CMAKE_BINARY_DIR}/catch2/catch_amalgamated.hpp
  EXPECTED_MD5 b9e33e9a8198294a87b64dcf641dee16)
file(DOWNLOAD
  https://github.com/catchorg/Catch2/releases/download/v3.4.0/catch_amalgamated.cpp
  ${CMAKE_BINARY_DIR}/catch2/catch_amalgamated.cpp
  EXPECTED_MD5 b4ee03064bf6be8f41313df1649ff7a9)

add_library(catch2
  ${CMAKE_BINARY_DIR}/catch2/catch_amalgamated.hpp
  ${CMAKE_BINARY_DIR}/catch2/catch_amalgamated.cpp)
target_compile_features(catch2 PRIVATE cxx_std_23)
target_include_directories(catch2 INTERFACE ${CMAKE_BINARY_DIR}/catch2)
target_compile_options(catch2 PRIVATE -w)

# Configure arduino_dsmr_test project
file(GLOB_RECURSE arduino_dsmr_test_src_files CONFIGURE_DEPENDS "test/*.h" "test/*.cpp")
add_executable(arduino_dsmr_test
  ${arduino_dsmr_test_src_files}
  ${CMAKE_SOURCE_DIR}/src/dsmr/util.h
  ${CMAKE_SOURCE_DIR}/src/dsmr/fields.h
  ${CMAKE_SOURCE_DIR}/src/dsmr/parser.h
  ${CMAKE_SOURCE_DIR}/src/dsmr/crc16.h
  ${CMAKE_SOURCE_DIR}/src/dsmr/fields.cpp)
target_include_directories(arduino_dsmr_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_precompile_headers(arduino_dsmr_test PRIVATE ${CMAKE_SOURCE_DIR}/test/precompiled_header.h)
target_link_libraries(arduino_dsmr_test catch2)
target_compile_options(arduino_dsmr_test PRIVATE -Wall -Wextra)
