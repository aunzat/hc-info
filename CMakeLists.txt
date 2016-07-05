cmake_minimum_required(VERSION 3.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

## hcc compiler
find_program(HCC_COMMAND NAMES hcc HINTS ENV HCC_HOME)
if(NOT HCC_COMMAND)
  message(FATAL_ERROR "hcc compiler not found.")
endif()
set(CMAKE_CXX_COMPILER ${HCC_COMMAND})

## hcc-config
find_program(HCC_CONFIG_COMMAND NAMES hcc-config HINTS ENV HCC_HOME)
if(NOT HCC_CONFIG_COMMAND)
  message(FATAL_ERROR "hcc-config program not found.")
endif()

project(hc-info)


execute_process(COMMAND ${HCC_CONFIG_COMMAND} --cxxflags
  OUTPUT_VARIABLE CMAKE_CXX_FLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${HCC_CONFIG_COMMAND} --ldflags
  OUTPUT_VARIABLE CMAKE_EXE_LINKER_FLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE)

add_executable(hc-info hc-info.cpp)

install(TARGETS hc-info DESTINATION bin)

# tests
include(CTest)

add_test(execute hc-info)

add_test(invalid_path hc-info "no_an_accelerator_path")
set_tests_properties(invalid_path PROPERTIES WILL_FAIL TRUE)
