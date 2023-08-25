#!/usr/bin/python3
import sys
import os


def _creat_bss_info(product):
    bin_file = f"../../Project/C/build/linux/{product.lower()}/{product.upper()}-BIN"
    bss_file = f"./output/{product.lower()}-bss.txt"

    if not os.path.exists("./output"):
        os.makedirs("./output")
    if not os.path.exists(bin_file):
        print(f'file:{bin_file} not exist!')
        return
    if os.path.exists(bss_file):
        os.remove(bss_file)

    os.system(f"objdump -j .bss -tT {bin_file} >> {bss_file}")
    return bss_file


def _get_bss_info(bss_file):
    bss_info = []
    total_size = 0
    with open(bss_file, "r", encoding="utf-8") as file_id:
        for line in file_id:
            line = line.split()
            if (len(line) == 6) and (line[2] == 'O') and (line[3] == '.bss'):
                total_size += int(line[4], 16)

                if int(line[4], 16) > 1024*1024:
                    line.append(str(int(line[4], 16)/1024/1024) + "MB")
                else:
                    line.append(str(int(line[4], 16)/1024) + "KB")
                bss_info.append(line)
    bss_info = sorted(bss_info, key=lambda x:x[4], reverse=True)

    if total_size > 1024*1024:
        total_size = str(total_size/1024/1024) + "MB"
    else:
        total_size = str(total_size/1024) + "KB"
    bss_info.insert(0, ["0000000000000000", "l", "O", ".bss", "0000000000000000", "Total Size", f"{total_size}"])
    return bss_info


def _save_bss_info(bss_file, bss_info):
    if os.path.exists(bss_file):
        os.remove(bss_file)

    with open(bss_file, "w", encoding="utf-8") as file_id:
        for line in bss_info:
            file_id.write(f"{line}\n")
    return


# =====================================================================


def main():
    if len(sys.argv) < 2:
        print(f'give invalid param:{sys.argv}')
        return
    product = sys.argv[1]

    bss_file = _creat_bss_info(product)
    bss_info = _get_bss_info(bss_file)
    _save_bss_info(bss_file, bss_info)
    return


if __name__ == '__main__':
    main()