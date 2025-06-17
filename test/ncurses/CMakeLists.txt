

find_package(Curses REQUIRED)

# See https://cmake.org/cmake/help/latest/module/FindCurses.html

include_directories(${CURSES_INCLUDE_DIRS})

link_libraries(${CURSES_LIBRARIES})

add_executable(basic
        basic.cpp
)

#target_compile_options(basic PUBLIC -lncurses)

add_executable(sandbox
        sandbox.cpp
)

