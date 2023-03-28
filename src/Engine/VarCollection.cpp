#include "VarCollection.hpp"

void VarCollection::set(const std::string &key, const bool &value) {
    this->_vars[key] = value;
}

void VarCollection::set(const std::string &key, const int &value) {
    this->_vars[key] = value;
}

void VarCollection::set(const std::string &key, const float &value) {
    this->_vars[key] = value;
}

void VarCollection::set(const std::string &key, const char *value) {
    this->_vars[key] = std::string(value);
}

void VarCollection::set(const std::string &key, const std::string &value) {
    this->_vars[key] = value;
}

bool VarCollection::getOrDefault(const std::string &key, const bool &defaultValue) {
    return this->get<bool>(key).value_or(defaultValue);
}

int VarCollection::getOrDefault(const std::string &key, const int &defaultValue) {
    return this->get<int>(key).value_or(defaultValue);
}

float VarCollection::getOrDefault(const std::string &key, const float &defaultValue) {
    return this->get<float>(key).value_or(defaultValue);
}

std::string VarCollection::getOrDefault(const std::string &key, const char *defaultValue) {
    return this->get<std::string>(key).value_or(defaultValue);
}

std::string VarCollection::getOrDefault(const std::string &key, const std::string &defaultValue) {
    return this->get<std::string>(key).value_or(defaultValue);
}
