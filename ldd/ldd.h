#pragma once

#include <unordered_map>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <ostream>

namespace fs = std::filesystem;

class LDD {
public:
    explicit LDD(const fs::path &binary_path);

    void execute();
    void report(std::ostream &out);

private:
    fs::path find_lib_in_path(const std::string &name, const fs::path &p);

    fs::path find_lib_in_ld_library_path(const std::string &name);

    fs::path find_lib(const std::string &name);

    void operate_binary(const fs::path &binary);

    std::string get_ld_library_path();

    std::unordered_map<std::string, fs::path> libs;
    std::array<std::string, 2> standart_paths = {"/lib", "/usr/lib"};

    fs::path binary_path;
};