/******* kvrangedb *******/
/* bloom_test.h
* 01/13/2021
* by Mian Qin
*/


#include <bloom.h>
#include <string>
#include <iostream>
#include <set>
#include <ctime>
#include <chrono>
#include <vector>
#include "kvrangedb/options.h"

using namespace kvrangedb;

int key_set[16] = {1,3,66,57,58,59,511,513,15,18,21,26,31,32,35,46};

#define FNV_OFFSET_BASIS_64  0xCBF29CE484222325
#define FNV_OFFSET_BASIS_32  0x811c9dc5
#define FNV_PRIME_64  1099511628211L
#define FNV_PRIME_32 16777619

//#define VERFIY

int64_t fnvhash64(int64_t val) {
    //from http://en.wikipedia.org/wiki/Fowler_Noll_Vo_hash
    int64_t hashval = FNV_OFFSET_BASIS_64;

    for (int i = 0; i < 8; i++) {
      int64_t octet = val & 0x00ff;
      val = val >> 8;

      hashval = hashval ^ octet;
      hashval = hashval * FNV_PRIME_64;
      //hashval = hashval ^ octet;
    }
    return hashval >= 0 ? hashval : -hashval;
  }

int main() {
  std::shared_ptr<Statistics> stats = kvrangedb::Options::CreateDBStatistics();
  HiBloomFilter hbf(32, 2, 4, 16, 16, stats.get());

  for (int i = 0; i < 16; i++) {
    Slice key((char *)&key_set[i], 4);
    hbf.InsertItem(key);
  }

  bool exist ;
  int lowkey, highkey;
  lowkey = 36; highkey = 44;
  Slice lkey((char *)&lowkey, 4);
  Slice hkey((char *)&highkey, 4);
  exist = hbf.RangeMayMatch(lkey, hkey);
  std::cout << "key" << lowkey <<" to key " <<highkey<<" " << exist << std::endl;
  stats->Reset();

  // benchmark
  int num_keys = 100000000;
  int num_ops = 1000000;

  std::vector<std::vector<int>> bench_filter_config {{1,1}, {4,1}, {4,2}};
  std::vector<int> bench_bits_per_key {12, 16, 20, 24};
  std::vector<int> bench_range_dist {15, 31, 63, 127, 255, 511};
  std::vector<int> bench_exam_suffix_bits {16};

  // std::vector<vector<int>> bench_filter_config {{1,1}, {4,1}, {4,2}};
  // std::vector<int> bench_bits_per_key {24};
  // std::vector<int> bench_range_dist {15, 31, 63, 127, 255, 511};
  // std::vector<int> bench_exam_suffix_bits {8};


  for (int bbk = 0; bbk < bench_bits_per_key.size(); bbk++) {
    for (int bf = 0; bf < bench_filter_config.size(); bf++) {
      for (int be = 0; be < bench_exam_suffix_bits.size(); be++) {
        printf("levels: %d, bits_per_level: %d, exam_suffix_bits: %d, bits_per_key: %d\n\n",bench_filter_config[bf][0], bench_filter_config[bf][1], bench_exam_suffix_bits[be], bench_bits_per_key[bbk]); 
        auto wcts = std::chrono::system_clock::now();
        
        std::set<int64_t> ground_truth;
        HiBloomFilter hbf2(bench_bits_per_key[bbk], bench_filter_config[bf][1], bench_filter_config[bf][0], bench_exam_suffix_bits[be], num_keys, stats.get());

        for (int i = 0; i < num_keys; i++) {
          int64_t hashkey = fnvhash64(i);
          Slice key((char *)&hashkey, 8);
          hbf2.InsertItem(key);
#ifdef VERFIY
          ground_truth.insert(hashkey);
#endif
        }
        
        std::chrono::duration<double> insertduration = (std::chrono::system_clock::now() - wcts);
        std::cout << "Total insert time: " << insertduration.count() << "seconds [Wall Clock], " << "Average insert time: " << insertduration.count()/num_keys*1e6 << " microseconds [Wall Clock]" << std::endl;
        
        for (int br = 0; br < bench_range_dist.size(); br++ ) {
          printf("range_dist: %d\n",bench_range_dist[br]);
          wcts = std::chrono::system_clock::now();
#ifdef VERFIY
          int seed = num_keys + num_ops/2;
#else
          int seed = num_keys + num_ops/2;
#endif
          int range_size = bench_range_dist[br];
          int neg_cnt = 0;
          int p_neg_cnt = 0;
          int ground_neg_cnt = 0;
          for (int i = seed; i < seed+num_ops; i++) {
            int64_t lowkey = fnvhash64(i);
            int64_t highkey = lowkey + range_size;
            Slice lkey((char *)&lowkey, 8);
            Slice hkey((char *)&highkey, 8);
            bool exist = hbf2.RangeMayMatch(lkey, hkey);
            bool p_exist = hbf2.KeyMayMatch(lkey);
            
            if(!exist) neg_cnt++;
            if(!p_exist) p_neg_cnt++;
#ifdef VERFIY
            std::set<int64_t>::iterator itlow;
            itlow=ground_truth.lower_bound (lowkey);
            bool ground_exist = (itlow != ground_truth.end() && *itlow < highkey);
            // printf("lkey %llX, hkey %llX\n",lowkey, highkey);
            if (!ground_exist) ground_neg_cnt++;
            else printf("ground positive\n");
            if (!exist && exist!=ground_exist) {
              printf("[error] false negtive catched\n");
              assert(false);
            }
#endif
          }
          printf("range filter neg ratio: %.3f\npoint filter neg ratio: %.3f\n", double(neg_cnt)/num_ops, double(p_neg_cnt)/num_ops);
#ifdef VERFIY
          printf("ground neg ratio: %.3f\n", double(ground_neg_cnt)/num_ops);
#endif
          std::chrono::duration<double> queryduration = (std::chrono::system_clock::now() - wcts);
          std::cout << "Average query time: " << queryduration.count()/num_ops*1e6 << " microseconds [Wall Clock]" << std::endl;
          std::cout << "Average #probes: " << (double)kvrangedb::GetTickerCount(stats.get(), FILTER_RANGE_PROBES) / num_ops << std::endl;

          // stats->reportStats();
          stats->Reset();
        }
      }
      
    }
  }
  
  

  return 0;
}