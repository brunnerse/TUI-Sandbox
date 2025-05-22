
add_executable(test_app_basic
        app_basic.cpp
        main.cpp
)

target_link_libraries(test_app_basic
        PRIVATE TUI_App
)

target_include_directories(test_app_basic 
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)

