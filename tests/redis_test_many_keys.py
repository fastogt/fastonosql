#!/usr/bin/env python2

import redis
import string
import random

KEYS_COUNT = 1000


def random_string(length):
    return ''.join(random.choice(string.ascii_letters) for m in xrange(length))


def test():
    r = redis.StrictRedis(host='localhost', port=6379, db=0)

    for x in range(KEYS_COUNT):
        r.set(random_string(random.randint(1, 9)), random_string(random.randint(1, 9)))


test()
