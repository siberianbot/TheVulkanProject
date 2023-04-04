#ifndef ENGINE_LOG_HPP
#define ENGINE_LOG_HPP

#include <exception>
#include <string>

#include "src/Utils/CircularBuffer.hpp"

enum LogCategory {
    INFO_LOG_CATEGORY,
    WARNING_LOG_CATEGORY,
    ERROR_LOG_CATEGORY
};

std::string toString(const LogCategory &category);

struct LogEntry {
    LogCategory category;
    std::string tag;
    std::string msg;
};

class Log {
private:
    CircularBuffer<LogEntry, 1024> _buffer;

public:
    void push(LogCategory category, const std::string &tag, const std::string &msg);

    void info(const std::string &tag, const std::string &msg);
    void warning(const std::string &tag, const std::string &msg);
    void warning(const std::string &tag, const std::exception &error);
    void error(const std::string &tag, const std::exception &error);

    [[nodiscard]] CircularBuffer<LogEntry, 1024> &buffer() { return this->_buffer; }
};

#endif // ENGINE_LOG_HPP
