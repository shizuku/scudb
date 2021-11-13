#include <list>

#include "hash/extendible_hash.h"
#include "page/page.h"

namespace scudb {

/*
 * constructor
 * array_size: fixed array bucket_size for each bucket
 */
template <typename K, typename V>
ExtendibleHash<K, V>::ExtendibleHash(size_t bucket_size)
    : global_depth{1}, bucket_size{bucket_size} {
  buckets.push_back(std::make_shared<Bucket>(bucket_size));
  buckets.push_back(std::make_shared<Bucket>(bucket_size));
}

/*
 * helper function to calculate the hashing address of input key
 */
template <typename K, typename V>
size_t ExtendibleHash<K, V>::HashKey(const K &key) {
  return (size_t)key;
}
template <>
size_t ExtendibleHash<page_id_t, Page *>::HashKey(const page_id_t &key) {
  return ((size_t)key & 0x00000000ffffffff) | (((size_t)key << 32) & 0xffffffff00000000);
}
template <>
size_t ExtendibleHash<Page *, std::list<Page *>::iterator>::HashKey(Page *const &key) {
  return (size_t)key & 0x00000000ffffffff;
}
template <>
size_t ExtendibleHash<int, std::string>::HashKey(const int &key) {
  return ((size_t)key & 0x00000000ffffffff) | (((size_t)key << 32) & 0xffffffff00000000);
}
template <>
size_t ExtendibleHash<int, std::list<int>::iterator>::HashKey(const int &key) {
  return ((size_t)key & 0x00000000ffffffff) | (((size_t)key << 32) & 0xffffffff00000000);
}
template <>
size_t ExtendibleHash<int, int>::HashKey(const int &key) {
  return ((size_t)key & 0x00000000ffffffff) | (((size_t)key << 32) & 0xffffffff00000000);
}

/*
 * helper function to return global depth of hash table
 * NOTE: you must implement this function in order to pass test
 */
template <typename K, typename V>
int ExtendibleHash<K, V>::GetGlobalDepth() const {
  return global_depth;
}

/*
 * helper function to return local depth of one specific bucket
 * NOTE: you must implement this function in order to pass test
 */
template <typename K, typename V>
int ExtendibleHash<K, V>::GetLocalDepth(int bucket_id) const {
  return buckets[bucket_id]->local_depth;
}

/*
 * helper function to return current number of bucket in hash table
 */
template <typename K, typename V>
int ExtendibleHash<K, V>::GetNumBuckets() const {
  return buckets.size();
}

/*
 * lookup function to find value associate with input key
 */
template <typename K, typename V>
bool ExtendibleHash<K, V>::Find(const K &key, V &value) {
  auto hk = HashKey(key);
  size_t global_key = hk & (((size_t)1 << global_depth) - 1);
  return buckets[global_key]->data.find(key) != buckets[global_key]->data.end();
}

/*
 * delete <key,value> entry in hash table
 * Shrink & Combination is not required for this project
 */
template <typename K, typename V>
bool ExtendibleHash<K, V>::Remove(const K &key) {
  auto hk = HashKey(key);
  size_t global_key = hk & (((size_t)1 << global_depth) - 1);
  auto p = buckets[global_key]->data.find(key);
  if (p == buckets[global_key]->data.end()) {
    return false;
  } else {
    buckets[global_key]->data.erase(p);
    return true;
  }
}

/*
 * insert <key,value> entry in hash table
 * Split & Redistribute bucket when there is overflow and if necessary increase
 * global depth
 */
template <typename K, typename V>
void ExtendibleHash<K, V>::Insert(const K &key, const V &value) {
  auto refactor = [&](size_t bucket_id) {
    std::map<K, V> tmp{};
    tmp.swap(buckets[bucket_id]->data);
    buckets[bucket_id] = std::make_shared<Bucket>(bucket_size, global_depth);
    for (auto i = tmp.begin(); i != tmp.end(); ++i) {
      Insert(i->first, i->second);
    }
  };
  auto expand = [&](size_t bucket_id) {
    global_depth++;
    // add new buckets:
    size_t cur_bu_sz = buckets.size();
    for (size_t i = 0; i < cur_bu_sz; i++) {
      if (i != bucket_id) {
        buckets.push_back(buckets[i]);
      } else {
        buckets.push_back(std::make_shared<Bucket>(bucket_size, global_depth));
      }
    }
    refactor(bucket_id);
  };
  auto hk = HashKey(key);
  size_t bucket_id = hk & (((size_t)1 << global_depth) - 1);
  if (buckets[bucket_id]->data.size() >= bucket_size) {
    if (buckets[bucket_id]->local_depth == global_depth) {
      expand(bucket_id);
    } else {

    }
  }
  buckets[bucket_id]->data.insert({key, value});
}

template class ExtendibleHash<page_id_t, Page *>;
template class ExtendibleHash<Page *, std::list<Page *>::iterator>;
// test purpose
template class ExtendibleHash<int, std::string>;
template class ExtendibleHash<int, std::list<int>::iterator>;
template class ExtendibleHash<int, int>;

template <typename K, typename V>
ExtendibleHash<K, V>::Bucket::Bucket(size_t size)
    : size{size}, local_depth{0} {
  while (size > 0) {
    size >>= 2;

    local_depth++;
  }
}

template <typename K, typename V>
ExtendibleHash<K, V>::Bucket::Bucket(size_t size, int local_depth)
    : size{size}, local_depth{local_depth} {}

} // namespace scudb
