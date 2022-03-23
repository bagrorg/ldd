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

std::vector<std::string> get_needed_names(std::ifstream &file, size_t dynsect_size, size_t dynsect_offset, size_t strtab_offset, size_t LOAD) {

    std::vector<std::string> deps;
    for (size_t j = 0; j * sizeof(Elf64_Dyn) < dynsect_size; j++) {
        Elf64_Dyn dyn;
        size_t offset = dynsect_offset + j * sizeof(Elf64_Dyn);
        file.seekg(offset);
        file.read(reinterpret_cast<char *>(&dyn), sizeof(Elf64_Dyn));
        if (dyn.d_tag == DT_NEEDED) {
            std::string dep;
            file.seekg(strtab_offset + dyn.d_un.d_val - LOAD);
            std::getline(file, dep, '\0');
            deps.push_back(dep);
        }
    }
    return deps;
}

std::vector<std::string> read_dynamic_section(const Elf64_Ehdr &header, size_t LOAD, std::ifstream &file) {
    Elf64_Shdr section_header;
    file.seekg(header.e_shoff);
    std::vector<std::string> dynamic_libs;
    for (int i = 0; i < header.e_shnum; ++i) {
        file.read(reinterpret_cast<char *>(&section_header), sizeof(section_header));
        if (section_header.sh_type == SHT_DYNAMIC) {
            size_t strtab_offset = get_strtab_offset(file, section_header.sh_size, section_header.sh_offset);
            dynamic_libs = get_needed_names(file, section_header.sh_size, section_header.sh_offset, strtab_offset, LOAD);
        }
    }
    return dynamic_libs;
}

size_t read_LOAD(const Elf64_Ehdr &header, std::ifstream &file) {
    Elf64_Phdr program_header;
    file.seekg(header.e_phoff);
    std::vector<std::string> dynamic_libs;
    for (int i = 0; i < header.e_phnum; ++i) {
        file.read(reinterpret_cast<char *>(&program_header), sizeof(program_header));
        if (program_header.p_type == PT_LOAD) {
            return program_header.p_paddr;
        }
    }
    return 0;
}


std::vector<std::string> get_dynamic_libs(const fs::path &p) {
    std::vector<std::string> dynamic_libs;
    std::ifstream file(p, std::ios::binary | std::ios::in);
    if (!file.is_open()) {
        std::cerr << "File was not open" << std::endl;
        return {};
    }

    Elf64_Ehdr header = get_header(file);

    size_t LOAD = read_LOAD(header, file);

    return read_dynamic_section(header, LOAD, file);
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
