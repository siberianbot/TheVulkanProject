#include "Event.hpp"

std::shared_ptr<Object> Event::object() const {
    return std::get<std::shared_ptr<Object>>(this->value);
}

std::shared_ptr<Scene> Event::scene() const {
    return std::get<std::shared_ptr<Scene>>(this->value);
}

WindowData Event::window() const {
    return std::get<WindowData>(this->value);
}

InputData Event::input() const {
    return std::get<InputData>(this->value);
}
