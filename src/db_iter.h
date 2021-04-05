/******* rockskv *******/
/* db_iter.h
* 08/06/2019
* by Mian Qin
*/

#ifndef _rockskv_db_iter_h_
#define _rockskv_db_iter_h_

#include "rockskv/iterator.h"
#include "db_impl.h"

namespace rockskv {

Iterator* NewDBIterator(DBImpl *db, const ReadOptions &options);
} // end namespace rockskv

#endif