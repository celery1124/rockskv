/******* rockskv *******/
/* options.h
* 07/23/2019
* by Mian Qin
*/

#ifndef _options_h_
#define _options_h_


#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <string>

#include "rockskv/comparator.h"
#include "rockskv/statistics.h"

namespace rockskv {

class Comparator;
class Slice;

// Options to control the behavior of a database (passed to DB::Open)
struct Options {
  // -------------------
  // Parameters that affect behavior

  // Comparator used to define the order of keys in the table.
  // Default: a comparator that uses lexicographic byte-wise ordering
  //
  // REQUIRES: The client must ensure that the comparator supplied
  // here has the same name and orders keys *exactly* the same as the
  // comparator provided to previous open calls on the same DB.
  const Comparator* comparator;

  // RocksDB block size
  // Default: 65536 (64KB)
  int blockSize;

  // RocksDB block size
  // Default: 64<<20 (64MB)
  int levelBaseSize;

  // Block cache size in MB
  // Default: 128MB
  int blockCacheSize;
  
  // Whether enable value prefetch for iterators
  // Default: false
  bool prefetchEnabled;

  // Prefetch buffer size
  // Default: 64
  int prefetchDepth;

  // Prefetch total request threshold
  // Default: 128
  int prefetchReqThres;

  // Bits per key for bloom filter
  // Default: 8
  int filterBitsPerKey;

  // Statistic (create to record count)
  // Default: NULL
  std::shared_ptr<Statistics> statistics;

  // Statistic dump interval in seconds
  // Default: -1 (no dump)
  int stats_dump_interval;

  Options() : comparator(BytewiseComparator()),
              blockSize(65536),
              levelBaseSize(64<<20),
              blockCacheSize(128),
              prefetchEnabled(false),
              prefetchDepth(64),
              prefetchReqThres(128),
              filterBitsPerKey(8),
              statistics(nullptr),
              stats_dump_interval(-1) {
    // Load from environment variable
    char *env_p;
    
    if(env_p = std::getenv("PREFETCH_ENA")) {
      if (strcmp(env_p, "TRUE") == 0 || strcmp(env_p, "true") == 0)
        prefetchEnabled = true;
      else
        prefetchEnabled = false;
    }

    if(env_p = std::getenv("PREFETCH_DEPTH")) {
      prefetchDepth = atoi(env_p);
    }
  };
  static std::shared_ptr<Statistics> CreateDBStatistics() {
    printf("rockskv Statistics Created\n");
    return std::make_shared<Statistics>();
  } 
};

// Options that control read operations
struct ReadOptions {
  // Define the upper key (Non-Inclusive) for range query
  // Default: NULL
  Slice* upper_key;

  // Potential user hint for the length of a scan (how many next after seek?)
  // Default: 1 (adptively increase)
  int scan_length;

  // Potential user hint for the size of the value (packed or unpacked?)
  // 0 -> auto, 1 -> unpacked, 2 -> packed
  // Default: 0 (no hints)
  int hint_packed;

  // // Buffer size for base iterator in Bytes
  // // Default: 4MB
  // int base_iter_buffer_size;

  ReadOptions()
      : upper_key(NULL),
        scan_length(1),
        hint_packed(0) {
  }
};

// Options that control write operations
struct WriteOptions {
  // From LevelDB write options, currently we don't use this
  // Default: false
  bool sync;
  // Write Index in batch
  // Default: false
  bool batchIDXWrite;
  // Batch size for batch index write
  // Default: 8
  size_t batchIDXSize;
  // Update existing record
  // Default: false
  bool update;

  WriteOptions()
      : sync(false),
        batchIDXWrite(false),
        batchIDXSize(8),
        update(false) {
  }
};

}  // namespace rockskv

#endif
