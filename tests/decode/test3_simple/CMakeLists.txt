#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

set(PROJECT_NAME test3_simple)
include(../../cmake/test_template.cmake)

target_sources(app PRIVATE
  ../../fuzz/fuzz_pet.c)

set(py_command_pet
  zcbor
  -c ${CMAKE_CURRENT_LIST_DIR}/../../cases/pet.cddl
  code
  --output-cmake ${PROJECT_BINARY_DIR}/pet.cmake
  -t Pet
  -d
  ${bit_arg}
  )
set(py_command_serial_recovery
  zcbor
  -c ${CMAKE_CURRENT_LIST_DIR}/../../cases/serial_recovery.cddl
  code
  --oh ${PROJECT_BINARY_DIR}/include/serial/serial_recovery_decode.h
  --oht ${PROJECT_BINARY_DIR}/include/serial/serial_recovery_decode_types.h
  --oc ${PROJECT_BINARY_DIR}/src/serial/serial_recovery_decode.c
  -t Upload
  -d
  ${bit_arg}

  # Testing the --include-prefix option
  --include-prefix serial
  )
execute_process(
  COMMAND ${py_command_pet}
  COMMAND_ERROR_IS_FATAL ANY
  )
execute_process(
  COMMAND ${py_command_serial_recovery}
  COMMAND_ERROR_IS_FATAL ANY
  )

include(${PROJECT_BINARY_DIR}/pet.cmake)

zephyr_include_directories(${PROJECT_BINARY_DIR}/include
  ${CMAKE_CURRENT_LIST_DIR}/../../../include)
zephyr_sources(${PROJECT_BINARY_DIR}/src/serial/serial_recovery_decode.c
  ${CMAKE_CURRENT_LIST_DIR}/../../../src/zcbor_decode.c
  ${CMAKE_CURRENT_LIST_DIR}/../../../src/zcbor_common.c)

target_link_libraries(pet PRIVATE zephyr_interface)
target_link_libraries(app PRIVATE pet)
