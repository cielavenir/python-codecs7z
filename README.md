[![PyPI](https://img.shields.io/pypi/v/codecs7z)](https://pypi.org/project/codecs7z/)

## codecs7z

a (light) binding for 7-zip codecs.

For now Deflate/Deflate64/BZip2 are covered.

### caveats

1. LZMA/PPMd are not covered as they are not self-contained format (decoding OPTIONs have to be specified and SetDecoderProperties2 even looks implementation-specific), hence not covered. For PPMd, use [pyppmd](https://pypi.org/project/pyppmd).

2. Deflate(64) decompressors have to be flushed, hence decompression is incompatible with zipfile. For Deflate64 decompression, use [zipfile-deflate64](https://pypi.org/project/zipfile-deflate64/).

## tested versions

- Python 2.7
- Python 3.9
- PyPy [2.7] 7.3.3
- PyPy [3.7] 7.3.5
    - For PyPy2, pip needs to be 20.1.x cf https://github.com/pypa/pip/issues/8653
    - PyPy needs to be 7.3.1+ cf https://github.com/pybind/pybind11/issues/2436
- Pyston [3.8] 2.3

## Windows installation

On Python <3.5 you need to switch src/p7zip submodule tag to build Windows wheel and sdist installation is not supported, so **for http-direct-install you need to use binary wheel.**
