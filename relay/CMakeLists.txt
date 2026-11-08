cmake_minimum_required(VERSION 3.20)
project(ps3_remoteplay_relay LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
enable_testing()

add_library(ps3-relay-packetizer STATIC
    src/packetizer.c
)
target_include_directories(ps3-relay-packetizer
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../shared
)
target_compile_options(ps3-relay-packetizer PRIVATE -Wall -Wextra -Wpedantic)

add_executable(ps3-relay-packetizer-test tests/test_packetizer.c)
target_link_libraries(ps3-relay-packetizer-test PRIVATE ps3-relay-packetizer)
target_compile_options(ps3-relay-packetizer-test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME ps3-relay-packetizer-loopback COMMAND ps3-relay-packetizer-test)

# Optional adapter. Point CHIAKI_INCLUDE_DIR at a Chiaki-ng lib/include tree,
# and CHIAKI_LIBRARY at its built libchiaki static/shared library.
set(CHIAKI_INCLUDE_DIR "" CACHE PATH "Path containing chiaki/session.h")
set(CHIAKI_LIBRARY "" CACHE FILEPATH "Built Chiaki-ng library")
if(CHIAKI_INCLUDE_DIR AND CHIAKI_LIBRARY)
    add_library(ps3-relay-chiaki-adapter STATIC src/chiaki_adapter.c)
    target_include_directories(ps3-relay-chiaki-adapter
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/../shared
            ${CHIAKI_INCLUDE_DIR}
    )
    target_link_libraries(ps3-relay-chiaki-adapter PUBLIC ps3-relay-packetizer ${CHIAKI_LIBRARY})
    target_compile_options(ps3-relay-chiaki-adapter PRIVATE -Wall -Wextra -Wpedantic)
endif()
