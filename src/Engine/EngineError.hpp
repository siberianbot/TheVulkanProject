#ifndef ENGINE_ENGINEERROR_HPP
#define ENGINE_ENGINEERROR_HPP

#include <exception>
#include <string>

class EngineError : public std::exception {
private:
    std::string _msg;

public:
    EngineError(const std::string &msg);
    ~EngineError() override = default;

    const char *what() const noexcept override;
};


#endif // ENGINE_ENGINEERROR_HPP
