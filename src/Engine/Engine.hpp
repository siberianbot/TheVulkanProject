#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <memory>
#include <vector>

class Log;
class EventQueue;
class VarCollection;
class ResourceDatabase;
class ResourceLoader;
class InputProcessor;
class GpuManager;
class Renderer;
class Window;
class SceneManager;
class DebugUIRoot;

class Engine {
private:
    std::shared_ptr<Log> _log;
    std::shared_ptr<VarCollection> _vars;
    std::shared_ptr<EventQueue> _eventQueue;
    std::shared_ptr<ResourceDatabase> _resourceDatabase;
    std::shared_ptr<ResourceLoader> _resourceLoader;
    std::shared_ptr<InputProcessor> _inputProcessor;
    std::shared_ptr<Window> _window;
    std::shared_ptr<GpuManager> _gpuManager;
    std::shared_ptr<Renderer> _renderer;
    std::shared_ptr<SceneManager> _sceneManager;
    std::shared_ptr<DebugUIRoot> _debugUI;

    volatile bool _work = false;

public:
    Engine();

    void init();
    void cleanup();

    void run();
};

#endif // ENGINE_HPP
