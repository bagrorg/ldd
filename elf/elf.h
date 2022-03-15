#pragma once

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <vector>
#include <string>
#include <elf.h>
#include <cstring>
#include <fstream>
#include <array>

#include <filesystem>

namespace fs = std::filesystem;

std::vector<std::string> get_dynamic_libs(const fs::path &filename);