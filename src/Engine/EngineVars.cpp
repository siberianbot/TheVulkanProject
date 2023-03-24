#include "EngineVars.hpp"

EngineVars::~EngineVars() {
    for (auto &[key, value]: this->_vars) {
        delete value;
    }

    this->_vars.clear();
}

Var *EngineVars::getOrDefault(const std::string &key, const std::function<Var *()> &createFunc) {
    Var *var = this->_vars.contains(key)
               ? this->_vars[key]
               : nullptr;

    if (var == nullptr) {
        var = createFunc();
        this->_vars[key] = var;
    }

    return var;
}

Var *EngineVars::getOrDefault(const std::string &key, bool value) {
    return this->getOrDefault(key, [&value]() {
        return new Var{
                .type = BOOLEAN_VAR,
                .boolValue = value
        };
    });
}

Var *EngineVars::getOrDefault(const std::string &key, int value) {
    return this->getOrDefault(key, [&value]() {
        return new Var{
                .type = INTEGER_VAR,
                .intValue = value
        };
    });
}

Var *EngineVars::getOrDefault(const std::string &key, float value) {
    return this->getOrDefault(key, [&value]() {
        return new Var{
                .type = FLOAT_VAR,
                .floatValue = value
        };
    });
}

Var *EngineVars::getOrDefault(const std::string &key, char *value) {
    return this->getOrDefault(key, [&value]() {
        return new Var{
                .type = STRING_VAR,
                .stringValue = value
        };
    });
}

EngineVars *EngineVars::defaults() {
    EngineVars *vars = new EngineVars();

    return vars;
}
