#ifndef ENGINE_LOG_HPP
#define ENGINE_LOG_HPP

#include <exception>
#include <string>
#include <string_view>

#include "src/Engine/LogCategory.hpp"
#include "src/Utils/CircularBuffer.hpp"

struct LogEntry {
    LogCategory category;
    std::string tag;
    std::string msg;
};

class Log {
private:
    CircularBuffer<LogEntry, 1024> _buffer;

public:
    void push(LogCategory category, const std::string_view &tag, const std::string_view &msg);

    void verbose(const std::string_view &tag, const std::string_view &msg);
    void info(const std::string_view &tag, const std::string_view &msg);
    void warning(const std::string_view &tag, const std::string_view &msg);
    void warning(const std::string_view &tag, const std::exception &error);
    void error(const std::string_view &tag, const std::string_view &msg);
    void error(const std::string_view &tag, const std::exception &error);

    [[nodiscard]] CircularBuffer<LogEntry, 1024> &buffer() { return this->_buffer; }
};

#endif // ENGINE_LOG_HPP
