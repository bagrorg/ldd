#include <iostream>
#include "elf.h"

std::vector<char> read_elf(const fs::path &filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::in);
    if (!file.is_open()) {
        std::cerr << "File was not open" << std::endl;
        return {};
    }
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>{} };
}

template <typename Header>          //TODO header?
int verificate_header(const Header &header) {
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

size_t get_elf_class(const std::vector<char> &elf) {
    Elf32_Ehdr elf_hdr;
    memcpy(&elf_hdr, elf.data(), sizeof(elf_hdr));

    return elf_hdr.e_ident[EI_CLASS];
}

Elf32_Ehdr get_header_32(const std::vector<char> &elf) {
    Elf32_Ehdr elf_hdr;
    memcpy(&elf_hdr, elf.data(), sizeof(elf_hdr));
    return elf_hdr;
}

Elf64_Ehdr get_header_64(const std::vector<char> &elf) {
    Elf64_Ehdr elf_hdr;
    memcpy(&elf_hdr, elf.data(), sizeof(elf_hdr));
    return elf_hdr;
}


size_t get_strtab_offset(const std::vector<char> &elf_data, size_t dynsect_size, size_t dynsect_offset) {
    for (size_t j = 0; j * sizeof(Elf64_Sym) < dynsect_size; j++) {
        Elf64_Dyn dyn;
        size_t offset = dynsect_offset + j * sizeof(Elf64_Dyn);
        memcpy(&dyn, elf_data.data() + offset, sizeof(dyn));
        if (dyn.d_tag == DT_STRTAB) {
            return dyn.d_un.d_val;                      // d_ptr???
        }
    }
    return 0;
}

std::vector<std::string> get_needed_names(const std::vector<char> &elf_data, size_t dynsect_size, size_t dynsect_offset, size_t strtab_offset) {
    std::vector<std::string> deps;
    for (size_t j = 0; j * sizeof(Elf64_Sym) < dynsect_size; j++) {
        Elf64_Dyn dyn;
        size_t offset = dynsect_offset + j * sizeof(Elf64_Dyn);
        memcpy(&dyn, elf_data.data() + offset, sizeof(dyn));
        if (dyn.d_tag == DT_NEEDED) {
            std::string dep = elf_data.data() + strtab_offset + dyn.d_un.d_val;
            deps.push_back(dep);
        }
    }
    return deps;
}

template <typename Header, typename Section>
std::vector<std::string> read_dynamic_section(const Header &header, Section &section_header, const std::vector<char> &file_data) {
    std::vector<std::string> dynamic_libs;
    for (int i = 0; i < header.e_shnum; ++i) {
        size_t offset = header.e_shoff + i * header.e_shentsize;
        memcpy(&section_header, file_data.data() + offset, sizeof(section_header));
        if (section_header.sh_type == SHT_DYNAMIC) {
            size_t strtab_offset = get_strtab_offset(file_data, section_header.sh_size, section_header.sh_offset);
            dynamic_libs = get_needed_names(file_data, section_header.sh_size, section_header.sh_offset, strtab_offset);
            return dynamic_libs;
        }
    }
}

std::vector<std::string> get_dynamic_libs_32(const std::vector<char> &file_data) {
    Elf32_Ehdr header = get_header_32(file_data);
    Elf32_Shdr section_header;

    return read_dynamic_section(header, section_header, file_data);
}

std::vector<std::string> get_dynamic_libs_64(const std::vector<char> &file_data) {
    Elf64_Ehdr header = get_header_64(file_data);
    Elf64_Shdr section_header;

    return read_dynamic_section(header, section_header, file_data);
}


std::vector<std::string> get_dynamic_libs(const fs::path &filename) {
    std::vector<std::string> dynamic_libs;
    std::vector<char> file_data = read_elf(filename);
    if (file_data.empty()) {                                                    // TODO: better operate
        return {};
    }

    size_t elf_class = get_elf_class(file_data);

    switch (elf_class) {
        case ELFCLASS32:
            return get_dynamic_libs_32(file_data);
            break;
        case ELFCLASS64:
            return get_dynamic_libs_64(file_data);
            break;
        default:
            break;
    }

    return {};
}



bool is_supportable(const fs::path &p) {
    std::vector<std::string> dynamic_libs;
    std::vector<char> file_data = read_elf(p);
    if (file_data.empty()) {                                                    // TODO: better operate
        return {};
    }

    size_t elf_class = get_elf_class(file_data);

    switch (elf_class) {
        case ELFCLASS32: {
            Elf32_Ehdr hdr32 = get_header_32(file_data);
            if (verificate_header(hdr32)) {
                return false;
            } else {
                return true;
            }
        }

        case ELFCLASS64: {
            Elf64_Ehdr hdr64 = get_header_64(file_data);
            if (verificate_header(hdr64)) {
                return false;
            } else {
                return true;
            }
        }
        default:
            break;
    }

    return {};
}