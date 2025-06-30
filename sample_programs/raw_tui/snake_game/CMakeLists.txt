
add_executable(snake_game
        Snake.cpp
        main.cpp
)

target_link_libraries(snake_game
        PRIVATE TUI_App
)

target_include_directories(snake_game PRIVATE 
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_SOURCE_DIR}/src"
)

