#ifndef DEBUG_UI_RESOURCESLISTWINDOW_HPP
#define DEBUG_UI_RESOURCESLISTWINDOW_HPP

#include <memory>

#include "src/Debug/UI/WindowBase.hpp"

class ResourceManager;

class ResourcesListWindow : public WindowBase {
private:
    std::shared_ptr<ResourceManager> _resourceManager;

public:
    ResourcesListWindow(const std::shared_ptr<ResourceManager> &resourceManager);
    ~ResourcesListWindow() override = default;

    void draw(bool *visible) override;
};

#endif // DEBUG_UI_RESOURCESLISTWINDOW_HPP
