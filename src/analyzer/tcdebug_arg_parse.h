#pragma once

#include <assert.h>
#include <stdio.h>

#include <string>


struct tcdebug_args {
    std::string out_files[2];
    const char *program = nullptr;
    uint32_t delay_after_esc_expr_ms = 0;
    int program_argc = 0;
    char **program_argv;
};


inline void print_usage(char **argv, bool two_output_files, bool program_required, bool ask_delay) 
{
    printf(
        "Usage: %s [-o file.txt] [-o </dev/pts/XX>] %s [<args for program>]\n%s%s%s", 
        argv[0], 
        program_required ? "<program>" : "[program]", 
        "\t-o output file (default: <program>.txt)\n",
        two_output_files ? "\t-o output to another terminal </dev/pts/XX>; "
                            "use output of tty command in other terminal\n"  : "",
        ask_delay ? "\t-d <delay>  wait time in ms after each esc expression\n" : ""
    );
}

inline std::string get_default_out_filename(const tcdebug_args* parsed_args)
{
    // Assign default filename <program>.txt
    std::string filename;

    if (parsed_args->program != nullptr)
    {
        filename.assign(parsed_args->program);
        filename = filename.substr(filename.find_last_of('/')+1);
        filename.append(".txt");
    }
    else
    {
        filename.assign("program.txt");
    }

    return filename;
}

inline void print_parsed_args_info(const tcdebug_args* args)
{
    printf("--------------------\n");
    printf("Executing program '%s", args->program);
    for (int idx = 1; idx < args->program_argc; idx++)
            printf(" %s", args->program_argv[idx]);
    printf("'\n");
    printf("Output to file '%s'\n", args->out_files[0].c_str());
    if (!args->out_files[1].empty())
        printf("   And to file '%s'\n", args->out_files[1].c_str());
    if (args->delay_after_esc_expr_ms > 0)
        printf("\nWaiting %u ms after each escape expression\n", args->delay_after_esc_expr_ms);
    printf("--------------------\n");
}

inline bool tcdebug_parse_args(int argc, char**argv, bool two_output_files, bool program_required, bool ask_delay, tcdebug_args* parsed_args)
{ 
    assert(argc > 0);

    if (argc == 1) {
        print_usage(argv, two_output_files, program_required, ask_delay);
        return false;
    }

    // Iterate through flags; copy up to two filenames
    unsigned out_filename_idx = 0;

    int arg_idx = 1;
    while (arg_idx < argc && argv[arg_idx][0] == '-') {
        switch (argv[arg_idx][1]) {
        case 'o':
            arg_idx++;
            if (!(arg_idx < argc)) 
            {
                print_usage(argv, two_output_files, program_required, ask_delay);
                return false;
            }

            if (out_filename_idx < (two_output_files ? 2 : 1))
            {
                parsed_args->out_files[out_filename_idx++].assign(argv[arg_idx]);
            }
            else {
                fprintf(stderr, "Too many output files given\n"); 
                print_usage(argv, two_output_files, program_required, ask_delay);
                return false;
            }
            break;
        case 'd':
            if (ask_delay) {
                int32_t delay;
                arg_idx++;
                if (!(arg_idx < argc) || ( (delay = atoi(argv[arg_idx])) < 0) ) 
                {
                    print_usage(argv, two_output_files, program_required, ask_delay);
                    return false;
                }
                parsed_args->delay_after_esc_expr_ms = (uint32_t)delay;
                break;
            } else {}
            /* If ask_delay not set, it is an invalid command option => */
            /* INTENTIONAL FALLTHROUGH */
        default:
            fprintf(stderr, "Ignoring invalid command option '%s'\n", argv[arg_idx]);
        }
        arg_idx++;
    }

    // Assuming that <program> is at arg_idx now
    if (arg_idx < argc) 
    {
        parsed_args->program = argv[arg_idx];
    } 
    else
    {
        if (program_required) {
            print_usage(argv, two_output_files, program_required, ask_delay);
            return false;
        } else {
            parsed_args->program = nullptr;
        }
    } 

    parsed_args->program_argc = argc - arg_idx;
    parsed_args->program_argv = argv + arg_idx;

    return true;
}

