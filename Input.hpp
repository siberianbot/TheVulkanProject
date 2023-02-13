#ifndef INPUT_HPP
#define INPUT_HPP

#include <functional>
#include <map>
#include <vector>

using InputHandler = std::function<void()>;

class Input {
private:
    std::map<int, InputHandler> handlers;
    std::map<int, bool> pressed;

public:
    void addHandler(int key, const InputHandler &handler);

    void press(int key);
    void release(int key);

    void process();
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
