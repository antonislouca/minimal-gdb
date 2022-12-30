#pragma once
/* C standard library */
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/* POSIX */
#include <unistd.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <libelf.h>
#include <gelf.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <bits/stdc++.h>

/* Linux */
#include <syscall.h>
#include <sys/ptrace.h>
#include <sys/personality.h>

/*C++ headers*/
#include <iostream>
#include <string>
#include <map>
#include <list>
#include <sstream>

#include "elf_loader.hpp"
#include "breakpoint.hpp"
#include "disassembler.hpp"

#define TOOL "mdb"
#define maxbuf 1024
#define DEBUG 1

//@ macros

#define die(...)                                \
    do                                          \
    {                                           \
        fprintf(stderr, TOOL ": " __VA_ARGS__); \
        fputc('\n', stderr);                    \
        exit(EXIT_FAILURE);                     \
    } while (0)

//@structs and classes

class debugger
{
private:
public:
    elf_file *elf;                                    // mapped elf representation
    std::map<std::string, long> *symbol_mappings;     // symbol mappings
    std::map<std::intptr_t, breakpoint> *breakpoints; // active breakpoints

    long base_address;
    // int active_breakpoints;

    disassembler *disas;
    // int pedding_Waits;
    pid_t tracee_pid;
    bool running = false;

    bool is_traced = false;
    char **argv;

    //@functions

    // constructor
    debugger(char *argv[]);
    //  destructor
    ~debugger();
    /*
    reads /proc/pid/maps to get the base address for the
    child process
    */
    void read_base_Address();

    /*
  Begins the tracing process, by creating tracer and tracee processes
  */
    void start_tracing(char *argv[]);

    /*function that prints symbols*/
    void print_symbols();
    /*
    Function that is called using the debugger object and sets the break point
    for the given address.
    */

    void set_breakpoint(unsigned long breakpoint_address);

    /*function that sets a breakpoint using symbol str*/
    void set_bp_symbol(std::string input);

    /*function that prints a list of the active breakpoints*/
    void print_active_bp();

    /*
    function that deletes breakpoint @ the index given and uses ptrace to
    restore the code to that particular address
    */
    void delete_bp(int indext_bp);

    /*command that starts the process*/
    void run();

    /*
    function that serves each break point, it restores original code
    and resets the program counter to execute that command
    */
    void serve_breakpoint();

    /*
    function that tells the process to continue and then wait for signal
    Returns true if child process is still running
    Reuturn false if child process exited, thus continuing execution was not
    succesful
    */
    bool continue_exec();

    /*Restarts the debug process, by recreating a new process
    and reapplying breakpoints.
    This function will also run the program and cause it ot hit first
    break point
    */
    void restart_debug(char *argv[]);
    /*
    Restarts the debug process, by recreating a new process
   and reapplying breakpoints. This function does not run the program
   */
    void restart_tracing(char *argv[]);

    /*function for reapplying already set breakpoints if exist*/
    void reapply_bp();

    /*function that performs the step instruction command and prints disas
    for single command*/
    void step_instruction(bool print_disas);

    /*prints the dissassembly of current instruction until the end of function*/
    void disasembly();
    /*disables all set breakpoints*/
    void disable_all();
    
    /*enables all set breakpoints*/
    void enable_all();
};

//@ assisting function signatures
// function that begins the debugging process
void begin_debuging(debugger *mdb, char *argv[]);
/*prints the usage of mdb tool*/
void Usage();

/*prints input character times times*/
void print_char(int times, const char ch);
