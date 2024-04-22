#include "aes0.h"

#include <fstream>
#include <iostream>
#include <string>

struct ArgsType {
    char mode;
    std::string file_in_name;
    std::string file_out_name;
    std::string key;
};

std::string usage_str() {
    std::string str0{
        "Usage: aeszero -e|-d <infile> -k <keystring> | -w <mixkeyfile> -o "
        "<outfile>\nExample:\n"};

    std::string str1{"Encrypt (1): aeszero -e <infile> -o <outfile>\n"};
    std::string str2{
        "Encrypt (2): aeszero -e <infile> -k <keystring> -o <outfile>\n"};
    std::string str3{
        "Encrypt (3): aeszero -e <infile> -w <keyfile> -o <outfile>\n"};

    std::string str4{
        "Decrypt (1): aeszero -d <infile> -k <keystring> -o <outfile>\n"};
    std::string str5{
        "Decrypt (2): aeszero -d <infile> -w <keyfile> -o <outfile>\n"};

    return str0 + str1 + str2 + str3 + str4 + str5;
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

ArgsType init(const int argc, const char *argv[]) {
    char mode = 0;
    std::string file_in_name;
    std::string file_out_name;
    std::string key;

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
            if (key != "") {
                std::cerr << "Error: key already set " << std::endl;
                exit(1);
            }
            if (index + 1 >= argc) {
                std::cerr << "Error: Missing key string after -k" << std::endl;
                exit(1);
            }

            std::string key_tmp = argv[++index];
            key = AES0::Fixkey(key_tmp);
        }
        // 检查参数是否为扩展密钥文件标志（-w）
        else if (arg == "-w") {
            if (key != "") {
                std::cerr << "Error: key already set " << std::endl;
                exit(1);
            }
            if (index + 1 >= argc) {
                std::cerr << "Error: Missing key file after -f" << std::endl;
                exit(1);
            }

            std::string key_file_name = argv[++index];
            std::string mix_key_tmp;
            read_str_from_file(key_file_name, mix_key_tmp);
            key = AES0::InvMixkey(mix_key_tmp);
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

    if (key.empty()) {
        if (mode == 'd') {
            // 解密模式下不允许密钥为空
            std::cerr << "Error: Missing key string.\n";
            std::cerr << usage_str();
            exit(1);
        }
        // 加密模式下允许密钥为空，此时自动生成随机密钥
        key = AES0::Fixkey({});
    }

    // 在加密模式下，将扩充后的密钥mixkey自动存储在文件中
    if (mode == 'e') {
        std::string mix_key_str = AES0::Mixkey(key);
        write_str_to_file(file_out_name + ".key", mix_key_str);
    }

    return {mode, file_in_name, file_out_name, key};
}

void run(const ArgsType &args) {
    if (args.mode == 'e') { std::cout << "[Encrypting]\n"; }
    else { std::cout << "[Decrypting]\n"; }

    std::cout << "Input file: " << args.file_in_name << std::endl;
    std::cout << "Output file: " << args.file_out_name << std::endl;
    std::cout << "Key: " << args.key << std::endl;

    if (args.mode == 'e') {
        AES0::FileEncrypt(args.file_in_name, args.file_out_name, args.key);
    }
    else {  // args.mode == 'd'
        AES0::FileDecrypt(args.file_in_name, args.file_out_name, args.key);
    }
}

int main(int argc, const char *argv[]) {
    // int main() {
    //     // 自定义命令行参数数量
    //     int argc = 7;
    //     // 自定义命令行参数数组
    //     const char *argv[] = {"eaeszero", "-e", "image.png",    "-k",
    //                           "12345",    "-o", "image2.png"};

    if (argc == 1) {
        std::cout << usage_str();
        return 0;
    }

    auto args = init(argc, argv);

    run(args);

    return 0;
}
