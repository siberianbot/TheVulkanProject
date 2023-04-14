#include "Log.hpp"

#include <iostream>

#include <fmt/core.h>

void Log::push(LogCategory category, const std::string_view &tag, const std::string_view &msg) {
    LogEntry entry = {
            .category = category,
            .tag = std::string(tag),
            .msg = std::string(msg)
    };

    this->_buffer.push_back(entry);

    std::cout << fmt::format("{0}\t{1}: {2}", toString(category), tag, msg) << std::endl;
}

void Log::verbose(const std::string_view &tag, const std::string_view &msg) {
    this->push(VERBOSE_LOG_CATEGORY, tag, msg);
}

void Log::info(const std::string_view &tag, const std::string_view &msg) {
    this->push(INFO_LOG_CATEGORY, tag, msg);
}

void Log::warning(const std::string_view &tag, const std::string_view &msg) {
    this->push(WARNING_LOG_CATEGORY, tag, msg);
}

void Log::warning(const std::string_view &tag, const std::exception &error) {
    this->push(WARNING_LOG_CATEGORY, tag, error.what());
}

void Log::error(const std::string_view &tag, const std::string_view &msg) {
    this->push(ERROR_LOG_CATEGORY, tag, msg);
}

void Log::error(const std::string_view &tag, const std::exception &error) {
    this->push(ERROR_LOG_CATEGORY, tag, error.what());
}
