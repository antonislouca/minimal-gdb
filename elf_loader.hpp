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

/* Linux */
#include <syscall.h>
#include <sys/ptrace.h>

/*C++ headers*/
#include <iostream>
#include <string>
#include <map>

//@macros
#define DIE_elf(...)                  \
    do                                \
    {                                 \
        fprintf(stderr, __VA_ARGS__); \
        fputc('\n', stderr);          \
        exit(EXIT_FAILURE);           \
    } while (0)

//@ function prototypes

typedef struct struct_elf
{
    Elf *elf;              // the elf given
    Elf_Scn *symtab;       // used for  symbol table
    Elf_Scn *dynsym;       // used for dynamic symbol table
    size_t shstrtab_index; // used for section string table
    bool is_stripped;
    Elf_Data *text;
    long unsigned text_start = 0;
    long unsigned text_end = 0;
} elf_file;
/*
function that loads the elf in the elf_file struct using libelf
*/
void load_elf(const char *filename, elf_file *elf);
void debug_symbols(elf_file *elf, std::map<std::string, long> *symbol_mappings);
