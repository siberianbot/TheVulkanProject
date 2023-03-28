#ifndef DEBUG_UI_VARIABLESWINDOW_HPP
#define DEBUG_UI_VARIABLESWINDOW_HPP

#include <map>
#include <memory>
#include <string>

#include "src/Debug/UI/WindowBase.hpp"

class VarCollection;

class VariablesWindow : public WindowBase {
private:
    std::shared_ptr<VarCollection> _vars;
    std::map<std::string, bool> _varEditState;

public:
    VariablesWindow(const std::shared_ptr<VarCollection> &vars);
    ~VariablesWindow() override = default;

    void draw(bool *visible) override;
};

#endif // DEBUG_UI_VARIABLESWINDOW_HPP
