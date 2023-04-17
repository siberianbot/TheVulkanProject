#ifndef EVENTS_EVENTBASE_HPP
#define EVENTS_EVENTBASE_HPP

#include <cstdint>
#include <memory>
#include <variant>

#include "src/Resources/ResourceId.hpp"

class Object;
class Scene;
class Window;

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

    // *_RESOURCE_EVENT
    ADDED_RESOURCE_EVENT,
    REPLACED_RESOURCE_EVENT,
    REMOVED_RESOURCE_EVENT,
    LOADED_RESOURCE_EVENT,
    UNLOADED_RESOURCE_EVENT,
};

struct InputData {
    int keyboardKey;
    int mouseButton;
    double cursorHorizontal;
    double cursorVertical;
};

struct Event {
    EventType type;
    std::variant<
            std::shared_ptr<Object>,
            std::shared_ptr<Scene>,
            Window *,
            InputData,
            ResourceId
    > value;

    // *_OBJECT_EVENT
    std::shared_ptr<Object> object() const;

    // *_SCENE_EVENT
    std::shared_ptr<Scene> scene() const;

    // *_WINDOW_EVENT
    Window *window() const;

    // *_INPUT_EVENTS
    InputData input() const;

    ResourceId resourceId() const;
};

#endif // EVENTS_EVENTBASE_HPP
