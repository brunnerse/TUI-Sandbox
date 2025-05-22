cmake_minimum_required(VERSION 3.7.2)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

option(RUN_LINTER "Run clang-tidy linter." OFF)

if (RUN_LINTER)
    set(CMAKE_CXX_CLANG_TIDY
        "clang-tidy;"
    )
endif()

project("Pong-TUI" C CXX)

add_subdirectory(src)
add_subdirectory(test)


