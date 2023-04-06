#ifndef RESOURCES_RESOURCEDATA_HPP
#define RESOURCES_RESOURCEDATA_HPP

#include "src/Resources/ResourceId.hpp"
#include "src/Types/DataBuffer.hpp"

class DataStream;

class ResourceData {
private:
    ResourceId _id;
    DataBuffer _data;

public:
    ResourceData(const ResourceId &id, const DataBuffer &data);

    [[nodiscard]] const ResourceId &id() const { return this->_id; }

    [[nodiscard]] const DataBuffer &data() const { return this->_data; }

    [[nodiscard]] DataStream stream();
};

#endif // RESOURCES_RESOURCEDATA_HPP
