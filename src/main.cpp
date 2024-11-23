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

int main(int argc, char const * const argv[]) {
    if (argc == 0) {
        print_help();
    }
    else if (argc == 2) {
        char const * const fn = argv[1];
        try {
            ZipFile zf(fn);
            std::vector<uint8_t> data = zf.ExtractData(0);
            for (uint8_t c: data) {
                fprintf(stdout, "%c", c);
            }
            fprintf(stdout, "\n");
            
        }
        catch (std::exception const& e) {
            fprintf(stderr, "%s\n", e.what());
            fprintf(stderr, "ERROR reading file\n");
        }
    }
}