#ifndef OBJECTS_COMPONENTS_COMPONENT_HPP
#define OBJECTS_COMPONENTS_COMPONENT_HPP

#include <memory>

class ObjectEditVisitor;

class Component {
protected:
    bool _dirty = false;

public:
    virtual ~Component() = default;

    [[nodiscard]] bool isDirty() { return this->_dirty; }

    void resetDirty();

    virtual void acceptEdit(const std::shared_ptr<ObjectEditVisitor> &visitor);
};

#endif // OBJECTS_COMPONENTS_COMPONENT_HPP
