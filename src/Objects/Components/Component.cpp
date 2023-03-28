#include "Component.hpp"

void Component::resetDirty() {
    this->_dirty = false;
}

void Component::acceptEdit(const std::shared_ptr<ObjectEditVisitor> &visitor) {
    // nothing to do
}
