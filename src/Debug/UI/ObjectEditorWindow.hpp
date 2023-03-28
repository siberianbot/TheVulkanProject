#ifndef DEBUG_UI_OBJECTEDITORWINDOW_HPP
#define DEBUG_UI_OBJECTEDITORWINDOW_HPP

#include <memory>

#include "src/Debug/UI/WindowBase.hpp"

struct DebugUIState;
class ObjectEditVisitor;
class ResourceManager;

class ObjectEditorWindow : public WindowBase {
private:
    std::shared_ptr<DebugUIState> _debugUIState;
    std::shared_ptr<ObjectEditVisitor> _objectEditVisitor;

public:
    ObjectEditorWindow(const std::shared_ptr<DebugUIState> &debugUIState,
                       const std::shared_ptr<ResourceManager> &resourceManager);
    ~ObjectEditorWindow() override = default;

    void draw(bool *visible) override;
};


#endif // DEBUG_UI_OBJECTEDITORWINDOW_HPP
