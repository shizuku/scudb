/**
 * LRU implementation
 */
#include "buffer/lru_replacer.h"
#include "page/page.h"

namespace scudb {

template <typename T>
LRUReplacer<T>::LRUReplacer() {}

template <typename T>
LRUReplacer<T>::~LRUReplacer() {}

/*
 * Insert value into LRU
 */
template <typename T>
void LRUReplacer<T>::Insert(const T &value) {
  std::lock_guard<std::recursive_mutex> lock(mutex);
  auto p = std::find_if(data.begin(), data.end(), [&](const Item &i) {
    return std::get<0>(i) == value;
  });
  if (p == data.end()) {
    data.push_back(std::make_tuple(value, std::chrono::system_clock::now()));
  } else {
    std::get<1>(*p) = std::chrono::system_clock::now();
  }
  sort();
}

/* If LRU is non-empty, pop the head member from LRU to argument "value", and
 * return true. If LRU is empty, return false
 */
template <typename T>
bool LRUReplacer<T>::Victim(T &value) {
  std::lock_guard<std::recursive_mutex> lock(mutex);
  if (data.empty()) {
    return false;
  }
  value = std::get<0>(data.back());
  data.pop_back();
  return true;
}

/*
 * Remove value from LRU. If removal is successful, return true, otherwise
 * return false
 */
template <typename T>
bool LRUReplacer<T>::Erase(const T &value) {
  std::lock_guard<std::recursive_mutex> lock(mutex);
  auto p = std::find_if(data.begin(), data.end(), [&](const Item &i) {
    return std::get<0>(i) == value;
  });
  if (p == data.end()) {
    return false;
  }
  data.erase(p);
  sort();
  return true;
}

template <typename T>
size_t LRUReplacer<T>::Size() { return data.size(); }

template <typename T>
void LRUReplacer<T>::sort() {
  std::lock_guard<std::recursive_mutex> lock(mutex);
  std::sort(data.begin(), data.end(), [](const Item &a, const Item &b) {
    return std::get<1>(a) > std::get<1>(b);
  });
}

template class LRUReplacer<Page *>;
// test only
template class LRUReplacer<int>;

} // namespace scudb
