import subprocess
import sys


def run_test(command):
    result = subprocess.call(command, shell=True)
    return result


def run_main():
    command = sys.argv[1]

    # 成功加密
    if run_test(command + " demo.mp4") != 0:
        sys.exit(1)

    # 成功解密
    if run_test(command + " demo.mp4.aes0") != 0:
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

    if are_files_equal("demo.mp4", "demo.mp4.aes0.dec") == False:
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
