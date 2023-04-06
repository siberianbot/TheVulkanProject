#ifndef UTILS_DATASTREAM_HPP
#define UTILS_DATASTREAM_HPP

#include <istream>
#include <streambuf>

#include "src/Types/DataBuffer.hpp"

class DataStreamBuf : public std::basic_streambuf<char> {
public:
    DataStreamBuf(DataBuffer &data) {
        this->setg(data.data(), data.data(), data.data() + data.size());
    }

    DataStreamBuf(const DataStreamBuf &dataStreamBuf)
            : std::basic_streambuf<char>(dataStreamBuf) {
        //
    }

    ~DataStreamBuf() override = default;
};

class DataStream : public std::basic_istream<char> {
private:
    DataStreamBuf _buffer;

public:
    DataStream(DataBuffer &data)
            : std::basic_istream<char>(&_buffer),
              _buffer(data) {
        //
    }

    ~DataStream() override = default;
};

#endif // UTILS_DATASTREAM_HPP
