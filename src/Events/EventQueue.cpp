#include "EventQueue.hpp"

void EventQueue::addHandler(EventHandler handler) {
    this->_handlers.push_front(handler);
}

void EventQueue::process() {
    std::queue<Event> events;

    while (!this->_pendingEvents.empty()) {
        events.push(this->_pendingEvents.front());
        this->_pendingEvents.pop();
    }

    while (!events.empty()) {
        Event event = events.front();

        for (const EventHandler &handler: this->_handlers) {
            handler(event);
        }

        events.pop();
    }
}

void EventQueue::pushEvent(const Event &event) {
    this->_pendingEvents.push(event);
}
