#include "mdb_headers.hpp"
/*
Main function of the program
known bugs:
!1.  when using the si, can cause crashes on call instructions
!2. disas can produce wrong disassembly on ret instructions

*/
int main(int argc, char *argv[])
{
    // execution paradigm:
    // cat /proc/PID/maps
    // ./mdb.out /mnt/c/Users/santoryu/Desktop/Epl451hw2/src/hw2/src/tail-call.out
    if (argc <= 1)
        die("min_strace <program>: %d", argc);

    std::cerr << "Filename:" << argv[1] << std::endl;
    debugger *mdb = new debugger(argv);
    // the struct that contains the elf file representation

    begin_debuging(mdb, argv);

    //! free section
    delete (mdb);
}
