#ifndef ENGINE_VARCOLLECTION_HPP
#define ENGINE_VARCOLLECTION_HPP

#include <map>
#include <optional>
#include <string>
#include <string_view>
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

    [[deprecated]] bool getOrDefault(const std::string &key, const bool &defaultValue);
    [[deprecated]] int getOrDefault(const std::string &key, const int &defaultValue);
    [[deprecated]] float getOrDefault(const std::string &key, const float &defaultValue);
    [[deprecated]] std::string getOrDefault(const std::string &key, const char *defaultValue);
    [[deprecated]] std::string getOrDefault(const std::string &key, const std::string &defaultValue);

    [[nodiscard]] int32_t getIntOrDefault(const std::string_view &key, const int32_t &defaultValue);

    [[nodiscard]] VarMap &vars() { return this->_vars; }
};

#endif // ENGINE_VARCOLLECTION_HPP
