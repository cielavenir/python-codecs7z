#include <pybind11/pybind11.h>

#include "StdAfx.h"
#include "Common/MyInitGuid.h"
#include "7zip/ICoder.h"

extern "C" {
#include "libslz/src/slz.h"
}

namespace py = pybind11;
using namespace pybind11::literals;

class slz_compressobj{
    slz_stream strm;
    std::string out;
    int outsize;
public:
    slz_compressobj(int level=1, int format=SLZ_FMT_DEFLATE): outsize(0){
        slz_init(&strm, level, format);
    }
    py::bytes compress(const py::bytes &obj){
        int tempoutsize = py::len(obj)+py::len(obj)/16;
        if(outsize < tempoutsize){
            outsize = tempoutsize;
            out.reserve(outsize);
        }
        //out.resize(tempoutsize);
        size_t written = 0;
        {
            char *buffer = nullptr;
            ssize_t length = 0;
            PYBIND11_BYTES_AS_STRING_AND_SIZE(obj.ptr(), &buffer, &length);
            written = slz_encode(&strm, &out[0], buffer, length, 1);
        }
        //out.resize(written);
        return py::bytes(out.data(), written);
    }
    py::bytes flush(){
        int tempoutsize = 12;
        if(outsize < tempoutsize){
            outsize = tempoutsize;
            out.reserve(outsize);
        }
        //out.resize(tempoutsize);
        size_t written = slz_finish(&strm, &out[0]);
        //out.resize(written);
        return py::bytes(out.data(), written);
    }
};

PYBIND11_MODULE(slz, m){
    py::class_<slz_compressobj, std::shared_ptr<slz_compressobj> >(m, "compressobj")
    .def(py::init<int, int>(), "level"_a=1, "format"_a=int(SLZ_FMT_DEFLATE))
    .def("compress", &slz_compressobj::compress,
     "obj"_a
    )
    .def("flush", &slz_compressobj::flush)
    ;

    m.attr("SLZ_FMT_GZIP") = int(SLZ_FMT_GZIP);
    m.attr("SLZ_FMT_ZLIB") = int(SLZ_FMT_ZLIB);
    m.attr("SLZ_FMT_DEFLATE") = int(SLZ_FMT_DEFLATE);
}
