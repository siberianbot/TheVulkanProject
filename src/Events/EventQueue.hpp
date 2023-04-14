#ifndef EVENTS_EVENTQUEUE_HPP
#define EVENTS_EVENTQUEUE_HPP

#include <functional>
#include <map>
#include <queue>

#include "Event.hpp"

using EventHandlerIdx = uint64_t;
using EventHandler = std::function<void(const Event &)>;

class EventQueue {
private:
    EventHandlerIdx _nextIdx = 0;
    std::queue<Event> _pendingEvents;
    std::map<EventHandlerIdx, EventHandler> _handlers;

public:
    [[nodiscard]] EventHandlerIdx addHandler(EventHandler handler);
    void removeHandler(EventHandlerIdx handlerIdx);

    void process();

    void pushEvent(const Event &event);
};

#endif // EVENTS_EVENTQUEUE_HPP
