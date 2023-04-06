#ifndef DEBUG_UI_RESOURCESLISTWINDOW_HPP
#define DEBUG_UI_RESOURCESLISTWINDOW_HPP

#include <memory>

#include "src/Debug/UI/WindowBase.hpp"

class ResourceDatabase;
class ResourceLoader;

class ResourcesListWindow : public WindowBase {
private:
    std::shared_ptr<ResourceDatabase> _resourceDatabase;
    std::shared_ptr<ResourceLoader> _resourceLoader;

public:
    ResourcesListWindow(const std::shared_ptr<ResourceDatabase> &resourceDatabase,
                        const std::shared_ptr<ResourceLoader> &resourceLoader);
    ~ResourcesListWindow() override = default;

    void draw(bool *visible) override;
};

#endif // DEBUG_UI_RESOURCESLISTWINDOW_HPP
