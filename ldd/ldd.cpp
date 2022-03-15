#include <vector>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include "ldd.h"
#include "../elf/elf.h"                                 //TODO

LDD::LDD(const fs::path &binary_path) : binary_path(binary_path) {}

void LDD::execute() {
    operate_binary(binary_path);
}

void LDD::report(std::ostream &out) {
    for (const auto& lib: libs) {
        if (lib.second.empty()) {
            out << lib.first << " => " << "not found" << std::endl;
        } else {
            out << lib.first << " => " << lib.second << std::endl;
        }
    }
}

fs::path LDD::find_lib_in_path(const std::string &name, const fs::path &p) {
    if (p == "") return {};                                                     //TODO: better

    for (const fs::directory_entry& dir: fs::recursive_directory_iterator(p)){
        fs::path pt(dir.path());
        if (pt.filename() == name) {
            return pt;
        }
    }

    return {};
}

fs::path LDD::find_lib_in_ld_library_path(const std::string &name) {
    std::string s = get_ld_library_path();
    if (s.empty()) {
        return {};
    }
    std::vector<std::string> paths;
    boost::split(paths, s, boost::is_any_of(":"));

    for (const auto& path: paths) {
        auto p = find_lib_in_path(name, fs::path(path));
        if (!p.empty()) {
            return p;
        }
    }
    return {};
}

fs::path LDD::find_lib(const std::string &name) {
    auto p = find_lib_in_ld_library_path(name);
    if (!p.empty()) {
        return p;
    }

    for (const std::string& path: standart_paths) {
        p = find_lib_in_path(name, fs::path(path));
        if (!p.empty()) {
            return p;
        }
    }

    return {};
}

void LDD::operate_binary(const fs::path &binary) {
    std::vector<std::string> binary_deps = get_dynamic_libs(binary);

    for (const std::string& dep: binary_deps) {
        if (libs.find(dep) != libs.end()) continue;

        fs::path path = find_lib(dep);
        libs[dep] = path;

        if (!path.empty()) {
            operate_binary(path);
        }
    }
}

std::string LDD::get_ld_library_path() {
    char *s = std::getenv("LD_LIBRARY_PATH");
    if (s == NULL) {
        return "";
    } else {
        return {s};
    }
}
