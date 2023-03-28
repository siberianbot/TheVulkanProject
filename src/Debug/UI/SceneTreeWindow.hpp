#ifndef DEBUG_UI_SCENETREEWINDOW_HPP
#define DEBUG_UI_SCENETREEWINDOW_HPP

#include <memory>

#include "src/Debug/UI/WindowBase.hpp"

class SceneManager;
class SceneNode;

class SceneTreeWindow : public WindowBase {
private:
    std::shared_ptr<SceneManager> _sceneManager;

    std::weak_ptr<SceneNode> _selectedNode;

    void renderItem(const std::shared_ptr<SceneNode> &node);

public:
    SceneTreeWindow(const std::shared_ptr<SceneManager> &sceneManager);
    ~SceneTreeWindow() override = default;

    void draw(bool *visible) override;
};


#endif // DEBUG_UI_SCENETREEWINDOW_HPP
