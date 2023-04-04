#ifndef DEBUG_UI_RESOURCESLISTWINDOW_HPP
#define DEBUG_UI_RESOURCESLISTWINDOW_HPP

#include <memory>

#include "src/Debug/UI/WindowBase.hpp"

class ResourceDatabase;

class ResourcesListWindow : public WindowBase {
private:
    std::shared_ptr<ResourceDatabase> _resourceDatabase;

public:
    ResourcesListWindow(const std::shared_ptr<ResourceDatabase> &resourceDatabase);
    ~ResourcesListWindow() override = default;

    void draw(bool *visible) override;
};

#endif // DEBUG_UI_RESOURCESLISTWINDOW_HPP
