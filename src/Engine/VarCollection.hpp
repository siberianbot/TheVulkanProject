#ifndef ENGINE_VARCOLLECTION_HPP
#define ENGINE_VARCOLLECTION_HPP

#include <map>
#include <optional>
#include <string>
#include <variant>

using VarMap = std::map<std::string, std::variant<bool, int, float, std::string>>;

class VarCollection {
private:
    VarMap _vars;

    template<typename T>
    std::optional<T> get(const std::string &key) {
        auto it = this->_vars.find(key);

        if (it == this->_vars.end()) {
            return std::nullopt;
        }

        if (std::get_if<T>(&it->second) == nullptr) {
            return std::nullopt;
        }

        return std::get<T>(it->second);
    }

public:
    void set(const std::string &key, const bool &value);
    void set(const std::string &key, const int &value);
    void set(const std::string &key, const float &value);
    void set(const std::string &key, const char *value);
    void set(const std::string &key, const std::string &value);

    bool getOrDefault(const std::string &key, const bool &defaultValue);
    int getOrDefault(const std::string &key, const int &defaultValue);
    float getOrDefault(const std::string &key, const float &defaultValue);
    std::string getOrDefault(const std::string &key, const char *defaultValue);
    std::string getOrDefault(const std::string &key, const std::string &defaultValue);

    VarMap &vars() { return this->_vars; }
};

#endif // ENGINE_VARCOLLECTION_HPP
