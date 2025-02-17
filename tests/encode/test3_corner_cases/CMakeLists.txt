#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

set(PROJECT_NAME test3_strange)
include(../../cmake/test_template.cmake)

set(py_command
  zcbor
  -c ${CMAKE_CURRENT_LIST_DIR}/../../cases/corner_cases.cddl
  --default-max-qty 6
  code
  --output-cmake ${PROJECT_BINARY_DIR}/corner_cases.cmake
  --copy-sources
  -t
  NestedListMap NestedMapListMap
  Numbers
  Numbers2
  Strings
  Prim2
  Optional
  Union
  Map
  Level1
  Range
  ValueRange
  SingleBstr
  SingleInt
  SingleInt2
  Unabstracted
  QuantityRange
  DoubleMap
  Floats
  -e
  ${bit_arg}
  )

execute_process(
  COMMAND ${py_command}
  COMMAND_ERROR_IS_FATAL ANY
  )

include(${PROJECT_BINARY_DIR}/corner_cases.cmake)

target_link_libraries(corner_cases PRIVATE zephyr_interface)
target_link_libraries(app PRIVATE corner_cases)

if (NOT VERBOSE)
  # VERBOSE means including printk which doesn't build with these options.
  target_compile_options(corner_cases PRIVATE -Wpedantic -Wconversion)
endif()
