#include "ldd/ldd.h"


int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Not enough arguments" << std::endl;
        return 1;
    }
    fs::path binary(argv[1]);
    LDD ldd(binary);
    ldd.execute();
    ldd.report(std::cout);
}

