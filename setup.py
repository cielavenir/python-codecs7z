import sys
import platform
from os.path import join
from os.path import basename
from os.path import dirname
from os.path import abspath
from os.path import isfile
sys.path.append(dirname(abspath(__file__)))
import monkeypatch_distutils
import subprocess

from setuptools import setup
from setuptools.dist import Distribution
from setuptools.command.build_ext import build_ext
try:
    from pybind11.setup_helpers import Pybind11Extension
except ImportError:
    from setuptools import Extension as Pybind11Extension

extra_compile_args=['-O2']
class build_ext_hook(build_ext, object):
    def build_extension(self, ext):
        if platform.system() == 'Windows':
            if sys.maxsize < 1<<32:
                msiz = '-m32'
                plat = 'win32'
                win64flags = []
            else:
                msiz = '-m64'
                plat = 'win-amd64'
                win64flags = ['-DMS_WIN64=1']
            if sys.version_info < (3,5):
                import sysconfig
                import pybind11
                source = ext.sources[0]
                objname = basename(source)+'.o'
                subprocess.check_call([
                    'clang++', msiz, '-c', '-o', objname,
                    '-DHAVE_UINTPTR_T=1',
                    '-I', sysconfig.get_paths()['include'],
                    '-I', sysconfig.get_paths()['platinclude'],
                    '-I', pybind11.get_include(),
                    source]+extra_compile_args+win64flags+sum((['-I', dir] for dir in ext.include_dirs), []))
                ext.extra_objects.append(objname)
                ext.sources.pop(0)
                if True:
                    for source in ext.sources:
                        objname = basename(source)+'.o'
                        cmd = 'clang' if source.endswith('.c') else 'clang++'
                        subprocess.check_call([
                            cmd, msiz, '-c', '-o', objname, source
                        ]+extra_compile_args+sum((['-I', dir] for dir in ext.include_dirs), []))
                        ext.extra_objects.append(objname)
                    pydpath = 'build/lib.%s-%d.%d/%s.pyd'%(plat, sys.hexversion // 16777216, sys.hexversion // 65536 % 256, ext.name.replace('.', '/'))
                    subprocess.check_call(['mkdir', '-p', dirname(pydpath)])
                    libname = 'python%d%d.lib'%(sys.hexversion // 16777216, sys.hexversion // 65536 % 256)
                    # https://stackoverflow.com/a/48360354/2641271
                    d = Distribution()
                    b = d.get_command_class('build_ext')(d)
                    b.finalize_options()
                    libpath = next(join(dir, libname) for dir in b.library_dirs if isfile(join(dir, libname)))
                    print(libpath)
                    subprocess.check_call([
                        'clang++', msiz, '-shared', '-o', pydpath,
                    ]+ext.extra_objects+ext.extra_link_args+[libpath])
                    return
        build_ext.build_extension(self, ext)

ext_modules = [
    Pybind11Extension(
        name="codecs7z",
        sources=[
            'src/codecs7z.cpp',
            'src/p7zip/CPP/7zip/Compress/DeflateEncoder.cpp', 'src/p7zip/CPP/7zip/Compress/DeflateDecoder.cpp',
            'src/p7zip/CPP/7zip/Compress/BZip2Encoder.cpp', 'src/p7zip/CPP/7zip/Compress/BZip2Decoder.cpp',
            'src/p7zip/CPP/7zip/Compress/BZip2Crc.cpp',
            'src/p7zip/CPP/7zip/Compress/LzOutWindow.cpp', 'src/p7zip/CPP/7zip/Compress/BitlDecoder.cpp',
            'src/p7zip/CPP/7zip/Common/InBuffer.cpp', 'src/p7zip/CPP/7zip/Common/OutBuffer.cpp',
            'src/p7zip/CPP/7zip/Common/CWrappers.cpp', 'src/p7zip/CPP/7zip/Common/StreamUtils.cpp',
            'src/p7zip/C/Alloc.c', 'src/p7zip/C/LzFind.c', 'src/p7zip/C/HuffEnc.c', 'src/p7zip/C/Sort.c', 'src/p7zip/C/BwtSort.c', 'src/p7zip/C/Threads.c',
        ],
        include_dirs=['src/p7zip/CPP', 'src/p7zip/CPP/Windows', 'src/p7zip/CPP/myWindows', 'src/p7zip/CPP/include_windows'],
        extra_objects=[],
        extra_compile_args=list(extra_compile_args),
        extra_link_args=['-s'],
        #extra_link_args=['-Wl,--no-undefined'],
    ),
]

setup(
    name='codecs7z',
    description='a (light) binding for 7-zip codecs',
    long_description=open("README.md").read(),
    version='0.0.0.6',
    url='https://github.com/cielavenir/python-codecs7z',
    license='LGPL',
    author='cielavenir',
    author_email='cielartisan@gmail.com',
    setup_requires=["pybind11"],
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext_hook},
    zip_safe=False,
    include_package_data=True,
    # platforms='any',
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: GNU Lesser General Public License v2 or later (LGPLv2+)',
        'Operating System :: POSIX',
        'Operating System :: Microsoft :: Windows',
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
