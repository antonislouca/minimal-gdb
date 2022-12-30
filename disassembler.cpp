#include "disassembler.hpp"

disassembler::disassembler(const char *filename)
{
    /*initialized engine*/
    if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK)
    {
        fprintf(stderr, "Could not initialize capstone. Exiting...\n");
        exit(-1);
    }
    /* AT&T */
    cs_option(handle, CS_OPT_SYNTAX, CS_OPT_SYNTAX_ATT);

    /*
   detail mode. detailed mode is used since we need to be able to get the target
   for each command. we need access to detail structure in instruction structure
   */
    cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
}

disassembler::~disassembler()
{
    cs_close(&handle);
}

void disassembler::print_ins(cs_insn *ins, uint64_t base_addr)
{
    base_addr += ins->address;
    fprintf(stderr, "0x%016lx:\t%s\t%s\n", base_addr, ins->mnemonic, ins->op_str);
}

void disassembler::disas_single_cmd(elf_file *elf, uint64_t bp_adddress, uint64_t base_addr)
{
    unsigned long orig_bp = bp_adddress;
    bp_adddress = bp_adddress - base_addr;
    uint64_t offset = bp_adddress - elf->text_start;
    const uint8_t *pc = (const unsigned char *)elf->text->d_buf;
    pc += offset;
    size_t n = elf->text_end - elf->text_start;
    int repetitions = 0;
    cs_insn *cs_ins = cs_malloc(handle);

    // fprintf(stderr, "Bp ADDR: %lx, base addr: %lx, Offset: %lx, text start: %lx
    // , text end: %lx\n",
    //         bp_adddress, base_addr, offset, elf->text_start, elf->text_end);

    fprintf(stderr, "Disassembling address: 0x%016lx\n", orig_bp);
    while (cs_disasm_iter(handle, &pc, &n, &bp_adddress, cs_ins))
    {
        if (cs_ins->id == X86_INS_INVALID || cs_ins->size == 0) // check if we have valid command
        {
            fprintf(stderr, "Invalic command\n");
            break;
        }
        this->print_ins(cs_ins, base_addr);
        repetitions++;
        /* We reached the end of the fuction. */
        if (cs_ins->id == X86_INS_RET) //|| (cs_ins->id == X86_INS_JMP && get_cs_ins_immediate_target(cs_ins) >) we want it to be out of bounds in terms of size
            break;
        if (repetitions == 1) // force it to execute only once
            break;
    }
    cs_free(cs_ins, 1);
}
void disassembler::disas_old_code(elf_file *elf, uint64_t bp_adddress, uint64_t base_addr)
{

    bp_adddress = bp_adddress - base_addr;
    uint64_t offset = bp_adddress - elf->text_start;
    const uint8_t *pc = (const unsigned char *)elf->text->d_buf;
    pc += offset;
    size_t n = elf->text_end - elf->text_start;
    int repetitions = 0;
    cs_insn *cs_ins = cs_malloc(handle);

    while (cs_disasm_iter(handle, &pc, &n, &bp_adddress, cs_ins))
    {
        if (cs_ins->id == X86_INS_INVALID || cs_ins->size == 0) // check if we have valid command
        {
            fprintf(stderr, "Invalic command\n");
            break;
        }
        base_addr += cs_ins->address;
        fprintf(stderr, "%s\t%s\n", cs_ins->mnemonic, cs_ins->op_str);
        repetitions++;
        /* We reached the end of the fuction. */
        if (cs_ins->id == X86_INS_RET) //|| (cs_ins->id == X86_INS_JMP && get_cs_ins_immediate_target(cs_ins) >) we want it to be out of bounds in terms of size
            break;
        if (repetitions == 1) // force it to execute only once
            break;
    }
    cs_free(cs_ins, 1);
}
void disassembler::disas_commands(elf_file *elf, uint64_t bp_adddress, uint64_t base_addr)
{
    unsigned long orig_bp = bp_adddress;
    bp_adddress = bp_adddress - base_addr;
    uint64_t offset = bp_adddress - elf->text_start;
    const uint8_t *pc = (const unsigned char *)elf->text->d_buf;
    pc += offset;
    size_t n = elf->text_end - elf->text_start;

    cs_insn *cs_ins = cs_malloc(handle);

    // fprintf(stderr, "Bp ADDR: %lx, base addr: %lx, Offset: %lx, text start: %lx
    // , text end: %lx\n",
    //         bp_adddress, base_addr, offset, elf->text_start, elf->text_end);

    fprintf(stderr, "Disassembling address: 0x%016lx\n", orig_bp);
    while (cs_disasm_iter(handle, &pc, &n, &bp_adddress, cs_ins))
    {
        if (cs_ins->id == X86_INS_INVALID || cs_ins->size == 0) // check if we have valid command
        {
            fprintf(stderr, "Invalic command\n");
            break;
        }
        this->print_ins(cs_ins, base_addr);
        /* We reached the end of the fuction. */
        if (cs_ins->id == X86_INS_RET) //|| (cs_ins->id == X86_INS_JMP && get_cs_ins_immediate_target(cs_ins) >) we want it to be out of bounds in terms of size
            break;
    }
    cs_free(cs_ins, 1);
}