#!/usr/bin/env python3
import sys
import pickle


def print_usage():
    print("Usage:\n"
          "[required] argv[1] hexed data\n")


if __name__ == "__main__":
    argc = len(sys.argv)

    if argc > 1:
        hexed_data = sys.argv[1]
    else:
        print_usage()
        sys.exit(1)

    removed_x = hexed_data.replace('x', '')  # 1122
    unhexed = bytes.fromhex(removed_x)  # bytes 1122

    try:
        pickle_bytes = pickle.loads(unhexed, encoding="latin-1")
    except Exception as ex:
        sys.exit(1)

    result_str = ''.join('\\x{:02x}'.format(ord(x)) for x in pickle_bytes)
    # result_str = ''.join('\\x{:02x}'.format(x) for x in pickle_str)  # \x11\x22
    print(result_str)
    sys.exit(0)
