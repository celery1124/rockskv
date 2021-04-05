/******* rockskv *******/
/* db_impl.cc
* 07/23/2019
* by Mian Qin
*/
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <chrono>
#include "rockskv/db.h"
#include "rockskv/iterator.h"
#include "rockskv/write_batch.h"
#include "db_impl.h"
#include "db_iter.h"
#include "hash.h"

// meant to measure the benefit of Hot query acceleration
// extern int hitCnt;
// extern double hitCost;
// extern double missCost;
// extern double hitNextCost;
// extern double missNextCost;
namespace rockskv {

// WriteBatch definition
WriteBatch::WriteBatch() {}
WriteBatch::~WriteBatch() {}

void WriteBatch::Put(const Slice& key, const Slice& value) {
    batch_.push_back(std::make_pair(key.ToString(), value.ToString()));
}
void WriteBatch::Delete(const Slice& key) {
    batch_.push_back(std::make_pair(key.ToString(), std::string()));
}
void WriteBatch::Clear() {
    batch_.clear();
}
int WriteBatch::Size() {
    return batch_.size();
}

DBImpl::DBImpl(const Options& options, const std::string& dbname) 
: options_(options),
  name_(dbname) {
  
  // setup stats dump
  options_.statistics.get()->setStatsDump(options_.stats_dump_interval);
  
  // rocksdb option
  rocksdb::Options rdb_options;
  rdb_options.IncreaseParallelism();
  // rdb_options.OptimizeLevelStyleCompaction();
  rdb_options.create_if_missing = true;
  rdb_options.max_open_files = 1000;
  rdb_options.compression = rocksdb::kNoCompression;
  rdb_options.paranoid_checks = false;
  rdb_options.level0_slowdown_writes_trigger = 16;
  rdb_options.level0_stop_writes_trigger = 10;
  rdb_options.write_buffer_size = 128 << 20;
  rdb_options.target_file_size_base = 64 * 1048576;
  rdb_options.max_bytes_for_level_base = options_.levelBaseSize;

  rocksdb::BlockBasedTableOptions table_options;
  //table_options.filter_policy.reset(rocksdb::NewBloomFilterPolicy(16, false));
  table_options.block_size = options_.blockSize;
  // table_options.cache_index_and_filter_blocks = true;
  if (options_.blockCacheSize > 0)
    table_options.block_cache = rocksdb::NewLRUCache((size_t)options_.blockCacheSize * 1024 * 1024LL); 
  else {
    table_options.no_block_cache = true;
  }
  rdb_options.table_factory.reset(rocksdb::NewBlockBasedTableFactory(table_options));

  // apply db options
  cmp_ = new ComparatorRocks(options_.comparator);
  rdb_options.comparator = cmp_;

  rdb_options.env = rocksdb::NewKVEnvOpt(rocksdb::Env::Default(), kvd_);
  rocksdb::Status status = rocksdb::DB::Open(rdb_options, name_, &rdb_);
  if (status.ok()) printf("rocksdb open ok\n");
  else printf("rocksdb open error\n");

  }

DBImpl::~DBImpl() {
  rocksdb::CancelAllBackgroundWork(rdb_, true);
  delete rdb_;
  delete cmp_;
	delete kvd_;
}


Status DBImpl::Put(const WriteOptions& options,
                     const Slice& key,
                     const Slice& value) {
  RecordTick(options_.statistics.get(), REQ_PUT);
  // rocksdb impl in kvssd

  rocksdb::WriteOptions wopts;
  wopts.disableWAL = true;
  rocksdb::Slice put_key(key.data(), key.size());
  rocksdb::Slice put_val(value.data(), value.size()); 
  bool ret = (rdb_->Put(wopts, put_key, put_val)).ok();
  
  if (ret)
    return Status();
  else return Status().IOError(Slice());
}

Status DBImpl::Delete(const WriteOptions& options, const Slice& key) {
  RecordTick(options_.statistics.get(), REQ_DEL);
  rocksdb::WriteOptions wopts;
  wopts.disableWAL = true;
  rocksdb::Slice put_key(key.data(), key.size());
  bool ret = (rdb_->Delete(wopts, put_key)).ok();

  if (ret)
    return Status();
  else return Status().IOError(Slice());
}

Status DBImpl::Write(const WriteOptions& options, WriteBatch* updates) {
  // Not Support yet
  return Status();
}

Status DBImpl::Get(const ReadOptions& options,
                     const Slice& key,
                     std::string* value) {
  RecordTick(options_.statistics.get(), REQ_GET);
  // rocksdb impl in kvssd
  rocksdb::ReadOptions ropts;
  std::string getVal;
  rocksdb::Slice get_key(key.data(), key.size());
  bool ret = (rdb_->Get(ropts, get_key, &getVal)).ok();
  if (ret) {
    value->swap(getVal);
    return Status();
  }
  else return Status().NotFound(Slice());
}

Iterator* DBImpl::NewIterator(const ReadOptions& options) {
  return NewDBIterator(this, options);
}

Status DB::Open(const Options& options, const std::string& dbname,
                DB** dbptr) {

  *dbptr = NULL;

  DB *db = new DBImpl(options, dbname);
  *dbptr = db;
  return Status(Status::OK());
}

}  // namespace rockskv

