import subprocess
import sys

def run_test(command):
    result = subprocess.call(command, shell=True)
    return result

def run_main():
    # 成功加密
    if run_test("aeszero -e test.txt -k abcdefghijklmnop -o test.aes") != 0:
        sys.exit(1)

    # 成功解密
    if run_test("aeszero -d test.aes -k abcdefghijklmnop -o test_new.txt") != 0:
        sys.exit(1)

    # 成功解密
    if run_test("aeszero -d test.aes -f test.aes.key -o test_new2.txt") != 0:
        sys.exit(1)

    # 失败解密
    if run_test("aeszero -d test.aes -k abcdefghijklm -o test_fail.txt") != 0:
        sys.exit(1)


def are_files_equal(file1_path, file2_path):
    with open(file1_path, "rb") as file1, open(file2_path, "rb") as file2:
        while True:
            data1 = file1.read(1024)
            data2 = file2.read(1024)

            if data1 != data2:
                return False

            if (not data1) and (not data2):
                break

        return True


def main():
    run_main()

    if are_files_equal("test.txt", "test_new.txt") == False:
        return 1

    if are_files_equal("test.txt", "test_new2.txt") == False:
        return 1

    if are_files_equal("test.txt", "test_fail.txt") == True:
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
