#include "mdb_headers.hpp"

debugger::debugger(char *argv[])
{
    this->elf = new elf_file;
    this->symbol_mappings = new std::map<std::string, long>();
    // this->active_breakpoints = 0;
    // this->pedding_Waits = 0;
    this->tracee_pid = 0;
    this->breakpoints = new std::map<std::intptr_t, breakpoint>();
    this->running = false;
    this->base_address = 0;
    this->argv = argv;
    this->disas = new disassembler(argv[1]);
    load_elf(argv[1], elf);              // load and initialize elf
    debug_symbols(elf, symbol_mappings); // map symbols and addresses
    this->start_tracing(argv);
    this->read_base_Address();
}

debugger::~debugger()
{
    delete this->elf;
    delete this->symbol_mappings;
    delete this->breakpoints;
    delete this->disas;
}

void debugger::read_base_Address()
{
    char filename[256];
    sprintf(filename, "/proc/%d/maps", this->tracee_pid);
    std::ifstream proc_maps(filename); // open /procs/maps in read only mode
    std::string line;
    std::getline(proc_maps, line);
    std::string address = line.substr(0, line.find('-'));
    this->base_address = std::stol(address, 0, 16);
}

void debugger::start_tracing(char *argv[])
{
    /*
   fork() for executing the program that is analyzed.
   create the tracee process
   */

    pid_t pid = fork();
    switch (pid)
    {
    case -1: /* error */
        die("%s", strerror(errno));
    case 0:                             /* Code that is run by the child. */
                                        /* Start tracing.  */
        personality(ADDR_NO_RANDOMIZE); //@ disable ASLR

        ptrace(PTRACE_TRACEME, 0, 0, 0);
        /* execvp() is a system call, the child will block and
           the parent must do waitpid().
           The waitpid() of the parent is in the label
           waitpid_for_execvp.
         */
        execvp(argv[1], argv + 1);
        die("%s", strerror(errno));
    }
    this->tracee_pid = pid;
    is_traced = true;
    ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL);
    waitpid(pid, 0, 0); // wait for sigtrap singal from child process
}

void debugger::print_symbols()
{
    print_char(80, '=');
    for (auto itr = symbol_mappings->begin(); itr != symbol_mappings->end(); itr++)
    {
        fprintf(stderr, "Symbol: %s @ 0x%lx\n", (*itr).first.c_str(),
                ((*itr).second + this->base_address));
    }
    print_char(80, '=');
}

void debugger::print_active_bp()
{
    std::cerr << "Active breakpoints: "
              << this->breakpoints->size() << std::endl;

    int count = 1;
    for (auto itr = breakpoints->begin(); itr != breakpoints->end(); itr++)
    {
        fprintf(stderr, "Breakpoint %d: @ 0x%lx ---- old code: ", count, (*itr).first);
        this->disas->disas_old_code(this->elf, (*itr).first,
                                    (uint64_t)base_address);
        count++;
    }
}
void debugger::reapply_bp()
{
    for (auto itr = breakpoints->begin(); itr != breakpoints->end(); itr++)
    {
        (*itr).second.pid = this->tracee_pid;
        (*itr).second.enable_bp();
    }
}
void debugger::enable_all()
{
    for (auto itr = breakpoints->begin(); itr != breakpoints->end(); itr++)
    {
        (*itr).second.enable_bp();
    }
}
void debugger::disable_all()
{
    for (auto itr = breakpoints->begin(); itr != breakpoints->end(); itr++)
    {
        (*itr).second.disable_bp();
    }
}
void debugger::restart_debug(char *argv[])
{
    // kill process if is being traced
    kill(this->tracee_pid, SIGKILL);
    this->start_tracing(argv);
    read_base_Address(); //! check
    this->reapply_bp();
    this->running = true;
    this->is_traced = true;
}

void debugger::restart_tracing(char *argv[])
{
    this->start_tracing(argv);
    this->reapply_bp();
    read_base_Address(); //! check
    this->is_traced = true;
}

void debugger::delete_bp(int indext_bp)
{

    if (breakpoints->size() == 0)
    {
        fprintf(stderr, "No breakpoints to delete\n");
    }
    if (is_traced)
    {
        this->disable_all();
    }

    int count = 1;
    for (auto itr = breakpoints->begin(); itr != breakpoints->end(); itr++)
    {
        if (count == indext_bp)
        { // if the count is equal to the index, delete that element

            const intptr_t address = (*itr).first;

            breakpoints->erase((*itr).first); // this is ok since only one element is
                                              // removed per call
            fprintf(stderr, "Breakpoint @ address 0x%lx deleted\n",
                    address);
            break;
        }
        count++;
    }
    if (is_traced)
    {
        this->enable_all();
    }
}

/*
We want to take an address, set the break point for that address
and store in a map the original instruction for that break point.
*/
void debugger::set_breakpoint(unsigned long breakpoint_address)
{
    if (breakpoint_address > (this->elf->text_end + this->base_address) &&
        breakpoint_address < (this->elf->text_start + this->base_address))
    {
        fprintf(stderr, "Invalid address\n");
        return;
    }

    if (breakpoints->count(breakpoint_address) > 0)
    {
        std::cerr << "Breakpoint already exists" << std::endl;
        return;
    }

    if (!is_traced) // this is for when we child process ends and we want to add BP before
                    // re running the process
    {
        this->restart_tracing(this->argv);
    }

    breakpoint bp = {breakpoint_address, tracee_pid};
    if (bp.enable_bp())
    {
        // add break point to map
        breakpoints->insert(std::make_pair(breakpoint_address, bp));

        // active_breakpoints++; // increase active break points
        fprintf(stderr, "Breakpoint added successfully @ 0x%lx\nActive breakpoints: %ld\n",
                breakpoint_address, breakpoints->size());
    }
}

