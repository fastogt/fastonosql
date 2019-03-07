#!/usr/bin/env python2

import sys
import pickle
import argparse

if __name__ == "__main__":
    argc = len(sys.argv)

    parser = argparse.ArgumentParser()
    parser.add_argument('data', help='pickle encode/decode hexed string', type=str)
    parser.add_argument('--encode', action='store_true', help='pickle encode string')
    parser.add_argument('--decode', action='store_false', help='pickle decode string')
    args = parser.parse_args()

    hexed_data = args.data
    unhexed = hexed_data.replace('x', '')  # 1122
    raw_pickle = unhexed.decode('hex')

    try:
        if args.encode:
            pickle_str = pickle.dumps(raw_pickle)
        else:
            pickle_str = pickle.loads(raw_pickle)
    except Exception as ex:
        sys.exit(1)

    result_str = ''.join('\\x{:02x}'.format(ord(x)) for x in pickle_str)
    print(result_str)
    sys.exit(0)
