#ifndef OBJECTS_OBJECT_HPP
#define OBJECTS_OBJECT_HPP

#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>
#include <vector>
#include <string>

struct IComponent;

class Object {
private:
    uint64_t _id;

protected:
    std::vector<std::shared_ptr<IComponent>> _components;

    Object();

public:
    virtual ~Object() = default;

    [[nodiscard]] virtual std::string displayName() = 0;

    [[nodiscard]] const uint64_t &id() const { return this->_id; }
    [[nodiscard]] const std::vector<std::shared_ptr<IComponent>> &components() const { return this->_components; }

    template<typename T>
    [[nodiscard]] std::weak_ptr<T> getComponent();
};

template<typename T>
std::weak_ptr<T> Object::getComponent() {
    static_assert(std::is_base_of<IComponent, T>::value);

    auto it = std::find_if(this->_components.begin(), this->_components.end(),
                           [](std::shared_ptr<IComponent> &component) {
                               return std::dynamic_pointer_cast<T>(component) != nullptr;
                           });

    if (it == this->_components.end()) {
        return std::weak_ptr<T>();
    }

    return std::dynamic_pointer_cast<T>(*it);
}

#endif // OBJECTS_OBJECT_HPP
