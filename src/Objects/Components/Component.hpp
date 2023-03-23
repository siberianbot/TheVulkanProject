#ifndef OBJECTS_COMPONENTS_COMPONENT_HPP
#define OBJECTS_COMPONENTS_COMPONENT_HPP

class Component {
protected:
    bool _dirty = false;

public:
    virtual ~Component() = default;

    [[nodiscard]] bool isDirty() { return this->_dirty; }

    void resetDirty();
};

#endif // OBJECTS_COMPONENTS_COMPONENT_HPP
