#!/usr/bin/env python2

import zlib
import bz2
import lz4.block
import snappy
import redis

from abc import ABCMeta, abstractmethod

little_data = 'FastoNoSQL'

big_data = """
FastoNoSQL it is GUI platform for NoSQL databases. Currently we support next databases:
  * Redis
  * Memcached
  * SSDB
  * LevelDB
  * RocksDB
  * UnQLite
  * LMDB
  * UpscaleDB
  * ForestDB
  * Pika
  For all this databases you can connect and use them in any way you need.
  Also ${BRANDING_PROJECT_NAME} is IDE tool for NoSQL which works on the most famous platforms like:
  Windows, Linux, MacOSX, Android, FreeBSD.
"""


class CompressorBase:
    __metaclass__ = ABCMeta

    @abstractmethod
    def name(self):
        pass

    @abstractmethod
    def compress(self, data):
        pass

    @abstractmethod
    def decompress(self, data):
        pass


class CompressorSnappy(CompressorBase):
    def __init__(self):
        pass

    def compress(self, data):
        return snappy.compress(data)

    def decompress(self, data):
        return snappy.decompress(data)

    def name(self):
        return 'snappy'


class CompressorLZ4(CompressorBase):
    def __init__(self):
        self.context = lz4.block

    def compress(self, data):
        return self.context.compress(data, store_size=False)

    def decompress(self, data):
        return self.context.decompress(data, uncompressed_size=2048)

    def name(self):
        return 'lz4'


class CompressorBzip2(CompressorBase):
    def __init__(self, compresslevel=9):
        self.compress_level = compresslevel

    def compress(self, data):
        return bz2.compress(data, self.compress_level)

    def decompress(self, data):
        return bz2.decompress(data)

    def name(self):
        return 'bzip2'


class CompressorZlib(CompressorBase):
    def __init__(self, gzip=False):
        zl = zlib.MAX_WBITS | 16 if gzip else zlib.MAX_WBITS
        c = zlib.compressobj(9, zlib.DEFLATED, zl)
        self.c_context = c.copy()

        d = zlib.decompressobj(zl)
        self.d_context = d.copy()
        self.is_gzlip = gzip

    def compress(self, data):
        c = self.c_context.copy()
        t = c.compress(data)
        t2 = c.flush(zlib.Z_FINISH)
        return t + t2

    def decompress(self, data):
        d = self.d_context.copy()
        t = d.decompress(data)
        while d.unconsumed_tail:
            t += d.decompress(d.unconsumed_tail)
        return t

    def name(self):
        return 'gzip' if self.is_gzlip else 'zlib'


def test_routine(compress, test_data):
    compressed = compress.compress(test_data)
    print('{0} initial length={1}, compressed={2}'.format(compress.name(), len(test_data), len(compressed)))
    dt = compress.decompress(compressed)
    assert dt == test_data
    return compressed


def test():
    r = redis.StrictRedis(host='localhost', port=6379, db=0)

    # zlib
    zlib_compress = CompressorZlib()
    zlib_little = test_routine(zlib_compress, little_data)
    r.set('zlib_little', zlib_little)
    zlib_big = test_routine(zlib_compress, big_data)
    r.set('zlib_big', zlib_big)

    # gzip
    gzip_compress = CompressorZlib(True)
    gzip_little = test_routine(gzip_compress, little_data)
    r.set('gzip_little', gzip_little)
    gzip_big = test_routine(zlib_compress, big_data)
    r.set('gzip_big', gzip_big)

    # bzip2
    bzip2_compress = CompressorBzip2()
    bzip2_little = test_routine(bzip2_compress, little_data)
    r.set('bzip2_little', bzip2_little)
    bzip2_big = test_routine(bzip2_compress, big_data)
    r.set('bzip2_big', bzip2_big)

    # lz4
    lz4_compress = CompressorLZ4()
    lz4_little = test_routine(lz4_compress, little_data)
    r.set('lz4_little', lz4_little)
    lz4_big = test_routine(lz4_compress, big_data)
    r.set('lz4_big', lz4_big)

    # snappy
    snappy_compress = CompressorSnappy()
    snappy_little = test_routine(snappy_compress, little_data)
    r.set('snappy_little', snappy_little)
    snappy_big = test_routine(snappy_compress, big_data)
    r.set('snappy_big', snappy_big)


test()
