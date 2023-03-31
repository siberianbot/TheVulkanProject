#ifndef DEBUG_UI_LOGWINDOW_HPP
#define DEBUG_UI_LOGWINDOW_HPP

#include <memory>

#include "src/Debug/UI/WindowBase.hpp"

class Log;

class LogWindow : public WindowBase {
private:
    std::shared_ptr<Log> _log;

public:
    LogWindow(const std::shared_ptr<Log> &log);
    ~LogWindow() override = default;

    void draw(bool *visible) override;
};


#endif // DEBUG_UI_LOGWINDOW_HPP
