// ziptool.cpp: CLI hỗ trợ liệt kê các file trong file zip, giải nén 1 file zip ra stdout, hoặc tấn công từ điển vào 1 file trong file zip

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
#include"ZipFile.h"

void print_help() {
    //pass
    fprintf(stdout, "[program] [ARGS]\n");
    fprintf(stdout, "[ARGS]: \n");
    fprintf(stdout, "  -h [help]: print this help\n");
    fprintf(stdout, "  -m [mode name]: one of \"[b]ruteforce\", \"[l]ist\", \"[e]xtract\"\n" );
    fprintf(stdout, "  -i [index]: the index of the file to extract/bruteforce, refer to \"list\" mode to find which index is needed\n");
    fprintf(stdout, "  -p [path to password] file: specify the file containing password use to extract file, only used in \"extract\" mode\n");
    fprintf(stdout, "  -o [output] path file: specify where to write the extracted data to, only used in \"extract\" mode, default to stdout if not specified\n"); /* windows \r\n shenaningans*/
    fprintf(stdout, "  -j [number]: number of thread to use, only used in password \"bruteforce\" mode\n");
    fprintf(stdout, "  -d [DICTIONARY]: dictionary, only used in password \"bruteforce\" mode\n");
    fprintf(stdout, "  -l [logging frequency]: log progress after [frequency] password, set to 0 to disable, only used in password \"bruteforce\" mode\n");
    fprintf(stdout, "  -f [file_path]: path to the zip file\n");
    fprintf(stdout, "[DICTIONARY]: \n");
    fprintf(stdout, "  dictionary is a collection of file, each may be used as binary mode or text mode\n");
    fprintf(stdout, "  specify pattern for dictionary, use {[mode]:[file_path]}\n");
    fprintf(stdout, "  [mode] may be 't' or 'b'\n");
    fprintf(stdout, "  [file_path] is the path to the file, use \\ to escape character (e.g \\\\, \\{, \\}, \\:, \\[, \\])\n");
    fprintf(stdout, "  only support ascii file path, unicode path may get the program to terminate\n");
}

// danh sách các flag hỗ trợ
int flag_help; // -h

int flag_mode_set;
int flag_mode; // -m

int flag_index_set;
int flag_index; // -i

int flag_password_set;
std::string flag_password;

int flag_output_set;
std::string flag_output;


int flag_job_set;
int flag_job; // -j

int const DICT_TYPE_BINARY = 0;
int const DICT_TYPE_TEXT = 1;
int flag_dictionary_set;
std::vector<std::pair<std::string, int>> flag_dictionary; // -d

int flag_logging_frequency_set;
int flag_logging_frequency;

int flag_file_set;
std::string flag_file; // -f

int parse_nonnegative_int(char const * const s) {
    int slen = strlen(s);
    int d = 0;
    for (int i = 0; i < slen; ++i) {
        if ('0' <= s[i] && s[i] <= '9') {
            d *= 10;
            d = d + s[i] - '0';
            if (d < 0) { // overflow
                return -1;
            }
        }
        else { // invalid number string
            return -2;
        }
    }
    return d;
}
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
    std::string curs, curs2;
    std::stringstream ss(data);
    while (std::getline(ss, curs)) {
        if (curs.size() == 0) continue;
        curs2 = "";
        for (char c: curs) {
            if (c == '\r') { // Windows 
                continue;
            }
            curs2.push_back(c);
        }
        vT.emplace_back(curs2);
    }
    return vT;
}

// Đọc dữ liệu, mỗi byte sẽ trở thành một string độ dài 1
std::vector<std::string> text_split_binary(std::vector<uint8_t> const& data) {
    std::vector<std::string> vT;
    for (uint8_t d: data) { //chú ý \r\n trong windows
        vT.emplace_back(std::string(1, (char)d));
    }
    return vT;
}

void check_arg_count(int cur_index, int argc, int arg_count, char const * const flag_name) {
    if (cur_index + arg_count >= argc) {
        fprintf(stderr, "Cannot parse argument for %s flag, aborting\n", flag_name);
        exit(EXIT_FAILURE);
    }
}


