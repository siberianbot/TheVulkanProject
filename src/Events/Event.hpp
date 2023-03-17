#ifndef EVENTS_EVENTBASE_HPP
#define EVENTS_EVENTBASE_HPP

#include <cstdint>

class Object;
class Light;
class Scene;

enum EventType {
    CLOSE_REQUESTED_EVENT,
    RENDERER_RELOADING_REQUESTED_EVENT,
    OBJECT_CREATED_EVENT,
    OBJECT_DESTROYED_EVENT,
    LIGHT_CREATED_EVENT,
    LIGHT_DESTROYED_EVENT,
    SCENE_TRANSITION_EVENT,

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
    union {
        // OBJECT_CREATED, OBJECT_DESTROYED
        Object *object;
        // LIGHT_CREATED, LIGHT_DESTROYED
        Light *light;
        // SCENE_TRANSITION_EVENT
        Scene *scene;

        // *_WINDOW_EVENT
        WindowData window;
        // *_INPUT_EVENT
        InputData input;
    };
};

#endif // EVENTS_EVENTBASE_HPP
