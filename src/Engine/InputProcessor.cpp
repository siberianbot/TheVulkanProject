#include "InputProcessor.hpp"

#include "src/Events/EventQueue.hpp"

InputProcessor::InputProcessor(const std::shared_ptr<EventQueue> &eventQueue)
        : _eventQueue(eventQueue) {
    this->_eventQueue->addHandler([this](const Event &event) {
        switch (event.type) {
            case KEYBOARD_PRESS_INPUT_EVENT: {
                for (InputHandler &handler: this->_keyboardPressHandlers[event.input().keyboardKey]) {
                    handler();
                }
            }
                break;

            case KEYBOARD_RELEASE_INPUT_EVENT:
                for (InputHandler &handler: this->_keyboardReleaseHandlers[event.input().keyboardKey]) {
                    handler();
                }
                break;

            case MOUSE_PRESS_INPUT_EVENT:
                for (InputHandler &handler: this->_mousePressHandlers[event.input().mouseButton]) {
                    handler();
                }
                break;

            case MOUSE_RELEASE_INPUT_EVENT:
                for (InputHandler &handler: this->_mouseReleaseHandlers[event.input().mouseButton]) {
                    handler();
                }
                break;

            case CURSOR_MOVE_INPUT_EVENT: {
                InputData input = event.input();
                for (CursorMoveHandler &handler: this->_cursorMoveHandlers) {
                    handler(input.cursorHorizontal, input.cursorVertical);
                }
                break;
            }

            default:
                break;
        }
    });
}

void InputProcessor::addKeyboardPressHandler(int key, const InputHandler &handler) {
    this->_keyboardPressHandlers[key].push_back(handler);
}

void InputProcessor::addKeyboardReleaseHandler(int key, const InputHandler &handler) {
    this->_keyboardReleaseHandlers[key].push_back(handler);
}

void InputProcessor::addMousePressHandler(int button, const InputHandler &handler) {
    this->_mousePressHandlers[button].push_back(handler);
}

void InputProcessor::addMouseReleaseHandler(int button, const InputHandler &handler) {
    this->_mouseReleaseHandlers[button].push_back(handler);
}

void InputProcessor::addCursorMoveHandler(const CursorMoveHandler &handler) {
    this->_cursorMoveHandlers.push_back(handler);
}
