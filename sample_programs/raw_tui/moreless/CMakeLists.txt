
add_executable(moreless
        MoreLess_App.cpp
        main.cpp
)

target_link_libraries(moreless
        PRIVATE TUI_App
)

target_include_directories(moreless PRIVATE 
        "${CMAKE_CURRENT_SOURCE_DIR}"
        ${CMAKE_SOURCE_DIR}/src
)

