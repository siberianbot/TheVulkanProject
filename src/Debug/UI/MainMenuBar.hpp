#ifndef DEBUG_UI_MAINMENUBAR_HPP
#define DEBUG_UI_MAINMENUBAR_HPP

#include <memory>

struct DebugUIState;
class EventQueue;

class MainMenuBar {
private:
    std::shared_ptr<DebugUIState> _debugUIState;
    std::shared_ptr<EventQueue> _eventQueue;

public:
    MainMenuBar(const std::shared_ptr<DebugUIState> &debugUIState,
                const std::shared_ptr<EventQueue> &eventQueue);

    void draw();
};


#endif // DEBUG_UI_MAINMENUBAR_HPP
