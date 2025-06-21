list(APPEND analyzer_files 
	use_tee_cmd_manual 
	use_tee_cmd 
	use_pipes
)


foreach (file IN LISTS analyzer_files)
	add_executable(${file}
        	${file}.cpp
	)

	target_link_libraries(${file}
        	terminal_ctrl
		TrafficAnalyzer
	)
endforeach()

set_property(TARGET use_pipes 
	PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
set_property(TARGET use_pipes 
	PROPERTY RUNTIME_OUTPUT_NAME "analyzer"
)



add_library(TrafficAnalyzer
	"traffic_analyzer/terminal_traffic_analyzer.cpp"
)


target_include_directories(TrafficAnalyzer PUBLIC 
        "${CMAKE_CURRENT_SOURCE_DIR}/traffic_analyzer"
        "${CMAKE_SOURCE_DIR}/src"
)
