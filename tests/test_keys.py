import zlib
import redis

__doc__ = """
Compressor object for medium-sized, statistically-similar strings.

The idea is that you have a lot of moderate-sized strings (short email
messages or the like) that you would like to compress independently,
for storage in a lookup table where space is at a premium.  They
strings might be a few hundred bytes long on average.  That's not
enough to get much compression by gzipping without context.  gzip
works by starting with no knowledge, then building up knowledge (and
improving its compression ratio) as it goes along.

The trick is to "pre-seed" the gzip compressor with a bunch of text
(say a few kilobytes of messages concatenated) similar to the ones
that you want to compress separately, and pre-seed the gzip
decompressor with the same initial text.  That lets the compressor and
decompressor both start with enough knowledge to get good compression
even for fairly short strings.  This class puts a compressor and
decompressor into the same object, called a Compressor for convenience.

Usage: running the three lines

    compressor = Compressor(initial_seed)
    compressed_record = compressor.compress(some_record)
    restored_record = compressor.decompress(compressed_record)

where initial_seed is a few kilobytes of messages, and some_record is
a single record of maybe a few hundred bytes, for typical text, should
result in compressed_record being 50% or less of the size of
some_record, and restored_record being identical to some_record.
"""


class CompressorZlib(object):
    def __init__(self, gzip = False):
        zl = zlib.MAX_WBITS | 16  if gzip else zlib.MAX_WBITS
        c = zlib.compressobj(9, zlib.DEFLATED, zl)
        self.c_context = c.copy()

        d = zlib.decompressobj(zl)
        self.d_context = d.copy()

    def compress(self, text):
        c = self.c_context.copy()
        t = c.compress(text)
        t2 = c.flush(zlib.Z_FINISH)
        return t + t2

    def decompress(self, ctext):
        d = self.d_context.copy()
        t = d.decompress(ctext)
        while d.unconsumed_tail:
            t += d.decompress(d.unconsumed_tail)
        return t


def test():
    zlib_compress = CompressorZlib()
    zlib_test_string = "zlib is a pretty good compression algorithm"
    zlib_ct = zlib_compress.compress(zlib_test_string)

    r = redis.StrictRedis(host='localhost', port=6379, db=0)
    r.set('zlib', zlib_ct)

    print('initial length={0}, compressed={1}'.format(len(zlib_test_string), len(zlib_ct)))
    # the above string compresses from 43 bytes to 29 bytes using the
    # current doc text as compression seed, not bad for such short input.
    dt = zlib_compress.decompress(zlib_ct)
    assert dt == zlib_test_string

    gzip_compress = CompressorZlib(True)
    gzip_test_string = "gzip is a pretty good compression algorithm"
    gzip_ct = gzip_compress.compress(gzip_test_string)

    r.set('gzip', gzip_ct)

    print('initial length={0}, compressed={1}'.format(len(gzip_test_string), len(gzip_ct)))
    # the above string compresses from 43 bytes to 29 bytes using the
    # current doc text as compression seed, not bad for such short input.
    dt = gzip_compress.decompress(gzip_ct)
    assert dt == gzip_test_string

test()
