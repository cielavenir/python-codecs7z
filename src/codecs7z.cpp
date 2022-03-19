#if defined(_WIN32) || (!defined(__GNUC__) && !defined(__clang__))
#define _hypot hypot
#include <cmath>
#endif

#include <pybind11/pybind11.h>

#include "StdAfx.h"
#include "Common/MyInitGuid.h"
#include "7zip/ICoder.h"

#include "7zip/Compress/DeflateEncoder.h"
#include "7zip/Compress/DeflateDecoder.h"
#include "7zip/Compress/BZip2Encoder.h"
#include "7zip/Compress/BZip2Decoder.h"

const unsigned int SLEEP_US = 10;

#include <chrono>
#include <thread>
#define usleep(usec) std::this_thread::sleep_for(std::chrono::microseconds(usec))

#if defined(_WIN32) || (!defined(__GNUC__) && !defined(__clang__))
#include "winpthreads.h"
#define ssize_t Py_ssize_t
#else
#include <pthread.h>
#endif

namespace py = pybind11;
using namespace pybind11::literals;

template <typename ... Args>
std::string format(const std::string& fmt, Args ... args ){
    // http://pyopyopyo.hatenablog.com/entry/2019/02/08/102456
    size_t len = std::snprintf( nullptr, 0, fmt.c_str(), args ... );
    std::vector<char> buf(len + 1);
    std::snprintf(&buf[0], len + 1, fmt.c_str(), args ... );
    return std::string(&buf[0], &buf[0] + len);
}

class compressobj_base: public ISequentialInStream, public ISequentialOutStream, public CMyUnknownImp{
    std::string instr;
    std::vector<char>outstr;
    bool requireInput;
    bool hasInput;
    bool first;

    unsigned int offset;
public:
    unsigned int level;
    pthread_t thread;
    int result;
    bool finished;
    compressobj_base(int level=-1):
        requireInput(false), hasInput(false), first(true), finished(false), result(0), thread(NULL),
        level(level)
    {
        outstr.reserve(65536);
    }
    ~compressobj_base(){
        if(thread){
            pthread_cancel(thread);
            pthread_detach(thread);
        }
    }

    MY_UNKNOWN_IMP2(ISequentialInStream, ISequentialOutStream)
    STDMETHOD(Write)(const void *_data, UInt32 size, UInt32 *processedSize){
        const char *data = (const char*)_data;
        outstr.insert(outstr.end(), data, data+size);
        *processedSize = size;
        return S_OK;
    }
    STDMETHOD(Read)(void *_data, UInt32 size, UInt32 *processedSize){
        char *data = (char*)_data;
        if(offset == instr.size()){
            requireInput = true;
            for(;!hasInput;)usleep(SLEEP_US);
            requireInput = false;
            offset = 0;
        }
        hasInput = false;
        unsigned int copysize = offset+size > instr.size() ? instr.size()-offset : size;
        memcpy(data,instr.data()+offset,copysize);
        offset+=copysize;
        *processedSize = copysize;
        return S_OK;
    }

    virtual int start_thread() = 0;

    py::bytes compress(const py::bytes &obj){
        outstr.resize(0);
        {
            char *buffer = nullptr;
            ssize_t length = 0;
            PYBIND11_BYTES_AS_STRING_AND_SIZE(obj.ptr(), &buffer, &length);
            instr = std::string(buffer, length);
            hasInput = true;
        }
        {
            py::gil_scoped_release release;
            if(first)offset=0,start_thread();
            first=false;
            for(;hasInput && !finished;)usleep(SLEEP_US);
            for(;!requireInput && !finished;)usleep(SLEEP_US);
            if(finished){
                pthread_join(thread,NULL);
                thread=NULL;
                if(result)throw std::runtime_error(format("Code() error (%d)", result));
            }
        }
        return py::bytes((char*)outstr.data(), outstr.size());
    }

    py::bytes flush(){
        outstr.resize(0);
        {
            instr = "";
            hasInput = true;
        }
        {
            py::gil_scoped_release release;
            if(first)offset=0,start_thread();
            first=false;
            for(;hasInput && !finished;)usleep(SLEEP_US);
            for(;!requireInput && !finished;)usleep(SLEEP_US);
            if(finished){
                pthread_join(thread,NULL);
                thread=NULL;
                if(result)throw std::runtime_error(format("Code() error (%d)", result));
            }
        }
        return py::bytes((char*)outstr.data(), outstr.size());
    }
};

class deflate_compressobj: public compressobj_base{
public:
    deflate_compressobj(int level=-1):
        compressobj_base(level)
    {
    }
    void impl(){
        AddRef(); // ISequentialInStream
        AddRef(); // ISequentialOutStream
        NCompress::NDeflate::NEncoder::CCOMCoder coder;
        if(level>=0){
            PROPID ID[]={NCoderPropID::kLevel};
            PROPVARIANT VAR[]={{VT_UI4,0,0,0,{0}}};
            VAR[0].uintVal=level;
            coder.SetCoderProperties(ID,VAR,1);
        }
        result = coder.Code(this, this, NULL, NULL, NULL);
        finished = true;
    }
    static void* C_impl(void *ptr){
        ((deflate_compressobj*)ptr)->impl();
        return NULL;
    }
    virtual int start_thread(){
        return pthread_create(&thread,NULL,C_impl,this);
    }
};

