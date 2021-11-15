#include "buffer/buffer_pool_manager.h"

namespace scudb {

/*
 * BufferPoolManager Constructor
 * When log_manager is nullptr, logging is disabled (for test purpose)
 * WARNING: Do Not Edit This Function
 */
BufferPoolManager::BufferPoolManager(size_t pool_size,
                                     DiskManager *disk_manager,
                                     LogManager *log_manager)
    : pool_size_(pool_size), disk_manager_(disk_manager),
      log_manager_(log_manager) {
  // a consecutive memory space for buffer pool
  pages_ = new Page[pool_size_];
  page_table_ = new ExtendibleHash<page_id_t, Page *>(BUCKET_SIZE);
  replacer_ = new LRUReplacer<Page *>;
  free_list_ = new std::list<Page *>;

  // put all the pages into free list
  for (size_t i = 0; i < pool_size_; ++i) {
    free_list_->push_back(&pages_[i]);
  }
}

/*
 * BufferPoolManager Deconstructor
 * WARNING: Do Not Edit This Function
 */
BufferPoolManager::~BufferPoolManager() {
  delete[] pages_;
  delete page_table_;
  delete replacer_;
  delete free_list_;
}

/**
 * 1. search hash table.
 *  1.1 if exist, pin the page and return immediately
 *  1.2 if no exist, find a replacement entry from either free list or lru
 *      replacer. (NOTE: always find from free list first)
 * 2. If the entry chosen for replacement is dirty, write it back to disk.
 * 3. Delete the entry for the old page from the hash table and insert an
 * entry for the new page.
 * 4. Update page metadata, read page content from disk file and return page
 * pointer
 */
Page *BufferPoolManager::FetchPage(page_id_t page_id) {
  std::lock_guard<std::recursive_mutex> lock(latch_);
  Page *page = nullptr;
  if (page_table_->Find(page_id, page)) {
    // 1.1
    page->pin_count_++;
    return page;
  }
  // 1.2
  if (!free_list_->empty()) {
    page = free_list_->back();
    free_list_->pop_back();
  } else {
    if (!replacer_->Victim(page)) {
      throw std::runtime_error("");
    }
  }
  // 2
  if (page->is_dirty_) {
    FlushPage(page->GetPageId());
  }
  // 3
  page_table_->Remove(page->GetPageId());
  page_table_->Insert(page_id, page);
  // 4
  page->page_id_ = page_id;
  page->pin_count_ = 1;
  page->is_dirty_ = false;
  page->WLatch();
  disk_manager_->ReadPage(page->GetPageId(), page->GetData());
  page->WUnlatch();
  return page;
}

/*
 * Implementation of unpin page
 * if pin_count>0, decrement it and if it becomes zero, put it back to
 * replacer if pin_count<=0 before this call, return false. is_dirty: set the
 * dirty flag of this page
 */
bool BufferPoolManager::UnpinPage(page_id_t page_id, bool is_dirty) {
  std::lock_guard<std::recursive_mutex> lock(latch_);
  Page *page;
  if (page_table_->Find(page_id, page)) {
    page->is_dirty_ |= is_dirty;
    if (page->pin_count_ > 0) {
      page->pin_count_--;
      if (page->pin_count_ == 0) {
        replacer_->Insert(page);
      }
      return true;
    } else {
      return false;
    }
  }
  return false;
}

/*
 * Used to flush a particular page of the buffer pool to disk. Should call the
 * write_page method of the disk manager
 * if page is not found in page table, return false
 * NOTE: make sure page_id != INVALID_PAGE_ID
 */
bool BufferPoolManager::FlushPage(page_id_t page_id) {
  std::lock_guard<std::recursive_mutex> lock(latch_);
  if (page_id == INVALID_PAGE_ID) {
    throw std::runtime_error("INVALID_PAGE_ID");
  }
  Page *page;
  if (page_table_->Find(page_id, page)) {
    if (page->is_dirty_) {
      disk_manager_->WritePage(page->GetPageId(), page->data_);
    }
    return true;
  }
  return false;
}

void BufferPoolManager::FlushAllPages() {
  std::lock_guard<std::recursive_mutex> lock(latch_);
  for (size_t i = 0; i < pool_size_; ++i) {
    disk_manager_->WritePage(pages_[i].GetPageId(), pages_[i].GetData());
  }
}

/**
 * User should call this method for deleting a page. This routine will call
 * disk manager to deallocate the page. First, if page is found within page
 * table, buffer pool manager should be reponsible for removing this entry out
 * of page table, reseting page metadata and adding back to free list. Second,
 * call disk manager's DeallocatePage() method to delete from disk file. If
 * the page is found within page table, but pin_count != 0, return false
 */
bool BufferPoolManager::DeletePage(page_id_t page_id) {
  std::lock_guard<std::recursive_mutex> lock(latch_);
  Page *page;
  if (page_table_->Find(page_id, page)) {
    // remove from table
    if (!page_table_->Remove(page_id)) {
      throw std::runtime_error("");
    }
    if (page->pin_count_ != 0) {
      return false;
    }
    // reset meta
    page->page_id_ = 0;
    page->is_dirty_ = false;
    page->pin_count_ = 0;
    page->ResetMemory();
    // add to freelist
    free_list_->push_back(page);
    // dealloc
    disk_manager_->DeallocatePage(page_id);
    return true;
  }
  return false;
}

/**
 * User should call this method if needs to create a new page. This routine
 * will call disk manager to allocate a page.
 * Buffer pool manager should be responsible to choose a victim page either
 * from free list or lru replacer(NOTE: always choose from free list first),
 * update new page's metadata, zero out memory and add corresponding entry
 * into page table. return nullptr if all the pages in pool are pinned
 */
Page *BufferPoolManager::NewPage(page_id_t &page_id) {
  std::lock_guard<std::recursive_mutex> lock(latch_);
  Page *page;
  page_id = disk_manager_->AllocatePage();
  if (!free_list_->empty()) {
    page = free_list_->back();
    free_list_->pop_back();
  } else {
    if (!replacer_->Victim(page)) {
      return nullptr;
    }
  }
  if (page->is_dirty_) {
    FlushPage(page->GetPageId());
  }
  page_table_->Remove(page->GetPageId());
  // update meta
  page->page_id_ = page_id;
  page->is_dirty_ = false;
  page->pin_count_ = 1;
  page->ResetMemory();
  page_table_->Insert(page_id, page);
  return page;
}

} // namespace scudb
