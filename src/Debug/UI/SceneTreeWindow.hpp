#ifndef DEBUG_UI_SCENETREEWINDOW_HPP
#define DEBUG_UI_SCENETREEWINDOW_HPP

#include <memory>

#include "src/Debug/UI/WindowBase.hpp"

struct DebugUIState;
class SceneManager;
class SceneNode;

class SceneTreeWindow : public WindowBase {
private:
    std::shared_ptr<DebugUIState> _debugUIState;
    std::shared_ptr<SceneManager> _sceneManager;

    std::weak_ptr<SceneNode> _selectedNode;

    std::shared_ptr<SceneNode> drawItem(const std::shared_ptr<SceneNode> &node);

public:
    SceneTreeWindow(const std::shared_ptr<DebugUIState> &debugUIState,
                    const std::shared_ptr<SceneManager> &sceneManager);
    ~SceneTreeWindow() override = default;

    void draw(bool *visible) override;
};


#endif // DEBUG_UI_SCENETREEWINDOW_HPP