class deflate_decompressobj: public compressobj_base{
public:
    deflate_decompressobj():
        compressobj_base(-1)
    {
    }
    void impl(){
        AddRef(); // ISequentialInStream
        AddRef(); // ISequentialOutStream
        NCompress::NDeflate::NDecoder::CCOMCoder coder;
        result = coder.Code(this, this, NULL, NULL, NULL);
        finished = true;
    }
    static void* C_impl(void *ptr){
        ((deflate_decompressobj*)ptr)->impl();
        return NULL;
    }
    virtual int start_thread(){
        return pthread_create(&thread,NULL,C_impl,this);
    }
};

class deflate64_compressobj: public compressobj_base{
public:
    deflate64_compressobj(int level=-1):
        compressobj_base(level)
    {
    }
    void impl(){
        AddRef(); // ISequentialInStream
        AddRef(); // ISequentialOutStream
        NCompress::NDeflate::NEncoder::CCOMCoder64 coder;
        if(level>=0){
            PROPID ID[]={NCoderPropID::kLevel};
            PROPVARIANT VAR[]={{VT_UI4,0,0,0,{0}}};
            VAR[0].uintVal=level;
            coder.SetCoderProperties(ID,VAR,1);
        }
        result = coder.Code(this, this, NULL, NULL, NULL);
        finished = true;
    }
    static void* C_impl(void *ptr){
        ((deflate64_compressobj*)ptr)->impl();
        return NULL;
    }
    virtual int start_thread(){
        return pthread_create(&thread,NULL,C_impl,this);
    }
};

class deflate64_decompressobj: public compressobj_base{
public:
    deflate64_decompressobj():
        compressobj_base(-1)
    {
    }
    void impl(){
        AddRef(); // ISequentialInStream
        AddRef(); // ISequentialOutStream
        NCompress::NDeflate::NDecoder::CCOMCoder64 coder;
        result = coder.Code(this, this, NULL, NULL, NULL);
        finished = true;
    }
    static void* C_impl(void *ptr){
        ((deflate64_decompressobj*)ptr)->impl();
        return NULL;
    }
    virtual int start_thread(){
        return pthread_create(&thread,NULL,C_impl,this);
    }
};

class bzip2_compressobj: public compressobj_base{
public:
    bzip2_compressobj(int level=-1):
        compressobj_base(level)
    {
    }
    void impl(){
        AddRef(); // ISequentialInStream
        AddRef(); // ISequentialOutStream
        NCompress::NBZip2::CEncoder coder;
        if(level>=0){
            PROPID ID[]={NCoderPropID::kLevel};
            PROPVARIANT VAR[]={{VT_UI4,0,0,0,{0}}};
            VAR[0].uintVal=level;
            coder.SetCoderProperties(ID,VAR,1);
        }
        result = coder.Code(this, this, NULL, NULL, NULL);
        finished = true;
    }
    static void* C_impl(void *ptr){
        ((bzip2_compressobj*)ptr)->impl();
        return NULL;
    }
    virtual int start_thread(){
        return pthread_create(&thread,NULL,C_impl,this);
    }
};

class bzip2_decompressobj: public compressobj_base{
public:
    bzip2_decompressobj():
        compressobj_base(-1)
    {
    }
    void impl(){
        AddRef(); // ISequentialInStream
        AddRef(); // ISequentialOutStream
        NCompress::NBZip2::CDecoder coder;
        result = coder.Code(this, this, NULL, NULL, NULL);
        finished = true;
    }
    static void* C_impl(void *ptr){
        ((bzip2_decompressobj*)ptr)->impl();
        return NULL;
    }
    virtual int start_thread(){
        return pthread_create(&thread,NULL,C_impl,this);
    }
};

PYBIND11_MODULE(codecs7z, m){
    py::class_<deflate_compressobj, std::shared_ptr<deflate_compressobj> >(m, "deflate_compressobj")
    .def(py::init<int>(), "level"_a=-1)
    .def("compress", &deflate_compressobj::compress,
     "obj"_a
    )
    .def("flush", &deflate_compressobj::flush)
    .def_readwrite("eof", &deflate_compressobj::finished)
    ;

    py::class_<deflate_decompressobj, std::shared_ptr<deflate_decompressobj> >(m, "deflate_decompressobj")
    .def(py::init<>())
    .def("decompress", &deflate_decompressobj::compress,
     "obj"_a
    )
    .def_readwrite("eof", &deflate_decompressobj::finished)
    ;

    py::class_<deflate64_compressobj, std::shared_ptr<deflate64_compressobj> >(m, "deflate64_compressobj")
    .def(py::init<int>(), "level"_a=-1)
    .def("compress", &deflate64_compressobj::compress,
     "obj"_a
    )
    .def("flush", &deflate64_compressobj::flush)
    .def_readwrite("eof", &deflate64_compressobj::finished)
    ;

    py::class_<deflate64_decompressobj, std::shared_ptr<deflate64_decompressobj> >(m, "deflate64_decompressobj")
    .def(py::init<>())
    .def("decompress", &deflate64_decompressobj::compress,
     "obj"_a
    )
    .def_readwrite("eof", &deflate64_decompressobj::finished)
    ;

    py::class_<bzip2_compressobj, std::shared_ptr<bzip2_compressobj> >(m, "bzip2_compressobj")
    .def(py::init<int>(), "level"_a=-1)
    .def("compress", &bzip2_compressobj::compress,
     "obj"_a
    )
    .def("flush", &bzip2_compressobj::flush)
    .def_readwrite("eof", &bzip2_compressobj::finished)
    ;

    py::class_<bzip2_decompressobj, std::shared_ptr<bzip2_decompressobj> >(m, "bzip2_decompressobj")
    .def(py::init<>())
    .def("decompress", &bzip2_decompressobj::compress,
     "obj"_a
    )
    .def_readwrite("eof", &bzip2_decompressobj::finished)
    ;
}
