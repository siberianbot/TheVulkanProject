#ifndef UTILS_CIRCULARBUFFER_HPP
#define UTILS_CIRCULARBUFFER_HPP

#include <vector>
#include <stdexcept>

template<typename T, size_t Size>
struct Iterator;

template<typename T, size_t Size>
class CircularBuffer {
private:
    std::vector<T> _buffer;
    uint32_t _idx;

public:
    CircularBuffer();

    [[nodiscard]] uint32_t idx() const { return this->_idx; }

    void push_back(const T &value);

    [[nodiscard]] T &at(uint32_t idx);

    [[nodiscard]] Iterator<T, Size> begin();
    [[nodiscard]] Iterator<T, Size> end();
};

template<typename T, size_t Size>
CircularBuffer<T, Size>::CircularBuffer() {
    this->_buffer = std::vector<T>(Size);
}

template<typename T, size_t Size>
void CircularBuffer<T, Size>::push_back(const T &value) {
    this->_buffer[this->_idx % Size] = value;
    this->_idx++;
}

template<typename T, size_t Size>
T &CircularBuffer<T, Size>::at(uint32_t idx) {
    return this->_buffer[idx % Size];
}

template<typename T, size_t Size>
struct Iterator {
    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T *;
    using reference = T &;

private:
    CircularBuffer<T, Size> *_buffer;
    uint32_t _idx;

public:
    Iterator(CircularBuffer<T, Size> *buffer, uint32_t from);

    [[nodiscard]] CircularBuffer<T, Size> *buffer() const { return this->_buffer; }
    [[nodiscard]] uint32_t idx() const { return this->_idx; }
    [[nodiscard]] reference operator*() const;
    [[nodiscard]] pointer operator->();

    Iterator &operator++();
    Iterator operator++(int);
};

template<typename T, size_t Size>
Iterator<T, Size> CircularBuffer<T, Size>::begin() {
    uint32_t idx = this->_idx < Size ? 0 : this->_idx - Size;

    return Iterator(this, idx);
}

template<typename T, size_t Size>
Iterator<T, Size> CircularBuffer<T, Size>::end() {
    return Iterator(this, this->_idx);
}

template<typename T, size_t Size>
Iterator<T, Size>::Iterator(CircularBuffer<T, Size> *buffer, uint32_t from) {
    this->_buffer = buffer;
    this->_idx = from;
}

template<typename T, size_t Size>
Iterator<T, Size>::reference Iterator<T, Size>::operator*() const {
    return this->_buffer->at(this->_idx);
}

template<typename T, size_t Size>
Iterator<T, Size>::pointer Iterator<T, Size>::operator->() {
    return &this->_buffer->at(this->_idx);
}

template<typename T, size_t Size>
Iterator<T, Size> &Iterator<T, Size>::operator++() {
    if (this->_idx >= this->_buffer->idx()) {
        throw std::runtime_error("Iterator is out of range");
    }

    this->_idx++;
    return *this;
}

template<typename T, size_t Size>
Iterator<T, Size> Iterator<T, Size>::operator++(int) {
    Iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<typename T, size_t Size>
bool operator==(const Iterator<T, Size> &a, const Iterator<T, Size> &b) {
    return a.buffer() == b.buffer() && a.idx() == b.idx();
}

template<typename T, size_t Size>
bool operator!=(const Iterator<T, Size> &a, const Iterator<T, Size> &b) {
    return !(a == b);
}

#endif // UTILS_CIRCULARBUFFER_HPP
