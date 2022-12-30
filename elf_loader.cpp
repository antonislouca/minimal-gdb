#include "mdb_headers.hpp"

void load_elf(const char *filename, elf_file *elf_obj)
{
    elf_obj->dynsym = NULL;
    elf_obj->symtab = NULL;
    elf_obj->is_stripped = 0;
    elf_obj->shstrtab_index = 0;
    // check version
    if (elf_version(EV_CURRENT) == EV_NONE)
        DIE_elf("(elf_version) %s", elf_errmsg(-1));

    int fd = open(filename, O_RDONLY);
    elf_obj->elf = elf_begin(fd, ELF_C_READ, NULL);
    if (elf_obj->elf == NULL)
        DIE_elf("(Not an elf file) %s", elf_errmsg(-1));

    Elf_Scn *scn = NULL; // section
    GElf_Shdr shdr;      // section header

    // get section strtab index
    if (elf_getshdrstrndx(elf_obj->elf, &elf_obj->shstrtab_index) != 0)
        DIE_elf("(elf_getshdrstrndx) %s", elf_errmsg(-1));
    while ((scn = elf_nextscn(elf_obj->elf, scn)) != NULL)
    {
        if (gelf_getshdr(scn, &shdr) != &shdr)
            DIE_elf("(gelf_getshdr) %s", elf_errmsg(-1));

        // get symtab section
        if (!strcmp(elf_strptr(elf_obj->elf, elf_obj->shstrtab_index, shdr.sh_name),
                    ".symtab"))
        {
            elf_obj->symtab = scn;
        }
        if (!strcmp(elf_strptr(elf_obj->elf, elf_obj->shstrtab_index, shdr.sh_name), ".text"))
        {
            elf_obj->text = elf_getdata(scn, elf_obj->text);
            if (!elf_obj->text)
                DIE_elf("(getdata) %s", elf_errmsg(-1));
            elf_obj->text_start = shdr.sh_addr;
            elf_obj->text_end = shdr.sh_size + elf_obj->text_start;
        }

        // get dynsym section
        if (!strcmp(elf_strptr(elf_obj->elf, elf_obj->shstrtab_index, shdr.sh_name),
                    ".dynsym"))
        {
            elf_obj->dynsym = scn;
        }
    }

    // set boolean if symtab does not exist
    if (elf_obj->symtab == NULL)
    {
        elf_obj->is_stripped = 1;
    }

    std::cerr
        << "Binary file locked and loaded" << std::endl;
}

void debug_symbols(elf_file *elf, std::map<std::string, long> *symbol_mappings)
{
    GElf_Shdr symtab_shdr;

    /* Get the descriptor.  */
    if (gelf_getshdr(elf->symtab, &symtab_shdr) != &symtab_shdr)
        DIE_elf("(error getting symbtab section header) %s", elf_errmsg(-1));

    // get symtab section data --> the indices to strtab
    Elf_Data *data = elf_getdata(elf->symtab, NULL);

    // find how many entries are stored in section using entry size
    int num_of_entries = symtab_shdr.sh_size / symtab_shdr.sh_entsize;

    for (int i = 0; i < num_of_entries; i++)
    {
        GElf_Sym sym;
        // get i-th symbol from data adn store it in sym
        gelf_getsym(data, i, &sym);
        // print symbol if is not a section or a file related symbol

        // MAP SYMBOLS THAT BELONG TO FUNCTIONS ONLY

        if (ELF64_ST_TYPE(sym.st_info) == STT_FUNC && sym.st_value >= elf->text_start && sym.st_value < elf->text_end)
        {
            symbol_mappings->insert(std::make_pair(
                elf_strptr(elf->elf, symtab_shdr.sh_link, sym.st_name), sym.st_value));
        }
    }
}
