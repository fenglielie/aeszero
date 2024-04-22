#include <string>

class AES0 {
public:
    static void FileEncrypt(const std::string &in_file,
                            const std::string &out_file,
                            const std::string &key_str);
    static void FileDecrypt(const std::string &in_file,
                            const std::string &out_file,
                            const std::string &key_str);

    static std::string Fixkey16(const std::string &key);

    static std::string Mixkey64(const std::string &key16);
    static std::string InvMixkey64(const std::string &key64);
};
