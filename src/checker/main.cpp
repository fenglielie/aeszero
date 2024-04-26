#include <cstring>
#include <fstream>
#include <iostream>

bool are_files_equal(const std::string &file1_path,
                     const std::string &file2_path) {
    std::ifstream file1(file1_path, std::ios::binary);
    std::ifstream file2(file2_path, std::ios::binary);

    if (!file1.is_open() || !file2.is_open()) {
        std::cerr << "Error opening files." << std::endl;
        return false;
    }

    char buffer1[1024];
    char buffer2[1024];

    while (true) {
        // 逐个读取文件块
        file1.read(buffer1, sizeof(buffer1));
        file2.read(buffer2, sizeof(buffer2));

        // 比较读取到的数据
        std::streamsize bytesRead1 = file1.gcount();
        std::streamsize bytesRead2 = file2.gcount();

        if (bytesRead1 != bytesRead2
            || std::memcmp(buffer1, buffer2, bytesRead1) != 0) {
            return false;
        }

        // 如果两文件都读完，返回 true
        if (file1.eof() && file2.eof()) { return true; }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <file1> <file2>" << std::endl;
        return 1;
    }

    std::string file1_path = argv[1];
    std::string file2_path = argv[2];

    bool is_same = are_files_equal(file1_path, file2_path);

    return is_same ? 0 : 1;
}
