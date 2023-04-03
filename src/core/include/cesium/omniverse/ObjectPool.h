#pragma once

#include <deque>
#include <memory>

namespace cesium::omniverse {

template <typename T> class ObjectPool {
  public:
    ObjectPool::ObjectPool() {}

    std::shared_ptr<T> acquire() {
        if (_available.empty()) {
            return nullptr;
        }

        const auto item = _available.back();
        _available.pop_back();
        setActive(item, true);
        return item;
    }

    void release(std::shared_ptr<T> item) {
        _available.emplace_back(item);
        setActive(item, false);
    }

  protected:
    void add(std::shared_ptr<T> item) {
        _available.emplace_back(item);
    }

    virtual void setActive(std::shared_ptr<T> prim, [[maybe_unused]] bool active) {}

  private:
    std::deque<std::shared_ptr<T>> _available;
};

} // namespace cesium::omniverse
