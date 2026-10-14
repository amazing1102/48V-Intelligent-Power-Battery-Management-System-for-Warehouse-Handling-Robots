cmake_minimum_required(VERSION 3.16)
project(BMS48ProCore C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(bms_core
    firmware/src/bms_can.c
    firmware/src/bms_power_control.c
    firmware/src/bms_protection.c
)
target_include_directories(bms_core PUBLIC firmware/include)
if(MSVC)
    target_compile_options(bms_core PRIVATE /W4)
else()
    target_compile_options(bms_core PRIVATE -Wall -Wextra -Werror -Wconversion)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(bms_core_tests tests/test_bms_core.c)
    target_link_libraries(bms_core_tests PRIVATE bms_core)
    if(MSVC)
        target_compile_options(bms_core_tests PRIVATE /W4)
    else()
        target_compile_options(bms_core_tests PRIVATE -Wall -Wextra -Werror -Wconversion)
    endif()
    add_test(NAME bms_core_tests COMMAND bms_core_tests)
endif()
