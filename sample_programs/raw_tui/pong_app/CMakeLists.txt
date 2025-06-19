
add_executable(basic_app
        Basic_App.cpp
        main.cpp
)

target_link_libraries(basic_app
        PRIVATE TUI_App
)

target_include_directories(basic_app PRIVATE 
        "${CMAKE_CURRENT_SOURCE_DIR}"
        ${CMAKE_SOURCE_DIR}/src
)

