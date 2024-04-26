import subprocess
import sys

def run_test(command):
    result = subprocess.call(command, shell=True)
    return result

def run_main():
    # 成功加密
    if run_test("aeszero -e image.png -o image.aes") != 0:
        sys.exit(1)

    # 成功解密
    if run_test("aeszero -d image.aes -o image_new.png -f image.aes.key") != 0:
        sys.exit(1)

    # 失败解密
    if run_test("aeszero -d image.aes -o image_fail.png -k bsgbgvcxz") != 0:
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

    if are_files_equal("image.png", "image_new.png") == False:
        return 1

    if are_files_equal("image.png", "image_fail.png") == True:
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
