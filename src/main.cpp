#include<iostream>
#include<exception>
#include<string>
#include<vector>
#include<cstdint>
#include<cstdio>
#include"ZipFile.h"

void print_help() {
    //pass
}

std::vector<std::string> const d1 = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z"};
std::vector<std::string> const d2 = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};

std::vector<std::vector<std::string>> Dict = {d1, d1, d1, d1, d1, d1};

int main(int argc, char const * const argv[]) {
    if (argc == 0) {
        print_help();
    }
    else if (argc == 2) {
        char const * const fn = argv[1];
        try {
            ZipFile zf(fn);
            zf.BruteForceFile(0, Dict, 8);
            
        }
        catch (std::exception const& e) {
            fprintf(stderr, "%s\n", e.what());
            fprintf(stderr, "ERROR reading file\n");
        }
    }
}