void debugger::set_bp_symbol(std::string symbol)
{
    // if (DEBUG)
    //     std::cerr << "symbol: " << symbol << std::endl;

    // check if key exists in map
    if (this->symbol_mappings->count(symbol) > 0)
    {
        // get address using symbol mappings
        long address = this->symbol_mappings->at(symbol);
        // fprintf(stderr, "Address: 0x%lx, base: %lx\n", address, base_address);

        address = address + this->base_address;
        // add break point using the same method as the address one
        this->set_breakpoint(address);
    }
    else
    {
        std::cerr << "Cannot add breakpoint. Invalid symbol." << std::endl;
    }
}

void debugger::serve_breakpoint()
{

    struct user_regs_struct regs;

    if (ptrace(PTRACE_GETREGS, tracee_pid, 0, &regs) == -1)
        die("(getregs) %s", strerror(errno));
    // rip executes int3 and stopes to the next instruction
    // since int3 is one byte it stops to bp address +1
    unsigned long bp_Address = regs.rip - 1;
    // fprintf(stderr, "RIP value: 0x%llx--Breakpoint Adrress: 0x%lx\n",
    //         regs.rip, bp_Address);

    if (breakpoints->count(bp_Address) > 0)
    {
        fprintf(stderr, "Serving BP... @ 0x%lx\n", bp_Address);
        // retrive old code
        // create a mapp from modified code to address
        breakpoint bp = breakpoints->at(bp_Address);
        this->disas->disas_commands(this->elf, (uint64_t)bp.address, (uint64_t)base_address);

        // disable all BP
        this->disable_all();

        regs.rip = bp.address; // restores BP address

        // fprintf(stderr, "Restoring RIP: 0x%llx\n", regs.rip);

        if (ptrace(PTRACE_SETREGS, tracee_pid, 0, &regs) == -1)
            die("(setregs) %s", strerror(errno));

        // Single step the process to execute BP instruction
        // execute the single step functionality
        if (ptrace(PTRACE_SINGLESTEP, this->tracee_pid, 0, 0) == -1)
            die("(singlestep) %s", strerror(errno));

        int status;
        if (waitpid(tracee_pid, &status, 0) == -1)
        {
            perror("waitpid failed\n");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(status))
        {
            std::cerr << "Debugee process exited." << std::endl;
            running = false;   // set that process has now stopped running
            is_traced = false; // set that process stopped being traced
            return;
        }

        if (ptrace(PTRACE_GETREGS, tracee_pid, 0, &regs) == -1)
            die("(getregs) %s", strerror(errno));
        // fprintf(stderr, "Current value RIP: 0x%llx\n", regs.rip);

        // enable all BP
        this->enable_all();
    }
}

void debugger::run()
{

    /* Resume process.  */
    if (ptrace(PTRACE_CONT, tracee_pid, 0, 0) == -1)
        die("(cont) %s", strerror(errno));
    waitpid(tracee_pid, 0, 0);
}
bool debugger::continue_exec()
{
    if (ptrace(PTRACE_CONT, tracee_pid, 0, 0) == -1)
        die("(cont) %s", strerror(errno));

    int status;
    if (waitpid(tracee_pid, &status, 0) == -1)
    {
        perror("waitpid failed\n");
        exit(EXIT_FAILURE);
    }

    if (WIFEXITED(status))
    {
        std::cerr << "Debugee process exited." << std::endl;
        running = false;   // set that process has now stopped running
        is_traced = false; // set that process stopped being traced
        return false;
    }
    else
    {
        fprintf(stderr, "Continuing...\n");
        this->serve_breakpoint();
    }
    return true;
}

void debugger::step_instruction(bool print_disas)
{
    struct user_regs_struct regs;

    if (ptrace(PTRACE_GETREGS, tracee_pid, 0, &regs) == -1)
        die("(getregs) %s", strerror(errno));

    if (print_disas)
    {

        this->disas->disas_single_cmd(this->elf, (uint64_t)regs.rip,
                                      (uint64_t)base_address);
    }
    // disable all BP
    this->disable_all();
    // execute the single step functionality
    if (ptrace(PTRACE_SINGLESTEP, this->tracee_pid, 0, 0) == -1)
        die("(singlestep) %s", strerror(errno));

    int status;
    if (waitpid(tracee_pid, &status, 0) == -1)
    {
        perror("waitpid failed\n");
        exit(EXIT_FAILURE);
    }

    if (WIFEXITED(status))
    {
        std::cerr << "Debugee process exited." << std::endl;
        running = false;   // set that process has now stopped running
        is_traced = false; // set that process stopped being traced
        return;
    }
    // enable all BP
    this->enable_all();
}

void debugger::disasembly()
{

    struct user_regs_struct regs;

    if (ptrace(PTRACE_GETREGS, tracee_pid, 0, &regs) == -1)
        die("(getregs) %s", strerror(errno));
    this->disas->disas_commands(this->elf, (uint64_t)regs.rip,
                                (uint64_t)base_address);
}