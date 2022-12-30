#pragma once
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "capstone/capstone.h"
#include "elf_loader.hpp"
#include <iostream>
#include <string>
#include <map>
#include <list>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>

//@classes
class disassembler
{
private:
    csh handle;

public:
    disassembler(const char *filename);
    ~disassembler();
    /*prints single instruction*/
    void print_ins(cs_insn *ins, uint64_t base_addr);
    /*disassembly of multiple commands for the command to be executed until end of function*/
    void disas_commands(elf_file *elf, uint64_t bp_adddress, uint64_t base_addr);
    /*dissasembly of a single command*/
    void disas_single_cmd(elf_file *elf, uint64_t bp_adddress, uint64_t base_addr);
    /*dissasembly of a single command, but used only for old code in breakpoints*/
    void disas_old_code(elf_file *elf, uint64_t bp_adddress, uint64_t base_addr);
};

//@function prototypes
