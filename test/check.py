import sys


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
    if len(sys.argv) != 3:
        return 1

    file1_path = sys.argv[1]
    file2_path = sys.argv[2]
    result = are_files_equal(file1_path, file2_path)

    # 返回结果：0表示文件相等，1表示文件不相等
    return 0 if result else 1


if __name__ == "__main__":
    sys.exit(main())
