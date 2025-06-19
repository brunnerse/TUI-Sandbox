
add_executable(mirror
        mirror.cpp
)

target_link_libraries(mirror
        PRIVATE terminal_ctrl
)


# This Executable simply forwards its input stream to its output stream, 
# while simultaneously copying it to a file (given as argument)
add_executable(mirror_with_output
        mirror_with_output.cpp
)

target_link_libraries(mirror_with_output
        PRIVATE terminal_ctrl
)

