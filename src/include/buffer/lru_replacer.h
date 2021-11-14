/**
 * lru_replacer.h
 *
 * Functionality: The buffer pool manager must maintain a LRU list to collect
 * all the pages that are unpinned and ready to be swapped. The simplest way to
 * implement LRU is a FIFO queue, but remember to dequeue or enqueue pages when
 * a page changes from unpinned to pinned, or vice-versa.
 */

#pragma once

#include <algorithm>
#include <chrono>

#include "buffer/replacer.h"
#include "hash/extendible_hash.h"

namespace scudb {

template <typename T>
class LRUReplacer : public Replacer<T> {
public:
  // do not change public interface
  LRUReplacer();

  ~LRUReplacer();

  void Insert(const T &value);

  bool Victim(T &value);

  bool Erase(const T &value);

  size_t Size();

private:
  // add your member variables here
  using Item = std::tuple<T, std::chrono::system_clock::time_point>;
  std::vector<Item> data;
  std::recursive_mutex mutex;

  void sort();
};

} // namespace scudb
