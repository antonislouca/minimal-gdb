#pragma once
#include <unistd.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <libelf.h>
#include <gelf.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <bits/stdc++.h>
#include <sys/personality.h>

/* Linux */
#include <syscall.h>
#include <sys/ptrace.h>
#define DEBUG 1
#define TOOL "mdb"

#define die_bp(...)                             \
    do                                          \
    {                                           \
        fprintf(stderr, TOOL ": " __VA_ARGS__); \
        fputc('\n', stderr);                    \
        exit(EXIT_FAILURE);                     \
    } while (0)

class breakpoint
{
private:
public:
    unsigned long address;
    long old_code;
    pid_t pid;
    bool is_enabled;

    //@functions
    breakpoint(unsigned long address, pid_t pid);

    /*function that will enable break points */
    bool enable_bp();

    /*
    function that will disable break points, restores original code by
    removing int3
    */
    void disable_bp();
};
