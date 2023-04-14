#ifndef ENGINE_LOGCATEGORY_HPP
#define ENGINE_LOGCATEGORY_HPP

#include <stdexcept>
#include <string_view>

static constexpr const std::string_view VERBOSE_LOG_CATEGORY_STRING = "verbose";
static constexpr const std::string_view INFO_LOG_CATEGORY_STRING = "info";
static constexpr const std::string_view WARNING_LOG_CATEGORY_STRING = "warning";
static constexpr const std::string_view ERROR_LOG_CATEGORY_STRING = "ERROR";

enum LogCategory {
    VERBOSE_LOG_CATEGORY,
    INFO_LOG_CATEGORY,
    WARNING_LOG_CATEGORY,
    ERROR_LOG_CATEGORY
};

constexpr std::string_view toString(const LogCategory &category) {
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

    throw std::out_of_range("Not supported");
}

#endif // ENGINE_LOGCATEGORY_HPP
