#ifndef DEBUG_UI_WINDOWBASE_HPP
#define DEBUG_UI_WINDOWBASE_HPP

class WindowBase {
public:
    virtual ~WindowBase() = default;

    virtual void draw(bool *visible) = 0;
};

#endif // DEBUG_UI_WINDOWBASE_HPP
