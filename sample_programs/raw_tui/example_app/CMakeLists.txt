
add_executable(example_app
        Example_App.cpp
        main.cpp
)

target_link_libraries(example_app
        PRIVATE TUI_App
)

target_include_directories(example_app PRIVATE 
        "${CMAKE_CURRENT_SOURCE_DIR}"
        ${CMAKE_SOURCE_DIR}/src
)

