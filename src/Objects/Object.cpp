#include "Object.hpp"

static uint64_t ids = 0;

Object::Object() : _id(++ids) {
    //
}

void Object::acceptEdit(const std::shared_ptr<ObjectEditVisitor> &visitor) {
    // nothing to do
}
