cmake_minimum_required(VERSION 3.0)
project(codecs7z)

find_package(pybind11 REQUIRED)
pybind11_add_module(codecs7z
  ../src/codecs7z.cpp
  ../src/p7zip/CPP/7zip/Compress/DeflateEncoder.cpp ../src/p7zip/CPP/7zip/Compress/DeflateDecoder.cpp
  ../src/p7zip/CPP/7zip/Compress/BZip2Encoder.cpp ../src/p7zip/CPP/7zip/Compress/BZip2Decoder.cpp
  ../src/p7zip/CPP/7zip/Compress/BZip2Crc.cpp
  ../src/p7zip/CPP/7zip/Compress/LzOutWindow.cpp ../src/p7zip/CPP/7zip/Compress/BitlDecoder.cpp
  ../src/p7zip/CPP/7zip/Common/InBuffer.cpp ../src/p7zip/CPP/7zip/Common/OutBuffer.cpp
  ../src/p7zip/CPP/7zip/Common/CWrappers.cpp ../src/p7zip/CPP/7zip/Common/StreamUtils.cpp
  ../src/p7zip/C/Alloc.c ../src/p7zip/C/LzFind.c ../src/p7zip/C/HuffEnc.c ../src/p7zip/C/Sort.c ../src/p7zip/C/BwtSort.c ../src/p7zip/C/Threads.c
)
include_directories(../src/p7zip/CPP ../src/p7zip/CPP/Windows ../src/p7zip/CPP/myWindows ../src/p7zip/CPP/include_windows)
