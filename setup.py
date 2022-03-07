import sys
from os.path import dirname
from os.path import abspath
sys.path.append(dirname(abspath(__file__)))
import monkeypatch_distutils

from setuptools import setup
try:
	from pybind11.setup_helpers import Pybind11Extension
except ImportError:
	from setuptools import Extension as Pybind11Extension

ext_modules = [
    Pybind11Extension(
        "codecs7z",
        [
            'src/codecs7z.cpp',
            'src/p7zip/CPP/7zip/Compress/DeflateEncoder.cpp', 'src/p7zip/CPP/7zip/Compress/DeflateDecoder.cpp',
            'src/p7zip/CPP/7zip/Compress/BZip2Encoder.cpp', 'src/p7zip/CPP/7zip/Compress/BZip2Decoder.cpp',
            'src/p7zip/CPP/7zip/Compress/BZip2Crc.cpp',
            'src/p7zip/CPP/7zip/Compress/LzOutWindow.cpp', 'src/p7zip/CPP/7zip/Compress/BitlDecoder.cpp',
            'src/p7zip/CPP/7zip/Common/InBuffer.cpp', 'src/p7zip/CPP/7zip/Common/OutBuffer.cpp',
            'src/p7zip/CPP/7zip/Common/CWrappers.cpp', 'src/p7zip/CPP/7zip/Common/StreamUtils.cpp',
            'src/p7zip/C/Alloc.c', 'src/p7zip/C/LzFind.c', 'src/p7zip/C/HuffEnc.c', 'src/p7zip/C/Sort.c', 'src/p7zip/C/BwtSort.c', 'src/p7zip/C/Threads.c',
        ],
        include_dirs=['src/p7zip/CPP', 'src/p7zip/CPP/myWindows', 'src/p7zip/CPP/include_windows'],
        extra_compile_args=['-O2'],
        extra_link_args=['-s'],
        #extra_link_args=['-Wl,--no-undefined'],
    ),
]

setup(
    name='codecs7z',
    description='a (light) binding for 7-zip codecs',
    long_description=open("README.md").read(),
    version='0.0.0.2',
    url='https://github.com/cielavenir/python-codecs7z',
    license='LGPL',
    author='cielavenir',
    author_email='cielartisan@gmail.com',
    setup_requires=["pybind11"],
    ext_modules=ext_modules,
    #cmdclass={"build_ext": build_ext},
    zip_safe=False,
    include_package_data=True,
    # platforms='any',
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: Python Software Foundation License',
        'Operating System :: POSIX',
        #'Operating System :: Microsoft :: Windows',
        'Operating System :: MacOS :: MacOS X',
        'Topic :: Software Development :: Libraries',
        'Topic :: Utilities',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: Implementation :: PyPy',
    ]
)
