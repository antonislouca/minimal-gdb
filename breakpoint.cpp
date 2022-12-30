#include "breakpoint.hpp"

breakpoint::breakpoint(unsigned long address, pid_t pid)
{
    this->address = address;

    this->pid = pid;
    this->is_enabled = false;
    // printf("bp constructor\n");

    // this->enable_bp();
}

bool breakpoint::enable_bp()
{
    // fprintf(stderr, "Address: %lx\n", address);
    this->old_code = ptrace(PTRACE_PEEKDATA, this->pid, (void *)this->address, NULL);
    if (this->old_code == -1)
    {
        //   fprintf(stderr, "Invalid address\n");
        return false;
        // die_bp("(peekdata) %s", strerror(errno));

    } // address given does not exist return

    // fprintf(stderr, "ENABLING BP ==> %p: 0x%lx pid: %d\n",
    //         (void *)this->address, this->old_code, this->pid);

    /* Insert the breakpoint. modify only one byte with int3 not all */
    long trap = (this->old_code & 0xFFFFFFFFFFFFFF00) | 0xCC;
    if (ptrace(PTRACE_POKEDATA, this->pid, (void *)this->address, (void *)trap) == -1)
        die_bp("(pokedata) %s", strerror(errno));
    this->is_enabled = true;
    return true;
}
void breakpoint::disable_bp()
{
    // if (DEBUG)
    //     fprintf(stderr, "Old code: %lx\n", this->old_code);
    // fprintf(stderr, "DISABLING BP ==> %p: 0x%lx pid: %d\n",
    //         (void *)this->address, this->old_code, this->pid);

    // reset code to regular state
    if (ptrace(PTRACE_POKEDATA, this->pid, (void *)this->address, (void *)this->old_code) == -1)
        die_bp("(pokedata) %s", strerror(errno));
    this->is_enabled = false;
}