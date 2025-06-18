
add_executable(tcdebug
        tcdebug.cpp
)

target_link_libraries(tcdebug
        terminal_ctrl
	TrafficAnalyzer
)



add_executable(tcdebug_tee
        tcdebug.cpp
)

target_link_libraries(tcdebug_tee
        terminal_ctrl
	TrafficAnalyzer
)

add_executable(tc_intercept
        tc_intercept.cpp
)

target_link_libraries(tc_intercept
        terminal_ctrl
	TrafficAnalyzer
)


add_executable(tc_intercept_manual
        tc_intercept_manual.cpp
)


target_link_libraries(tc_intercept_manual
        terminal_ctrl
	TrafficAnalyzer
)



add_library(TrafficAnalyzer
	terminal_traffic_analyzer.cpp
)


target_include_directories(TrafficAnalyzer PUBLIC 
        "${CMAKE_CURRENT_SOURCE_DIR}"
        ${CMAKE_SOURCE_DIR}/src
)
