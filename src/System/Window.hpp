#ifndef SYSTEM_SURFACE_HPP
#define SYSTEM_SURFACE_HPP

#include <memory>

struct GLFWwindow;
class EngineVars;
class EventQueue;

class Window {
private:
    std::shared_ptr<EngineVars> _engineVars;
    std::shared_ptr<EventQueue> _eventQueue;

    GLFWwindow *_handle = nullptr;
    uint32_t _width = 0;
    uint32_t _height = 0;
    double _cursorX = 0;
    double _cursorY = 0;

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void cursorCallback(GLFWwindow *window, double x, double y);
    static void closeCallback(GLFWwindow *window);

public:
    Window(const std::shared_ptr<EngineVars> &engineVars, const std::shared_ptr<EventQueue> &eventQueue);

    void create();
    void destroy();

    [[nodiscard]] GLFWwindow *handle() const { return this->_handle; }
    [[nodiscard]] uint32_t width() const { return this->_width; }
    [[nodiscard]] uint32_t height() const { return this->_height; }
    [[nodiscard]] double cursorX() const { return this->_cursorX; }
    [[nodiscard]] double cursorY() const { return this->_cursorY; }

    void hideCursor();
    void showCursor();
};

#endif // SYSTEM_SURFACE_HPP
