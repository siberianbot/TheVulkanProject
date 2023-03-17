#ifndef ENGINEVARS_HPP
#define ENGINEVARS_HPP

#include <functional>
#include <map>
#include <string>

enum VarType {
    BOOLEAN_VAR,
    INTEGER_VAR,
    FLOAT_VAR,
    STRING_VAR
};

struct Var {
    VarType type;
    union {
        bool boolValue;
        int intValue;
        float floatValue;
        char *stringValue;
    };
};

// TODO: better name required
class EngineVars {
private:
    std::map<std::string, Var *> _vars;

    Var *getOrDefault(const std::string &key, const std::function<Var *()> &createFunc);

public:
    ~EngineVars();

    Var *getOrDefault(const std::string &key, bool value);
    Var *getOrDefault(const std::string &key, int value);
    Var *getOrDefault(const std::string &key, float value);
    Var *getOrDefault(const std::string &key, char *value);

    std::map<std::string, Var *> &vars() { return this->_vars; }

    static EngineVars *defaults();
};

static constexpr const char *WINDOW_TITLE_VAR = "Window.Title";
static constexpr const char *WINDOW_WIDTH_VAR = "Window.Width";
static constexpr const char *WINDOW_HEIGHT_VAR = "Window.Height";

static constexpr const char *RENDERER_SKYBOX_ENABLED_VAR = "RENDERER_SKYBOX_ENABLED_VAR";

#endif // ENGINEVARS_HPP
