#include <stdio.h>

#include <string>
#include <unistd.h>
#include <stdint.h>

#include "ANSI_Escape_Codes.h"

class TerminalTrafficAnalyzer {

public:
	TerminalTrafficAnalyzer(FILE* out_file)
		: TerminalTrafficAnalyzer(out_file, isatty(fileno(out_file))) // Use colors if output is a terminal 
	{}	

	// Parameter: File Descriptor that TerminalTrafficAnalyzer writes to	
	TerminalTrafficAnalyzer(FILE* out_file, bool use_colors)
	: out_file(out_file), USE_COLORS(use_colors)
	{
		init_pre_suffixes();
	}	

	void capture_input(char data[], unsigned long size); 
	void capture_output(char data[], unsigned long size); 

private:

	std::string input_prefix, input_suffix;
	std::string output_prefix, output_suffix;
	std::string expression_prefix, expression_suffix;
	std::string esc_code_prefix, esc_code_suffix;



	FILE *out_file;

	const uint32_t OUTPUT_LF_AFTER_MS = 500; 
	const bool PRINT_COLORS_ONLY = false;
	const bool USE_COLORS; 

    uint32_t last_print_time_ms = 0;

	enum class State {
		TEXT,
		ESC_EXPR	
	};

	State state;


	std::string input_buffer;
	std::string output_buffer;

	void init_pre_suffixes();


	void parse_expression(const char* expr, size_t size, char* out_description, size_t out_length); 

	static bool parse_esc_code(char c, const char **out_token, const char **out_description);
};
