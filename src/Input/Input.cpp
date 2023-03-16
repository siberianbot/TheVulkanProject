#include "Input.hpp"

void Input::addPressHandler(int key, const InputHandler &handler) {
    this->_pressHandlers[key] = handler;
}

void Input::addReleaseHandler(int key, const InputHandler &handler) {
    this->_releaseHandlers[key] = handler;
}

void Input::press(int key) {
    this->_pending.push(Action{.state = INPUT_STATE_PRESS, .key = key});
}

void Input::release(int key) {
    this->_pending.push(Action{.state = INPUT_STATE_RELEASE, .key = key});
}

void Input::process(float delta) {
    std::vector<int> releasedKeys;

    while (!this->_pending.empty()) {
        Action action = this->_pending.front();

        switch (action.state) {
            case INPUT_STATE_PRESS:
                this->_pressed[action.key] = true;
                break;

            case INPUT_STATE_RELEASE:
                this->_pressed[action.key] = false;
                releasedKeys.push_back(action.key);
                break;
        }

        this->_pending.pop();
    }

    for (const auto &item: this->_pressed) {
        if (item.second) {
            auto func = this->_pressHandlers.find(item.first);

            if (func != this->_pressHandlers.end()) {
                func->second(delta);
            }
        } else if (std::find(releasedKeys.begin(), releasedKeys.end(), item.first) != releasedKeys.end()) {
            auto func = this->_releaseHandlers.find(item.first);

            if (func != this->_releaseHandlers.end()) {
                func->second(delta);
            }
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
