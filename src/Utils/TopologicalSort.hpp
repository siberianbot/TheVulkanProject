#ifndef UTILS_TOPOLOGICALSORT_HPP
#define UTILS_TOPOLOGICALSORT_HPP

#include <functional>
#include <map>
#include <optional>
#include <vector>

template<typename T>
class TopologicalSort {
public:
    using InputProvider = std::function<std::vector<T>()>;
    using DependenciesProvider = std::function<std::vector<T>(const T &)>;

private:
    std::map<T, std::optional<bool>> _marked;
    std::vector<T> _result;

    InputProvider _inputProvider;
    DependenciesProvider _dependenciesProvider;

    void step(const T &item);

public:
    TopologicalSort(const InputProvider &inputProvider,
                    const DependenciesProvider &dependenciesProvider);

    void sort();
    void reset();

    [[nodiscard]] const std::vector<T> &getResult() const { return this->_result; }
};

template<typename T>
TopologicalSort<T>::TopologicalSort(const InputProvider &inputProvider,
                                    const DependenciesProvider &dependenciesProvider)
        : _inputProvider(inputProvider),
          _dependenciesProvider(dependenciesProvider) {
    //
}

template<typename T>
void TopologicalSort<T>::step(const T &item) {
    auto mark = this->_marked[item];

    if (mark == std::nullopt) {
        this->_marked[item] = false;

        for (const auto &dep: this->_dependenciesProvider(item)) {
            this->step(dep);
        }

        this->_marked[item] = true;
        this->_result.insert(this->_result.begin(), item);
    } else if (!mark.value()) {
        throw std::runtime_error("Loop detected");
    }
}

template<typename T>
void TopologicalSort<T>::reset() {
    this->_marked.clear();
    this->_result.clear();
}

template<typename T>
void TopologicalSort<T>::sort() {
    for (const auto &item: this->_inputProvider()) {
        this->step(item);
    }
}

#endif // UTILS_TOPOLOGICALSORT_HPP
