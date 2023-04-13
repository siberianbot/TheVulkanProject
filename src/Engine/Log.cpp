#include "Log.hpp"

static constexpr const char *VERBOSE_LOG_CATEGORY_STRING = "verbose";
static constexpr const char *INFO_LOG_CATEGORY_STRING = "info";
static constexpr const char *WARNING_LOG_CATEGORY_STRING = "warning";
static constexpr const char *ERROR_LOG_CATEGORY_STRING = "ERROR";

std::string toString(const LogCategory &category) {
    switch (category) {
        case VERBOSE_LOG_CATEGORY:
            return VERBOSE_LOG_CATEGORY_STRING;

        case INFO_LOG_CATEGORY:
            return INFO_LOG_CATEGORY_STRING;

        case WARNING_LOG_CATEGORY:
            return WARNING_LOG_CATEGORY_STRING;

        case ERROR_LOG_CATEGORY:
            return ERROR_LOG_CATEGORY_STRING;
    }

    throw std::runtime_error("Not supported");
}

void Log::push(LogCategory category, const std::string &tag, const std::string &msg) {
    LogEntry entry = {
            .category = category,
            .tag = tag,
            .msg = msg
    };

    this->_buffer.push_back(entry);
}

void Log::verbose(const std::string &tag, const std::string &msg) {
    this->push(VERBOSE_LOG_CATEGORY, tag, msg);
}

void Log::info(const std::string &tag, const std::string &msg) {
    this->push(INFO_LOG_CATEGORY, tag, msg);
}

void Log::warning(const std::string &tag, const std::string &msg) {
    this->push(WARNING_LOG_CATEGORY, tag, msg);
}

void Log::warning(const std::string &tag, const std::exception &error) {
    this->push(WARNING_LOG_CATEGORY, tag, error.what());
}

void Log::error(const std::string &tag, const std::string &msg) {
    this->push(ERROR_LOG_CATEGORY, tag, msg);
}

void Log::error(const std::string &tag, const std::exception &error) {
    this->push(ERROR_LOG_CATEGORY, tag, error.what());
}
