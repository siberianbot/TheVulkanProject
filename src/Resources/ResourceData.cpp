#include "ResourceData.hpp"

#include "src/Utils/DataStream.hpp"

ResourceData::ResourceData(const ResourceId &id, const std::vector<char> &data)
        : _id(id),
          _data(data) {
    //
}

DataStream ResourceData::stream() {
    return DataStream(this->_data);
}
