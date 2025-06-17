#include <stdio.h>

#include <string>
#include <unistd.h>

#include "ANSI_Escape_Codes.h"

class TerminalTrafficAnalyzer {

public:
	TerminalTrafficAnalyzer(FILE* out_file)
		:TerminalTrafficAnalyzer(out_file, isatty(fileno(out_file))) // Use colors if output is a terminal 
	{}	

	// Parameter: File Descriptor that TerminalTrafficAnalyzer writes to	
	TerminalTrafficAnalyzer(FILE* out_file, bool use_colors)
	: out_file(out_file), use_colors(use_colors)
	{}	

	void capture_input(char data[], unsigned long size); 
	void capture_output(char data[], unsigned long size); 

private:

	const bool INPUT_WAIT_NEWLINE = false;

	FILE *out_file;
	const bool use_colors; 

	enum class State {
		TEXT,
		ESC_EXPR	
	};

	State state;


	std::string input_buffer;
	std::string output_buffer;

	void parse_expression(const char* expr, unsigned long size);

	static bool parse_esc_code(char c, const char **out_token, const char **out_description);
};
