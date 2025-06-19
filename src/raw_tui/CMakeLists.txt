
include_directories(${CMAKE_SOURCE_DIR}/src) # For ANSI_Escape_Codes.h

add_library(terminal_ctrl
        tc.cpp
        terminal_cfg.cpp
)

target_include_directories(terminal_ctrl
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)



add_library(TUI_App
        TUI_App.cpp
	print_helper.cpp
)

target_include_directories(TUI_App
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(TUI_App 
        PUBLIC terminal_ctrl
)
