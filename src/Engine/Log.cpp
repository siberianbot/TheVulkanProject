#include "Log.hpp"

void Log::push(LogCategory category, const std::string &tag, const std::string &msg) {
    LogEntry entry = {
            .category = category,
            .tag = tag,
            .msg = msg
    };

    this->_buffer.push_back(entry);
}
