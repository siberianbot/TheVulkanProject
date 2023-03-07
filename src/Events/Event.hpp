#ifndef EVENTS_EVENTBASE_HPP
#define EVENTS_EVENTBASE_HPP

#include <cstdint>

class Object;
class Light;

enum EventType {
    CLOSE_REQUESTED_EVENT,
    RENDERER_RELOADING_REQUESTED_EVENT,
    VIEWPORT_RESIZED_EVENT,
    OBJECT_CREATED_EVENT,
    OBJECT_DESTROYED_EVENT,
    LIGHT_CREATED_EVENT,
    LIGHT_DESTROYED_EVENT
};

struct ViewportEvent {
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
        // VIEWPORT_RESIZED_EVENT
        ViewportEvent viewport;
    };
};

#endif // EVENTS_EVENTBASE_HPP
