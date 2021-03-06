/******* rockskv *******/
/* comparator.cc
* 07/23/2019
* by Mian Qin
*/

#include <algorithm>
#include <stdint.h>
#include <pthread.h>
#include "rockskv/comparator.h"
#include "rockskv/slice.h"

namespace rockskv {

Comparator::~Comparator() { }

class BytewiseComparatorImpl : public Comparator {
 public:
  BytewiseComparatorImpl() { }

  virtual int Compare(const Slice& a, const Slice& b) const {
    return a.compare(b);
  }
};

static pthread_once_t once = PTHREAD_ONCE_INIT;
static const Comparator* bytewise;

static void InitModule() {
  bytewise = new BytewiseComparatorImpl;
}

const Comparator* BytewiseComparator() {
  pthread_once(&once, InitModule);
  return bytewise;
}

}  // namespace rockskv