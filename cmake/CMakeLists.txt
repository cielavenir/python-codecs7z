cmake_minimum_required(VERSION 3.0)
project(slz)

find_package(pybind11 REQUIRED)
pybind11_add_module(slz ../src/pyslz.cpp ../src/libslz/src/slz.c)
