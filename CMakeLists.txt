cmake_minimum_required(VERSION 3.10)
project(embedded_rtos_telemetry_engine C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(include)

add_library(telemetry_core STATIC
    src/crc_engine.c
    src/ring_buffer.c
    src/packet_parser.c
    src/modbus_rtu.c
    src/can_filter.c
)

add_executable(telemetry_daemon src/telemetry_daemon.c)
target_link_libraries(telemetry_daemon telemetry_core)

enable_testing()
add_executable(test_telemetry tests/test_telemetry.c)
target_link_libraries(test_telemetry telemetry_core)
add_test(NAME telemetry_tests COMMAND test_telemetry)
