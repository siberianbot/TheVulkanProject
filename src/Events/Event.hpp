#ifndef EVENTS_EVENTBASE_HPP
#define EVENTS_EVENTBASE_HPP

#include <cstdint>
#include <memory>
#include <variant>

class Object;

class Scene;

enum EventType {
    CLOSE_REQUESTED_EVENT,
    RENDERER_RELOADING_REQUESTED_EVENT,

    // *_OBJECT_EVENT
    CREATED_OBJECT_EVENT,
    DESTROYED_OBJECT_EVENT,

    // *_SCENE_EVENT
    TRANSITION_SCENE_EVENT,

    // *_WINDOW_EVENT
    RESIZE_WINDOW_EVENT,

    // *_INPUT_EVENT
    KEYBOARD_PRESS_INPUT_EVENT,
    KEYBOARD_RELEASE_INPUT_EVENT,
    MOUSE_PRESS_INPUT_EVENT,
    MOUSE_RELEASE_INPUT_EVENT,
    CURSOR_MOVE_INPUT_EVENT,
};

struct InputData {
    int keyboardKey;
    int mouseButton;
    double cursorHorizontal;
    double cursorVertical;
};

struct WindowData {
    uint32_t width;
    uint32_t height;
};

struct Event {
    EventType type;
    std::variant<
            std::shared_ptr<Object>,
            std::shared_ptr<Scene>,
            WindowData,
            InputData
    > value;

    // *_OBJECT_EVENT
    std::shared_ptr<Object> object() const;

    // *_SCENE_EVENT
    std::shared_ptr<Scene> scene() const;

    // *_WINDOW_EVENT
    WindowData window() const;

    // *_INPUT_EVENTS
    InputData input() const;
};

#endif // EVENTS_EVENTBASE_HPP
