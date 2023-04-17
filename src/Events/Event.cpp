#include "Event.hpp"

std::shared_ptr<Object> Event::object() const {
    return std::get<std::shared_ptr<Object>>(this->value);
}

std::shared_ptr<Scene> Event::scene() const {
    return std::get<std::shared_ptr<Scene>>(this->value);
}

Window *Event::window() const {
    return std::get<Window *>(this->value);
}

InputData Event::input() const {
    return std::get<InputData>(this->value);
}

ResourceId Event::resourceId() const {
    return std::get<ResourceId>(this->value);
}
