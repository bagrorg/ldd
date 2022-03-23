#include <iostream>
#include "elf.h"
#include <iomanip>

int verificate_header(const Elf64_Ehdr &header) {
    std::array<const unsigned char, 4> expected_magic = {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};

    if (std::memcmp(header.e_ident, expected_magic.data(), sizeof(expected_magic)) != 0) {
        //std::cerr << "Target is not an ELF executable\n";
        return 1;
    }

    if (header.e_ident[EI_CLASS] != ELFCLASS64) {
        //std::cerr << "Sorry, only ELF-64 is supported.\n";
        return 1;
    }

    if (header.e_machine != EM_X86_64) {
        //std::cerr << "Sorry, only x86-64 is supported.\n";
        return 1;
    }

    return 0;
}


Elf64_Ehdr get_header(std::ifstream &elf) {
    Elf64_Ehdr elf_hdr;
    elf.read(reinterpret_cast<char *>(&elf_hdr), sizeof(elf_hdr));
    return elf_hdr;
}


size_t get_strtab_offset(std::ifstream &file, size_t dynsect_size, size_t dynsect_offset) {
    file.seekg(dynsect_offset);
    size_t off;
    for (size_t j = 0; j * sizeof(Elf64_Dyn) < dynsect_size; j++) {
        Elf64_Dyn dyn;
        file.read(reinterpret_cast<char *>(&dyn), sizeof(Elf64_Dyn));
        if (dyn.d_tag == DT_STRTAB) {
            off = dyn.d_un.d_val;
        }
    }

    return off;
}

std::vector<std::string> get_needed_names(std::ifstream &file, size_t dynsect_size, size_t dynsect_offset, size_t strtab_offset) {

    std::vector<std::string> deps;
    for (size_t j = 0; j * sizeof(Elf64_Dyn) < dynsect_size; j++) {
        Elf64_Dyn dyn;
        size_t offset = dynsect_offset + j * sizeof(Elf64_Dyn);
        file.seekg(offset);
        file.read(reinterpret_cast<char *>(&dyn), sizeof(Elf64_Dyn));
        if (dyn.d_tag == DT_NEEDED) {
            std::string dep;
            file.seekg(strtab_offset + dyn.d_un.d_val);
            std::getline(file, dep, '\0');
            deps.push_back(dep);
        }
    }
    return deps;
}

std::vector<std::string> read_dynamic_section(const Elf64_Ehdr &header, Elf64_Shdr &section_header, std::ifstream &file) {
    file.seekg(header.e_shoff);
    std::vector<std::string> dynamic_libs;
    for (int i = 0; i < header.e_shnum; ++i) {
        file.read(reinterpret_cast<char *>(&section_header), sizeof(section_header));
        if (section_header.sh_type == SHT_DYNAMIC) {
            size_t strtab_offset = get_strtab_offset(file, section_header.sh_size, section_header.sh_offset);
            dynamic_libs = get_needed_names(file, section_header.sh_size, section_header.sh_offset, strtab_offset);
        }
    }
    return dynamic_libs;
}



std::vector<std::string> get_dynamic_libs(const fs::path &p) {
    std::vector<std::string> dynamic_libs;
    std::ifstream file(p, std::ios::binary | std::ios::in);
    if (!file.is_open()) {
        std::cerr << "File was not open" << std::endl;
        return {};
    }

    Elf64_Ehdr header = get_header(file);
    Elf64_Shdr section_header;

    return read_dynamic_section(header, section_header, file);
}



bool is_supportable(const fs::path &p) {
    std::vector<std::string> dynamic_libs;
    std::ifstream file(p, std::ios::binary | std::ios::in);
    if (!file.is_open()) {
        std::cerr << "File was not open" << std::endl;
        return false;
    }

    Elf64_Ehdr hdr = get_header(file);
    return !verificate_header(hdr);
}
