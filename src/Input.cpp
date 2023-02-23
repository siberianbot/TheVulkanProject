#include "Input.hpp"

void Input::addHandler(int key, const InputHandler &handler) {
    this->handlers[key] = handler;
}

void Input::press(int key) {
    this->pressed[key] = true;
}

void Input::release(int key) {
    this->pressed[key] = false;
}

void Input::process(float delta) {
    for (auto pair: this->pressed) {
        if (!pair.second) {
            continue;
        }

        auto it = this->handlers.find(pair.first);
        if (it != this->handlers.end()) {
            it->second(delta);
        }
    }
}

void MouseInput::addHandler(const MouseInputHandler &handler) {
    this->handlers.push_back(handler);
}

void MouseInput::process(double dx, double dy) {
    for (const auto &handler: this->handlers) {
        handler(dx, dy);
    }
}
