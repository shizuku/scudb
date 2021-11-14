/*
 * extendible_hash.h : implementation of in-memory hash table using extendible
 * hashing
 *
 * Functionality: The buffer pool manager must maintain a page table to be able
 * to quickly map a PageId to its corresponding memory location; or alternately
 * report that the PageId does not match any currently-buffered page.
 */

#pragma once

#include <cstdlib>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "hash/hash_table.h"

namespace scudb {

template <typename K, typename V>
class ExtendibleHash : public HashTable<K, V> {
public:
  // constructor
  explicit ExtendibleHash(size_t bucket_size);
  // helper function to generate hash addressing
  static size_t HashKey(const K &key);
  // helper function to get global & local depth
  int GetGlobalDepth() const;
  int GetLocalDepth(int bucket_id) const;
  int GetNumBuckets() const;
  // lookup and modifier
  bool Find(const K &key, V &value) override;
  bool Remove(const K &key) override;
  void Insert(const K &bucket_id, const V &value) override;

private:
  void refactor(size_t bucket_id);
  void expand(size_t bucket_id);

  struct Bucket {
    explicit Bucket(size_t size);
    Bucket(size_t size, int local_depth);
    size_t size;
    int local_depth;
    std::map<K, V> data;
  };

  // add your own member variables here
  int global_depth;
  size_t bucket_size;
  std::vector<std::shared_ptr<Bucket>> buckets;
  std::recursive_mutex w_mutex;
  std::recursive_mutex r_mutex;
};

} // namespace scudb
