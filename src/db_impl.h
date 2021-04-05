/******* rockskv *******/
/* db_impl.h
* 07/23/2019
* by Mian Qin
*/

#ifndef _db_impl_h_
#define _db_impl_h_

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include "rockskv/db.h"
#include "kvssd/kvssd.h"
#include "blockingconcurrentqueue.h"

#include "rocksdb/db.h"
#include "rocksdb/env.h"
#include "rocksdb/table.h"
#include "rocksdb/iterator.h"
#include "rocksdb/write_batch.h"
#include "rocksdb/comparator.h"
#include "rocksdb/filter_policy.h"

#include "rocksdb/convenience.h"

namespace rockskv {

class ComparatorRocks : public rocksdb::Comparator {
public:
  ComparatorRocks(const rockskv::Comparator* cmp) : cmp_(cmp) {};
  //~ComparatorRocks() {};
  int Compare(const rocksdb::Slice& a, const rocksdb::Slice& b) const {
    Slice aa(a.data(), a.size());
    Slice bb(b.data(), b.size());
    return cmp_->Compare(aa, bb);
  }
  const char* Name() const { return "Rocks.comparator"; }
  void FindShortestSeparator(
      std::string* start,
      const rocksdb::Slice& limit) const {
    // from rocksdb bytewise comparator
    // Find length of common prefix
    size_t min_length = std::min(start->size(), limit.size());
    size_t diff_index = 0;
    while ((diff_index < min_length) &&
          ((*start)[diff_index] == limit[diff_index])) {
      diff_index++;
    }

    if (diff_index >= min_length) {
      // Do not shorten if one string is a prefix of the other
    } else {
      uint8_t diff_byte = static_cast<uint8_t>((*start)[diff_index]);
      if (diff_byte < static_cast<uint8_t>(0xff) &&
          diff_byte + 1 < static_cast<uint8_t>(limit[diff_index])) {
        (*start)[diff_index]++;
        start->resize(diff_index + 1);
      }
    }
  }
  void FindShortSuccessor(std::string* key) const {
    // Find first character that can be incremented
    size_t n = key->size();
    for (size_t i = 0; i < n; i++) {
      const uint8_t byte = (*key)[i];
      if (byte != static_cast<uint8_t>(0xff)) {
        (*key)[i] = byte + 1;
        key->resize(i+1);
        return;
      }
    }
    // *key is a run of 0xffs.  Leave it alone.
  }
private:
  const rockskv::Comparator* cmp_;
};

class DBImpl : public DB{
friend class DBIterator;
public:
  DBImpl(const Options& options, const std::string& dbname);
  ~DBImpl();

  // Implementations of the DB interface
  Status Put(const WriteOptions&, const Slice& key, const Slice& value);
  Status Delete(const WriteOptions&, const Slice& key);
  Status Write(const WriteOptions& options, WriteBatch* updates);
  Status Get(const ReadOptions& options,
                     const Slice& key,
                     std::string* value);
  Iterator* NewIterator(const ReadOptions&);

  kvssd::KVSSD* GetKVSSD() {return kvd_;}
  const Comparator* GetComparator() {return options_.comparator;}

private:
  Options options_;
  kvssd::KVSSD *kvd_;

  std::string name_;
  rocksdb::DB* rdb_;
  ComparatorRocks* cmp_;

public:

};


}  // namespace rockskv



#endif
