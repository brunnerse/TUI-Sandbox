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

	std::string esc_code_prefix, esc_code_suffix;
	std::string expression_prefix, expression_suffix;
	std::string description_prefix, description_suffix;
	std::string acs_prefix, acs_suffix;


	FILE *out_file;

	const bool USE_COLORS; 
	const uint32_t OUTPUT_LF_AFTER_MS = 250; 

	// TODO let user configure
	const bool PRINT_COLORS_ONLY = false;
	const bool PRINT_ESC_CODE_DESCRIPTIONS = true;
	const bool PRINT_EXPRESSION_DESCRIPTIONS = true;

    uint32_t last_print_time_ms = 0;

	enum class State {
		TEXT,
		ESC_EXPR	
	};

	State state;

	struct fd_state {
		std::string buffer;
		bool is_in_acs = false;
		const bool single_escapes_possible;
		std::string prefix, suffix; 

		fd_state(bool single_escapes) : single_escapes_possible(single_escapes) {}
	};

	fd_state fd_input = fd_state(true);
	fd_state fd_output = fd_state(false);

	void init_pre_suffixes();

	void capture(char data[], unsigned long size, fd_state *fd_x);

	void print_data(const char data[], size_t length, bool is_acs);

	bool parse_expression(const char* expr, size_t size, char* out_description, size_t out_length); 

	static bool parse_esc_code(char c, const char **out_token, const char **out_description);

};
