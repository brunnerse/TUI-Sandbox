
# This Executable simply forwards its input stream to its output stream, 
# while simultaneously copying it to a file (given as argument)
add_executable(mirror
        mirror.cpp
)

target_link_libraries(mirror
        PRIVATE terminal_ctrl
)


