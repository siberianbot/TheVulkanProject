#ifndef INPUT_HPP
#define INPUT_HPP

#include <functional>
#include <map>
#include <vector>
#include <queue>

using InputHandler = std::function<void(float)>;

class Input {
private:
    enum State {
        INPUT_STATE_PRESS,
        INPUT_STATE_RELEASE
    };

    struct Action {
        State state;
        int key;
    };

    std::queue<Action> _pending;
    std::map<int, InputHandler> _pressHandlers;
    std::map<int, InputHandler> _releaseHandlers;
    std::map<int, bool> _pressed;

public:
    void addPressHandler(int key, const InputHandler &handler);
    void addReleaseHandler(int key, const InputHandler &handler);

    void press(int key);
    void release(int key);

    void process(float delta);
};

using MouseInputHandler = std::function<void(double, double)>;

class MouseInput {
private:
    std::vector<MouseInputHandler> handlers;

public:
    void addHandler(const MouseInputHandler &handler);
    void process(double dx, double dy);
};

#endif // INPUT_HPP
