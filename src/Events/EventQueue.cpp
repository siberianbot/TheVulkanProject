#include "EventQueue.hpp"

EventHandlerIdx EventQueue::addHandler(EventHandler handler) {
    EventHandlerIdx idx = this->_nextIdx;

    this->_handlers.emplace(idx, handler);
    this->_nextIdx++;

    return idx;
}

void EventQueue::removeHandler(EventHandlerIdx handlerIdx) {
    this->_handlers.erase(handlerIdx);
}

void EventQueue::process() {
    std::queue<Event> events;

    while (!this->_pendingEvents.empty()) {
        events.push(this->_pendingEvents.front());
        this->_pendingEvents.pop();
    }

    while (!events.empty()) {
        Event event = events.front();

        for (const auto &[idx, handler]: this->_handlers) {
            handler(event);
        }

        events.pop();
    }
}

void EventQueue::pushEvent(const Event &event) {
    this->_pendingEvents.push(event);
}
