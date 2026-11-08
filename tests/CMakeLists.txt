cmake_minimum_required(VERSION 3.20)
project(ps3_direct_chiaki_host_tests LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
enable_testing()

add_executable(test_direct_chiaki_config
    test_direct_chiaki_config.c
    ../src/direct_chiaki_config.c
)
target_include_directories(test_direct_chiaki_config PRIVATE ../src)
target_compile_options(test_direct_chiaki_config PRIVATE -Wall -Wextra -Wpedantic -Werror)
add_test(NAME direct-chiaki-config-valid-profile COMMAND test_direct_chiaki_config)
