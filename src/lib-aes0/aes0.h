#include <string>

class AES0 {
public:
    static void FileEncrypt(const std::string &in_file,
                            const std::string &out_file,
                            const std::string &key_str);
    static void FileDecrypt(const std::string &in_file,
                            const std::string &out_file,
                            const std::string &key_str);

    static std::string Fixkey(const std::string &key);

    static std::string Mixkey(const std::string &key);
    static std::string InvMixkey(const std::string &mixkey);
};
