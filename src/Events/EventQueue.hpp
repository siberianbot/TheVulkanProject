#ifndef EVENTS_EVENTQUEUE_HPP
#define EVENTS_EVENTQUEUE_HPP

#include <functional>
#include <forward_list>
#include <queue>

#include "Event.hpp"

using EventHandler = std::function<void(const Event &)>;

class EventQueue {
private:
    std::queue<Event> _pendingEvents;
    std::forward_list<EventHandler> _handlers;

public:
    void addHandler(EventHandler handler);

    void process();

    void pushEvent(const Event &event);
};

#endif // EVENTS_EVENTQUEUE_HPP
