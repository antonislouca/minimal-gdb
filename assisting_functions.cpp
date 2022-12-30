#include "mdb_headers.hpp"

void Usage()
{
    print_char(80, '=');
    std::cerr
        << "Supported commands:\n";
    print_char(80, '=');
    std::cerr << "b: [symbol | *0x--address--]\n"
              << "l: List active breakpoints\n"
              << "d n: Delete n-th breakpoint\n"
              << "r: Run program\n"
              << "c: Continue execution\n"
              << "si: Step Instruction\n"
              << "disas: Print Disassembly\n"
              << "syms: Print Symbols\n"
              << "q: Quit"
              << std::endl;
    print_char(80, '=');
}

void print_char(int times, const char ch)
{
    for (int i = 0; i < times; i++)
        putc(ch, stderr);
    putc('\n', stderr);
}

void begin_debuging(debugger *mdb, char *argv[])
{
    const char *prompt = ">>> ";
    char *input_buf = new char[maxbuf];
    Usage();

    fprintf(stderr,
            "Debugging process initialized:\n");

    std::string input;
    bzero(input_buf, maxbuf);
    while (1)
    {
        print_char(80, '=');
        fprintf(stderr,
                "Current PID= %d and Base address: 0x%lx --- Type help for menu\n",
                mdb->tracee_pid, mdb->base_address);
        print_char(80, '=');

        // writing the prompt
        if (write(STDERR_FILENO, prompt, 4) < 0)
        {
            perror("Error while writing");
            exit(-1);
        }

        // read from user blocking syscall
        if (read(STDIN_FILENO, input_buf, maxbuf) < 0)
        {
            bzero(input_buf, maxbuf);
            perror("Error while reading");
            exit(-1);
        }

        // begin command switch
        // check first letter in input string to find the command

        input.assign(input_buf);
        // fprintf(stderr, "Input length: %ld\n", input.length());

        if (strncmp(input_buf, "b ", 2) == 0)
        {
            // command b *address: set breakpoint using address notation
            if (strncmp(input_buf, "b *0x", 5) == 0)
            {
                unsigned long address = std::stol(
                    input.substr(5, input.length() - 1), 0, 16);

                if (DEBUG)
                    fprintf(stderr, "Address: %lx\n", address);

                mdb->set_breakpoint(address);
            }
            else // use the symbol notation to set breakpoints
            {
                mdb->set_bp_symbol(input.substr(2, input.length() - 3));
            }
        }
        // command l
        else if (strncmp(input_buf, "l", 1) == 0 && input.length() == 2)
        {
            mdb->print_active_bp();
        }
        // command d
        else if (strncmp(input_buf, "d ", 2) == 0 && input.length() == 4)
        {
            int index_bp = std::atoi(input.substr(2, input.length() - 1).c_str());
            mdb->delete_bp(index_bp);
        }
        // command r
        else if (strncmp(input_buf, "r", 1) == 0 && input.length() == 2)
        {
            if (mdb->is_traced && !mdb->running)
            {
                mdb->running = true;
                mdb->continue_exec();
            }
            else if (mdb->is_traced && mdb->running)
            {
                fprintf(stderr,
                        "Program is already running.\nDo you want to restart type (Y/y) ");

                char answer;
                std::cin >> answer; // get input
                if (answer == 'Y' || answer == 'y')
                {
                    mdb->restart_debug(argv);
                    mdb->continue_exec();
                }
            }
            else if (!mdb->is_traced && !mdb->running)

            { // not running neither traced-->happens when child process exits
                // and we run again and we need to restart the whole process
                mdb->restart_debug(argv);
                mdb->continue_exec();
            }
        }

        // command c
        else if (strncmp(input_buf, "c", 1) == 0 && input.length() == 2)
        {
            if (mdb->running && mdb->is_traced)
            {
                mdb->continue_exec();
            }
            else
            {
                fprintf(stderr, "Program is not running.\n");
            }
        }
        // command si
        else if (strncmp(input_buf, "si", 2) == 0 && input.length() == 3)
        {

            if (mdb->running && mdb->is_traced)
            {
                mdb->step_instruction(true);
            }
            else
            {
                fprintf(stderr, "Program is not running.\n");
            }
        }
        // command disas
        else if (strncmp(input_buf, "disas", 5) == 0 && input.length() == 6)
        {
            if (mdb->running && mdb->is_traced)
            {
                mdb->disasembly();
            }
            else
            {
                fprintf(stderr, "Program is not running.\n");
            }
        }
        // use q to quit
        else if (strncmp(input_buf, "q", 1) == 0 && input.length() == 2)
        {
            std::cerr << "Debugging process terminated." << std::endl;
            if (mdb->is_traced)
            {
                kill(mdb->tracee_pid, SIGKILL);
            }
            // free if we need to free anything
            // add a function for this
            delete (input_buf);
            return;
        }
        else if (strncmp(input_buf, "syms", 4) == 0 && input.length() == 5)
        {
            mdb->print_symbols();
        }
        else if (strncmp(input_buf, "help", 4) == 0 && input.length() == 5)
        {
            Usage();
        }
        else
        {
            std::cerr << "Unsupported command\n";
            Usage();
        }

        // nullify input buffer for reuse
        bzero(input_buf, maxbuf);
        input.clear();
        fprintf(stderr, "\n");
    }
}