import os
import codecs7z
from zipfile_deflate64 import deflate64
import zlib
import io

def test_codecs7z_deflate():
    bytesio = io.BytesIO()
    with open(os.path.join(os.path.dirname(__file__), '10000SalesRecords.csv'), 'rb') as f:
        content = f.read()
        f.seek(0)
        l = len(content)
        siz = 1024
        cnt = (l+siz-1)//siz
        dfl = codecs7z.deflate_compressobj(9)
        for i in range(cnt):
            bytesio.write(dfl.compress(f.read(siz)))
        bytesio.write(dfl.flush())
        # print(len(bytesio.getvalue()))
    bytesio.seek(0)
    ifl = zlib.decompressobj(-15)
    assert ifl.decompress(bytesio.read()) == content

'''
def test_codecs7z_inflate():
    bytesio = io.BytesIO()
    with open(os.path.join(os.path.dirname(__file__), '10000SalesRecords.csv'), 'rb') as f:
        content = f.read()
        f.seek(0)
        l = len(content)
        siz = 1024
        cnt = (l+siz-1)//siz
        dfl = codecs7z.deflate_compressobj()
        for i in range(cnt):
            bytesio.write(dfl.compress(f.read(siz)))
        bytesio.write(dfl.flush())
        # print(len(bytesio.getvalue()))
    bytesio.seek(0)
    ifl = codecs7z.deflate_decompressobj()
    assert ifl.decompress(bytesio.read())+ifl.decompress(b'') == content
'''

def test_codecs7z_deflate64():
    bytesio = io.BytesIO()
    with open(os.path.join(os.path.dirname(__file__), '10000SalesRecords.csv'), 'rb') as f:
        content = f.read()
        f.seek(0)
        l = len(content)
        siz = 1024
        cnt = (l+siz-1)//siz
        dfl = codecs7z.deflate64_compressobj(9)
        for i in range(cnt):
            bytesio.write(dfl.compress(f.read(siz)))
        bytesio.write(dfl.flush())
        # print(len(bytesio.getvalue()))
    bytesio.seek(0)
    ifl = deflate64.Deflate64()
    assert ifl.decompress(bytesio.read()) == content
