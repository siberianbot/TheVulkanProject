#include "EngineError.hpp"

EngineError::EngineError(const std::string &msg) : _msg(msg) {
    //
}

const char *EngineError::what() const noexcept {
    return this->_msg.c_str();
}
