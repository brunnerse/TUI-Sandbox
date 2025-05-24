cmake_minimum_required(VERSION 3.7.2)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

option(RUN_LINTER "Run clang-tidy linter." OFF)

if (RUN_LINTER)
    set(CMAKE_CXX_CLANG_TIDY
        "clang-tidy;"
    )
endif()

project("TUI-Sandbox" C CXX)


add_compile_options(
    -Wall
    -Wextra
    -Wconversion # Warning for implicit type conversions
    -Wsign-conversion # Warning for sign conversion 
    -Wimplicit-fallthrough  # Warning if switch case falls through (i.e. no break), except when specified e.g. via /*Falls through*/ 
#    -Wswitch-enum # Warning for missing enums in switch statements; not enabled as pico sdk produces warnings
    -Wswitch-default      # Warning if switch has no default case
    -Wfloat-equal       # Warning if equal comparison with floats
    -Wtype-limits
    -Wcomments
    -Wignored-qualifiers
    -Wmisleading-indentation
    -Wuninitialized -Winit-self -Wmaybe-uninitialized
    $<$<COMPILE_LANGUAGE:CXX>:-Wuseless-cast>          # Warning if cast to its own type 
)



add_subdirectory(src)
add_subdirectory(test)
