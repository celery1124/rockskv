/******* rockskv *******/
/* db_iter.cc
* 08/06/2019
* by Mian Qin
*/
#include <mutex>
#include <iostream>
#include <condition_variable>
#include <unordered_map>
#include "rockskv/iterator.h"
#include "db_impl.h"
#include "db_iter.h"
#include "kvssd/kvssd.h"
#include <thread>
#include <atomic>

#include <ctime>
#include <chrono>

namespace rockskv {

class DBIterator : public Iterator {
public:
  DBIterator(DBImpl *db, const ReadOptions &options);
  ~DBIterator();

  bool Valid() const {
    return valid_;
  }
  void SeekToFirst();
  void SeekToLast() { /* NOT IMPLEMENT */ }
  void Seek(const Slice& target);
  void Next();
  void Prev();
  Slice key() const;
  Slice value();
private:
  DBImpl *db_;
  const ReadOptions &options_;
  // MergeIterator *it_;
  rocksdb::Iterator *it_;
  kvssd::KVSSD *kvd_;
  std::string value_;
  bool valid_;

  // upper key hint
  Slice upper_key_;
};

DBIterator::DBIterator(DBImpl *db, const ReadOptions &options) 
: db_(db), options_(options), kvd_(db->GetKVSSD()), valid_(false) {

  if (options_.upper_key != NULL) {
    upper_key_ = *(options_.upper_key);
  }
  // it_ = new MergeIterator(db, options, db->options_.indexNum);
  rocksdb::ReadOptions rd_option;
  it_ = db_->rdb_->NewIterator(rd_option);
}

DBIterator::~DBIterator() { 
  delete it_; 
}


void DBIterator::SeekToFirst() { 
  it_->SeekToFirst();
  valid_ = it_->Valid();
}

void DBIterator::Seek(const Slice& target) { 
  RecordTick(db_->options_.statistics.get(), REQ_SEEK);
  // rocksdb impl in kvssd
  rocksdb::Slice rkey(target.data(), target.size());
  it_->Seek(rkey); 
  rocksdb::Slice curr_key = it_->key();
  Slice ckey(curr_key.data(), curr_key.size());
  if (upper_key_.size() > 0 && it_->Valid() && db_->options_.comparator->Compare(ckey, upper_key_) >= 0) {
    valid_ = false;
  } else {
    valid_ = it_->Valid();
  }
  return;

}

void DBIterator::Prev() {
  assert(valid_);
  // rocksdb impl for kvssd
  it_->Prev();
  valid_ = it_->Valid();
  return;
}

void DBIterator::Next() {
  RecordTick(db_->options_.statistics.get(), REQ_NEXT);
  // rocksdb impl for kvssd
  it_->Next();
  rocksdb::Slice curr_key = it_->key();
  Slice ckey(curr_key.data(), curr_key.size());
  if (upper_key_.size() > 0 && it_->Valid() && db_->options_.comparator->Compare(ckey, upper_key_) >= 0) {
    valid_ = false;
  } else {
    valid_ = it_->Valid();
  }
  return;
}

Slice DBIterator::key() const {
  assert(valid_);
  // rocksdb impl for kvssd
  rocksdb::Slice rkey = it_->key();
  Slice ret(rkey.data(), rkey.size());
  return ret;
}

Slice DBIterator::value() {
  assert(valid_);
  // rocksdb impl for kvssd
  value_.clear();
  rocksdb::Slice vvv = it_->value();
  value_.append(vvv.data(), vvv.size());
  return Slice(value_);
}

Iterator* NewDBIterator(DBImpl *db, const ReadOptions &options) {
  return new DBIterator(db, options);
}

} // end namespace rockskv