int main(int argc, char const * const argv[]) {
    if (argc <= 1) {
        print_help();
        return EXIT_SUCCESS;
    }
    // parsing args
    for (int i = 1; i < argc; ) {
        int slen = strlen(argv[i]);
        if (slen < 1) {
            fprintf(stderr, "Zero-length arg, aborting\n");
            exit(EXIT_FAILURE);
        }

        // this is a flag
        if (argv[i][0] == '-') {
            if (slen > 2) {
                fprintf(stderr, "Unrecognize flag: %s, aborting\n", argv[i]);
                exit(EXIT_FAILURE);
            }
            if (argv[i][1] == 'h') {
                flag_help = 1;
                i += 1;
            }
            else if (argv[i][1] == 'j') {
                check_arg_count(i, argc, 1, argv[i]);
                if (flag_job_set) {
                    fprintf(stderr, "Duplicate flag: %s, aborting\n", argv[i]);
                    exit(EXIT_FAILURE);
                }

                int c = parse_nonnegative_int(argv[i+1]);
                if (c == -1) {
                    fprintf(stderr, "possible overflow in -j flag, aborting\n");
                    exit(EXIT_FAILURE);
                }
                else if (c == -2) {
                    fprintf(stderr, "invalid number string for -j flag, aborting\n");
                    exit(EXIT_FAILURE);
                }
                else if (c == 0) {
                    fprintf(stderr, "job must be a positive number, got -j %s, aborting\n", argv[i+1]);
                    exit(EXIT_FAILURE);
                }
                flag_job_set = 1;
                flag_job = c;
                i += 2;
            }
            else if (argv[i][1] == 'm') {
                check_arg_count(i, argc, 1, argv[i]);

                if (flag_mode_set) {
                    fprintf(stderr, "Duplicate flag: %s, aborting\n", argv[i]);
                    exit(EXIT_FAILURE);
                }

                if (strcmp(argv[i+1], "bruteforce") == 0) {
                    flag_mode = 'b';
                }
                else if (strcmp(argv[i+1], "b") == 0) {
                    flag_mode = 'b';
                }
                else if (strcmp(argv[i+1], "list") == 0) {
                    flag_mode = 'l';
                }
                else if (strcmp(argv[i+1], "l") == 0) {
                    flag_mode = 'l';
                }
                else if (strcmp(argv[i+1], "extract") == 0) {
                    flag_mode = 'e';
                }
                else if (strcmp(argv[i+1], "e") == 0) {
                    flag_mode = 'e';
                }
                else {
                    fprintf(stderr, "Unrecognize mode: %s, aborting\n", argv[i+1]);
                    exit(EXIT_FAILURE);
                }
                flag_mode_set = 1;
                i += 2;
            }
            else if (argv[i][1] == 'i') {
                check_arg_count(i, argc, 1, argv[i]);
                int c = parse_nonnegative_int(argv[i+1]);

                if (flag_index_set) {
                    fprintf(stderr, "Duplicate flag: %s, aborting\n", argv[i]);
                    exit(EXIT_FAILURE);
                }

                if (c == -1) {
                    fprintf(stderr, "possible overflow in -i flag, aborting\n");
                    exit(EXIT_FAILURE);
                }
                else if (c == -2) {
                    fprintf(stderr, "invalid number string for -i flag, aborting\n");
                    exit(EXIT_FAILURE);
                }
                flag_index_set = 1;
                flag_index = c;

                i += 2;
            }
            else if (argv[i][1] == 'l') {
                check_arg_count(i, argc, 1, argv[i]);
                int c = parse_nonnegative_int(argv[i+1]);

                if (flag_logging_frequency_set) {
                    fprintf(stderr, "Duplicate flag: %s, aborting\n", argv[i]);
                    exit(EXIT_FAILURE);
                }

                if (c == -1) {
                    fprintf(stderr, "possible overflow in -l flag, aborting\n");
                    exit(EXIT_FAILURE);
                }
                else if (c == -2) {
                    fprintf(stderr, "invalid number string for -l flag, aborting\n");
                    exit(EXIT_FAILURE);
                }
                flag_logging_frequency_set = 1;
                flag_logging_frequency = c;

                i += 2;
            }
            else if (argv[i][1] == 'f') {
                check_arg_count(i, argc, 1, argv[i]);
                if (flag_file_set) {
                    fprintf(stderr, "Duplicate flag: %s, aborting\n", argv[i]);
                    exit(EXIT_FAILURE);
                }
                flag_file_set = 1;
                flag_file = std::string(argv[i+1]);
                i += 2;
            }
            else if (argv[i][1] == 'p') {
                check_arg_count(i, argc, 1, argv[i]);
                if (flag_password_set) {
                    fprintf(stderr, "Duplicate flag: %s, aborting\n", argv[i]);
                    exit(EXIT_FAILURE);
                }
                flag_password_set = 1;
                flag_password = std::string(argv[i+1]);
                i += 2;
            }
            else if (argv[i][1] == 'o') {
                check_arg_count(i, argc, 1, argv[i]);
                if (flag_output_set) {
                    fprintf(stderr, "Duplicate flag: %s, aborting\n", argv[i]);
                    exit(EXIT_FAILURE);
                }
                flag_output_set = 1;
                flag_output = std::string(argv[i+1]); /* hopyfully have write permission, and not write to any critical resources */
                i += 2;
            }
            else if (argv[i][1] == 'd') {
                check_arg_count(i, argc, 1, argv[i]);
                if (flag_dictionary_set) {
                    fprintf(stderr, "Duplicate flag: %s, aborting\n", argv[i]);
                    exit(EXIT_FAILURE);
                }
                flag_dictionary_set = 1;
                flag_dictionary = parse_dictionary(argv[i+1]);
                i += 2;
            }
            else {
                fprintf(stderr, "Unrecognize flag: %s, aborting\n", argv[i]);
                exit(EXIT_FAILURE);
            }
        }
        else {
            fprintf(stderr, "Not a flag, aborting\n");
            exit(EXIT_FAILURE);
        }
    }

    if (flag_help) {
        print_help();
        return EXIT_SUCCESS;
    }

    if (!flag_file_set) {
        fprintf(stderr, "No file to operate on, aborting\n");
        exit(EXIT_FAILURE);
    }
    if (!flag_mode_set) {
        fprintf(stderr, "No file mode set, defaulting to \"list\" mode\n");
        flag_mode_set = 1;
        flag_mode = 'l';
    }

    try {
        ZipFile zf = ZipFile(flag_file);
        if (flag_mode == 'l') {
            std::vector<std::string> file_name_list = zf.GetFileList();
            for (int i = 0; i < (int)file_name_list.size(); ++i) {
                fprintf(stdout, "%d - %s\n", i, file_name_list[i].c_str());
            }
        }
        else if (flag_mode == 'e') {
            if (!flag_index_set) {
                fprintf(stderr, "No index found, consider using \"list\" mode to get file index, aborting\n");
                exit(EXIT_FAILURE);
            }
            std::vector<uint8_t> file_data;
            if (flag_password_set) {
                // read the password from text file
                std::string pwd = read_file_text(flag_password);
                file_data = zf.ExtractDataWithPassword(flag_index, pwd);
            }
            else {
                file_data = zf.ExtractData(flag_index);
            }
            
            if (flag_output_set) {
                std::ofstream file_out_stream(flag_output, std::ios::binary);
                if (file_out_stream.fail()) {
                    fprintf(stderr, "cannot open %s to write, aborting...\n", flag_output.c_str());
                    return EXIT_FAILURE;
                }
                file_out_stream.write((char*)file_data.data(), file_data.size());
                file_out_stream.close();
            }
            else {    
                for (uint8_t c: file_data) {
                    fprintf(stdout, "%c", c);
                }
            }
            return EXIT_SUCCESS;
        }
        else if (flag_mode == 'b') {
            if (!flag_dictionary_set) {
                fprintf(stderr, "No dictionary provided to bruteforce, aborting\n");
                exit(EXIT_FAILURE);
            }
            if (!flag_job_set) {
                fprintf(stderr, "Not specified how many threads to use, default to 1\n");
                flag_job = 1;
            }

            std::vector<std::vector<std::string>> Dict;
            for (int i = 0; i < (int)flag_dictionary.size(); ++i) {
                if (flag_dictionary[i].second == DICT_TYPE_TEXT) {
                    std::string text_data = read_file_text(flag_dictionary[i].first);
                    Dict.emplace_back(text_split_text(text_data));
                }
                else if (flag_dictionary[i].second == DICT_TYPE_BINARY) {
                    std::vector<uint8_t> binary_data = read_file_binary(flag_dictionary[i].first);
                    Dict.emplace_back(text_split_binary(binary_data));
                }
            }

            int logging_frequency = 0;
            if (flag_logging_frequency_set) {
                logging_frequency = flag_logging_frequency;
            }

            zf.BruteForceFile(flag_index_set ? flag_index: 0, Dict, flag_job, logging_frequency);
        }
    }
    catch (std::exception& e) {
        fprintf(stderr, "got error: %s, aborting\n", e.what());
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}