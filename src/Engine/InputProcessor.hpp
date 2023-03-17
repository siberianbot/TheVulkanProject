#ifndef ENGINE_INPUTPROCESSOR_HPP
#define ENGINE_INPUTPROCESSOR_HPP

#include <functional>
#include <map>
#include <memory>
#include <vector>

class EventQueue;

using InputHandler = std::function<void()>;
using CursorMoveHandler = std::function<void(double, double)>;

class InputProcessor {
private:
    std::shared_ptr<EventQueue> _eventQueue;

    std::map<int, std::vector<InputHandler>> _keyboardPressHandlers;
    std::map<int, std::vector<InputHandler>> _keyboardReleaseHandlers;
    std::map<int, std::vector<InputHandler>> _mousePressHandlers;
    std::map<int, std::vector<InputHandler>> _mouseReleaseHandlers;
    std::vector<CursorMoveHandler> _cursorMoveHandlers;

public:
    InputProcessor(const std::shared_ptr<EventQueue> &eventQueue);

    void addKeyboardPressHandler(int key, const InputHandler &handler);
    void addKeyboardReleaseHandler(int key, const InputHandler &handler);

    void addMousePressHandler(int button, const InputHandler &handler);
    void addMouseReleaseHandler(int button, const InputHandler &handler);

    void addCursorMoveHandler(const CursorMoveHandler &handler);
};

#endif // ENGINE_INPUTPROCESSOR_HPP
