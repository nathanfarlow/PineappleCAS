cmake_minimum_required(VERSION 3.5)
project(PineappleCAS C)

set(CMAKE_C_FLAGS "-std=c89 -Wall -Wextra -Werror -Wno-missing-field-initializers -DCOMPILE_PC")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG")

include_directories(
    src
    src/cas
    src/imath
    src/pc
)

file(GLOB SOURCE_FILES "src/*.[ch]" "src/*/*.[ch]")

add_executable(pineapple ${SOURCE_FILES})

add_custom_target(test
    COMMAND pineapple test tests.txt
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "run automated tests in ${CMAKE_CURRENT_SOURCE_DIR}"
    SOURCES tests.txt
)
add_dependencies(test pineapple)
