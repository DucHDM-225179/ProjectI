#include<iostream>
#include<fstream>
#include<sstream>
#include<exception>
#include<string>
#include<cstring>
#include<iterator>
#include<vector>
#include<cstdint>
#include<cstdio>
#include<chrono>
#include<thread>
#include"ZipFile.h"

void print_help() {
    //pass
    fprintf(stdout, "[program] [DICTIONARY] [PATH_TO_ZIP_FILE]\n");
    fprintf(stdout, "[DICTIONARY]: \n");
    fprintf(stdout, "  dictionary is a collection of file, each may be used as binary mode or text mode\n");
    fprintf(stdout, "  specify pattern for dictionary, use {[mode]:[file_path]}\n");
    fprintf(stdout, "  [mode] may be 't' or 'b'\n");
    fprintf(stdout, "  [file_path] is the path to the file, use \\ to escape character (e.g \\\\, \\{, \\}, \\:, \\[, \\])\n");
    fprintf(stdout, "  only support ascii file path, unicode path may get the program to terminate\n");
}

int const DICT_TYPE_BINARY = 0;
int const DICT_TYPE_TEXT = 1;
std::vector<std::pair<std::string, int>> Dictionary; // -d

std::vector<std::pair<std::string, int>> parse_dictionary(char const * const s) {
    static int const STATE_BEGIN = 0;
    static int const STATE_IN_MODE = 1;
    static int const STATE_END_MODE = 2;
    static int const STATE_IN_FILEPATH = 3;
    int state = STATE_BEGIN;

    std::vector<std::pair<std::string,int>> dict;

    std::string curs = "";
    int curm = -1;

    int slen = strlen(s);
    for (int i = 0; i < slen; ++i) {
        char c = s[i];
        if (state == STATE_BEGIN) {
            if (c != '{') {
                fprintf(stderr, "dictionary parse error: expecting '{', got '%c', aborting\n", c);
                exit(EXIT_FAILURE);
            }
            state = STATE_IN_MODE;
        }
        else if (state == STATE_IN_MODE) {
            if (c == 't') {
                curm = DICT_TYPE_TEXT;
            }
            else if (c == 'b') {
                curm = DICT_TYPE_BINARY;
            }
            else {
                fprintf(stderr, "dictionary parse error: unrecognize mode, got '%c', aborting\n", c);
                exit(EXIT_FAILURE);
            }
            state = STATE_END_MODE;
        }
        else if (state == STATE_END_MODE) {
            if (c == ':') {
                state = STATE_IN_FILEPATH;
            }
            else {
                fprintf(stderr, "dictionary parse error: expecting ':', got '%c', aborting\n", c);
                exit(EXIT_FAILURE);
            }
        }
        else if (state == STATE_IN_FILEPATH) {
            // got \, escape character
            if (c == '\\') {
                if (i + 1 == slen) {
                    fprintf(stderr, "dictionary parse error: unexpected EOF, aborting\n");
                    EXIT_FAILURE;
                }
                char nc = s[i+1];
                curs.push_back(nc);
                i += 1; // đã lấy chữ tiếp theo, có thể bỏ qua
            }
            else if (c == '}') {
                state = STATE_BEGIN;
                dict.emplace_back(std::make_pair(curs, curm));
                curs.clear();
                curm = -1;
            }
            else {
                curs.push_back(c);
            }
        }
    }
    return dict;
}

// Đọc một file, trả về dữ liệu nhị phân
std::vector<uint8_t> read_file_binary(std::string const& file_path) {
    std::vector<uint8_t> rawData;
    try {
        std::ifstream fn(file_path, std::ios::binary);
        if (fn.fail()) {
            throw std::invalid_argument("cannot open file\n");
        }
        fn.unsetf(std::ios::skipws);
        std::streampos fsize;
        fn.seekg(0, std::ios::end);
        fsize = fn.tellg();
        fn.seekg(0, std::ios::beg);

        rawData = std::vector<uint8_t>();
        rawData.reserve(fsize);
        rawData.insert(rawData.begin(), 
            std::istream_iterator<uint8_t>(fn), 
            std::istream_iterator<uint8_t>());
    } catch (std::exception &e) {
        throw;
    }
    return rawData;
}

// Đọc một file, trả về dữ liệu chữ cái, chỉ hỗ trợ bảng mã ascii
std::string read_file_text(std::string const& file_path) {
    std::string textData;
    try {
        std::ifstream fn(file_path);
        if (fn.fail()) {
            throw std::invalid_argument("cannot open file\n");
        }
        fn.unsetf(std::ios::skipws);
        std::streampos fsize;
        fn.seekg(0, std::ios::end);
        fsize = fn.tellg();
        fn.seekg(0, std::ios::beg);
        textData.reserve(fsize);
        
        textData.insert(textData.begin(), 
            std::istream_iterator<char>(fn), 
            std::istream_iterator<char>());
    } catch (std::exception &e) {
        throw;
    }
    return textData;
}

// Đọc dữ liệu, trả về một list các string từ các dòng, bỏ qua các dòng rỗng
std::vector<std::string> text_split_text(std::string const& data) {
    std::vector<std::string> vT;
    std::string curs;
    std::stringstream ss(data);
    while (std::getline(ss, curs)) {
        if (curs.size() == 0) continue;
        vT.emplace_back(curs);
    }
    return vT;
}

// Đọc dữ liệu, mỗi byte sẽ trở thành một string độ dài 1
std::vector<std::string> text_split_binary(std::vector<uint8_t> const& data) {
    std::vector<std::string> vT;
    for (uint8_t d: data) {
        vT.emplace_back(std::string(1, (char)d));
    }
    return vT;
}

int main(int argc, char const * const argv[]) {
    if (argc != 3) {
        print_help();
        return EXIT_SUCCESS;
    }
    try {
        Dictionary = parse_dictionary(argv[1]);
        std::vector<std::vector<std::string>> Dict;
        for (int i = 0; i < (int)Dictionary.size(); ++i) {
            if (Dictionary[i].second == DICT_TYPE_TEXT) {
                std::string text_data = read_file_text(Dictionary[i].first);
                Dict.emplace_back(text_split_text(text_data));
            }
            else if (Dictionary[i].second == DICT_TYPE_BINARY) {
                std::vector<uint8_t> binary_data = read_file_binary(Dictionary[i].first);
                Dict.emplace_back(text_split_binary(binary_data));
            }
        }

        ZipFile zf = ZipFile(argv[2]);
        int const max_job = std::thread::hardware_concurrency();

        // single_job
        std::cout << "Benchmark on single core" << std::endl;
        auto single_thread_start = std::chrono::steady_clock::now();
        zf.BruteForceFile(0, Dict, 1);
        auto single_thread_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - single_thread_start);
        std::cout << "Single core: " << single_thread_time.count() << " ms" << std::endl;

        // max_job
        std::cout << "Benchmark on all core" << std::endl;
        auto max_thread_start = std::chrono::steady_clock::now();
        zf.BruteForceFile(0, Dict, max_job);
        auto max_thread_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - max_thread_start);
        std::cout << "All " << max_job << " core(s): " << max_thread_time.count() << " ms" << std::endl;

        std::cout << "=====================" << std::endl;
        std::cout << "Single core: " << single_thread_time.count() << " ms" << std::endl;
        std::cout << max_job << " core(s): " << max_thread_time.count() << " ms" << std::endl;
        
    }
    catch( std::exception& e) {
        fprintf(stderr, "got error: %s, aborting\n", e.what());
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}