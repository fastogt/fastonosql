#!/usr/bin/env python2

import redis
import string
import random

KEYS_COUNT = 10000


def random_string(length):
    return ''.join(random.choice(string.ascii_letters) for m in xrange(length))


def test():
    r = redis.StrictRedis(host='localhost', port=6379, db=0)

    for x in range(KEYS_COUNT / 2):
        r.set(random_string(random.randint(1, 9)), random_string(random.randint(1, 9)))

    for x in range(KEYS_COUNT / 2):
        ns = random_string(random.randint(1, 9)) + ':' + random_string(random.randint(1, 9))
        r.set(ns, random_string(random.randint(1, 9)))


test()
