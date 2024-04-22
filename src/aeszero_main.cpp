#include "aes0.h"

#include <fstream>
#include <iostream>
#include <string>

struct ArgsType {
    char mode;
    std::string file_in_name;
    std::string file_out_name;
    std::string key16;
};

std::string usage_str() {
    return {"Usage: aeszero -e|-d <infile> -k <key16> | -w <key64file> -o "
            "<outfile>\n"};
}

void read_str_from_file(const std::string &file_name, std::string &str) {
    std::ifstream file(file_name, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << file_name << std::endl;
        exit(1);
    }
    file >> str;
    file.close();
}

void write_str_to_file(const std::string &file_name, const std::string &str) {
    std::ofstream file(file_name, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << file_name << std::endl;
        exit(1);
    }
    file << str;
    file.close();
}

ArgsType init(int argc, char *argv[]) {
    char mode = 0;
    std::string file_in_name;
    std::string file_out_name;
    std::string key16;

    for (int index = 1; index < argc; index++) {
        std::string arg = argv[index];

        // 检查参数是否为模式标志（-e 或 -d）
        if (arg == "-e" || arg == "-d") {
            if (mode != 0) {
                std::cerr << "Error: mode already set " << std::endl;
                exit(1);
            }
            if (index + 1 >= argc) {
                std::cerr << "Error: Missing input file path after " << arg
                          << std::endl;
                exit(1);
            }

            mode = arg[1];
            file_in_name = argv[++index];
        }
        // 检查参数是否为密钥字符串标志（-k）
        else if (arg == "-k") {
            if (key16 != "") {
                std::cerr << "Error: key string already set " << std::endl;
                exit(1);
            }
            if (index + 1 >= argc) {
                std::cerr << "Error: Missing key after -k" << std::endl;
                exit(1);
            }

            std::string key16_tmp = argv[++index];
            key16 = AES0::Fixkey16(key16_tmp);
        }
        // 检查参数是否为扩展密钥文件标志（-w）
        else if (arg == "-w") {
            if (key16 != "") {
                std::cerr << "Error: key already set " << std::endl;
                exit(1);
            }
            if (index + 1 >= argc) {
                std::cerr << "Error: Missing key file after -f" << std::endl;
                exit(1);
            }

            std::string key_file_name = argv[++index];
            std::string key64;
            read_str_from_file(key_file_name, key64);
            key16 = AES0::InvMixkey64(key64);
        }
        // 检查参数是否为输出文件标志（-o）
        else if (arg == "-o") {
            if (file_out_name != "") {
                std::cerr << "Error: output file already set " << std::endl;
                exit(1);
            }

            if (index + 1 >= argc) {
                std::cerr << "Error: Missing output file path after -o"
                          << std::endl;
                exit(1);
            }
            file_out_name = argv[++index];
        }
        // 其他参数均无效
        else {
            std::cerr << "Error: Invalid argument " << arg << std::endl;
            exit(1);
        }
    }

    // 检查是否提供了模式、文件路径和密钥字符串
    if (mode == 0 || file_in_name.empty() || file_out_name.empty()) {
        std::cerr << "Error: Missing required arguments.\n";
        std::cerr << usage_str();
        exit(1);
    }

    // 允许密钥为空，此时自动生成随机密钥
    if (key16.empty()) {
        std::cout << "use random key to encrypt\n";
        key16 = AES0::Fixkey16({});
    }

    // 在加密模式下，将扩充后的密钥key64自动存储在文件中
    if (mode == 'e') {
        std::string key_str64 = AES0::Mixkey64(key16);
        write_str_to_file(file_out_name + ".key", key_str64);
    }

    return {mode, file_in_name, file_out_name, key16};
}

void run(const ArgsType &args) {
    std::cout << "Mode: " << args.mode << std::endl;

    if (args.mode == 'e') { std::cout << "[Encrypting]\n"; }
    else { std::cout << "[Decrypting]\n"; }

    std::cout << "Input file: " << args.file_in_name << std::endl;
    std::cout << "Output file: " << args.file_out_name << std::endl;
    std::cout << "Key: " << args.key16 << std::endl;

    if (args.mode == 'e') {
        AES0::FileEncrypt(args.file_in_name, args.file_out_name, args.key16);
    }
    else {  // args.mode == 'd'
        AES0::FileDecrypt(args.file_in_name, args.file_out_name, args.key16);
    }
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        std::cout << usage_str();
        return 0;
    }

    auto args = init(argc, argv);

    run(args);

    return 0;
}