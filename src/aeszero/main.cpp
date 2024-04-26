#include "aes0.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

struct ArgsType {
    char mode;
    bool show_msg;
    std::string file_in_name;
    std::string file_out_name;
    std::string mixkey;
};

bool oneof(const std::string &str, const std::vector<std::string> &str_set) {
    for (const auto &s : str_set) {  // NOLINT(readability-use-anyofallof)
        if (str == s) { return true; }
    }
    return false;
}

std::string version_str() { return "aeszero (version " VERSION_INFO ")\n"; }

std::string usage_str() {
    std::string usage = "Usage: aeszero -e|-d <infile> -k <keystring> | -f "
                        "<mixkeyfile> -o <outfile>\n"
                        "Example:\n";

    std::string encrypt_examples =
        "Encrypt (1): aeszero -e <infile> -o <outfile>\n"
        "Encrypt (2): aeszero -e <infile> -k <keystring> -o <outfile>\n"
        "Encrypt (3): aeszero -e <infile> -f <keyfile> -o <outfile>\n";

    std::string decrypt_examples =
        "Decrypt (1): aeszero -d <infile> -k <keystring> -o <outfile>\n"
        "Decrypt (2): aeszero -d <infile> -f <keyfile> -o <outfile>\n";

    std::string web = "For more information, please visit "
                      "https://github.com/fenglielie/aeszero\n";

    return usage + encrypt_examples + decrypt_examples + web;
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

ArgsType init_smart(const char *single_arg) {
    char mode = 0;
    std::string file_in_name;
    std::string file_out_name;
    std::string mixkey;
    std::string arg1 = single_arg;

    auto ends_with = [](const std::string &str, const std::string &suffix) {
        return str.length() >= suffix.length()
               && str.substr(str.length() - suffix.length()) == suffix;
    };

    mode = ends_with(arg1, ".aes0") ? 'd' : 'e';

    if (mode == 'd') {
        file_in_name = arg1;
        file_out_name = file_in_name + ".dec";
        std::string key_file_name = file_in_name + ".key";
        std::string mixkey_tmp;
        read_str_from_file(key_file_name, mixkey_tmp);
        mixkey = AES0::Mixkey(AES0::InvMixkey(mixkey_tmp));
    }
    else {  // mode == 'e'
        file_in_name = arg1;
        file_out_name = file_in_name + ".aes0";
        mixkey = AES0::Mixkey(AES0::Fixkey(""));
    }

    return {mode, false, file_in_name, file_out_name, mixkey};
}

// NOLINTNEXTLINE
ArgsType init(const int argc, const char *argv[]) {
    char mode = 0;
    bool show_msg = true;
    std::string file_in_name;
    std::string file_out_name;
    std::string mixkey;

    for (int index = 1; index < argc; index++) {
        std::string arg = argv[index];

        // 检查参数是否为加密模式标志
        if (oneof(arg, {"-e", "-E", "--encrypt"})) {
            if (mode != 0) {
                std::cerr << "Error: mode already set " << std::endl;
                exit(1);
            }
            if (index + 1 >= argc) {
                std::cerr << "Error: Missing input file path after " << arg
                          << std::endl;
                exit(1);
            }

            mode = 'e';
            file_in_name = argv[++index];
        }
        // 检查参数是否为解密模式标志
        else if (oneof(arg, {"-d", "-D", "--decrypt"})) {
            if (mode != 0) {
                std::cerr << "Error: mode already set " << std::endl;
                exit(1);
            }
            if (index + 1 >= argc) {
                std::cerr << "Error: Missing input file path after " << arg
                          << std::endl;
                exit(1);
            }

            mode = 'd';
            file_in_name = argv[++index];
        }
        // 检查参数是否为密钥字符串标志
        else if (oneof(arg, {"-k", "-K", "--key"})) {
            if (mixkey != "") {
                std::cerr << "Error: key already set " << std::endl;
                exit(1);
            }
            if (index + 1 >= argc) {
                std::cerr << "Error: Missing key string after -k" << std::endl;
                exit(1);
            }

            std::string key_tmp = argv[++index];
            mixkey = AES0::Mixkey(AES0::Fixkey(key_tmp));
        }
        // 检查参数是否为扩展密钥文件标志
        else if (oneof(arg, {"-f", "-F", "--keyfile"})) {
            if (mixkey != "") {
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
            mixkey = AES0::Mixkey(AES0::InvMixkey(mix_key_tmp));
        }
        // 检查参数是否为输出文件标志
        else if (oneof(arg, {"-o", "-O", "--output"})) {
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
        // 关闭输出信息
        else if (oneof(arg, {"-q", "-Q", "--quiet"})) { show_msg = false; }
        // 其他参数均无效
        else {
            std::cerr << "Error: Invalid argument " << arg << std::endl;
            exit(1);
        }
    }

    // 检查是否提供了模式、文件路径和密钥字符串
    if (mode == 0 || file_in_name.empty() || file_out_name.empty()) {
        std::cerr << "Error: Missing required arguments.\n" << usage_str();
        exit(1);
    }

    if (mixkey.empty()) {
        if (mode == 'd') {
            // 解密模式下不允许密钥为空
            std::cerr << "Error: Missing key string.\n" << usage_str();
            exit(1);
        }
        // 加密模式下允许密钥为空，此时自动生成随机密钥
        mixkey = AES0::Mixkey(AES0::Fixkey({}));
    }

    return {mode, show_msg, file_in_name, file_out_name, mixkey};
}

void run(const ArgsType &args) {
    if (args.show_msg) {
        if (args.mode == 'e') { std::cout << "[Encrypting]\n"; }
        else { std::cout << "[Decrypting]\n"; }

        std::cout << "Input file: " << args.file_in_name << std::endl;
        std::cout << "Output file: " << args.file_out_name << std::endl;
        std::cout << "Key: " << args.mixkey << std::endl;
    }

    if (args.mode == 'e') {
        AES0::FileEncrypt(args.file_in_name, args.file_out_name,
                          AES0::InvMixkey(args.mixkey));
        write_str_to_file(args.file_out_name + ".key", args.mixkey);
    }
    else {  // args.mode == 'd'
        AES0::FileDecrypt(args.file_in_name, args.file_out_name,
                          AES0::InvMixkey(args.mixkey));
    }
}

int main(int argc, const char *argv[]) {
    if (argc == 1) {
        std::cout << version_str() << usage_str();
        return 0;
    }

    if (argc == 2) {
        if (oneof(argv[1], {"-v", "-V", "--version"})) {
            std::cout << version_str();
            return 0;
        }

        if (oneof(argv[1], {"-h", "-H", "--help"})) {
            std::cout << usage_str();
            return 0;
        }

        auto args = init_smart(argv[1]);
        run(args);
    }
    else {
        auto args = init(argc, argv);
        run(args);
    }

    return 0;
}
