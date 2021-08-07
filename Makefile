# (A) Optimized mode
 OPT ?= -O2 -DNDEBUG
# (B) Debug mode
# OPT ?= -g -O0

TARGET=librockskv.so

HOME=$(shell pwd)
CC=gcc
INCLUDES=-I$(HOME)/include
LIBS=-L$(HOME)/lib -Wl,-rpath,$(HOME)/lib -lrt -lpthread -lrocksdb -lkvssd -ltbb
CXXFLAG=-fPIC -w -march=native -std=c++11 $(OPT)

KVSSD_SRCS=$(HOME)/src/kvssd/kvssd.cc
DB_SRCS=$(HOME)/src/db_impl.cc $(HOME)/src/db_iter.cc $(HOME)/src/hash.cc 
UTIL_SRCS=$(HOME)/util/comparator.cc
SRCS=$(DB_SRCS) $(UTIL_SRCS)

all: kvssd rocksdb rockskv

kvssd:
	$(CC) -shared -o $(HOME)/lib/libkvssd.so $(INCLUDES) $(KVSSD_SRCS) -L$(HOME)/lib -Wl,-rpath,$(HOME)/lib -lkvapi -lnuma $(CXXFLAG)

rocksdb:
	make -C $(HOME)/src/rocksdb/ shared_lib -j8
	cp $(HOME)/src/rocksdb/librocksdb* $(HOME)/lib -P

rockskv:
	$(CC) -shared -o $(HOME)/lib/$(TARGET) $(SRCS) $(INCLUDES) $(LIBS) $(CXXFLAG)

clean:
	make -C $(HOME)/src/rocksdb/ clean
	rm -rf $(HOME)/lib/$(TARGET) $(HOME)/lib/libkvssd.so $(HOME)/lib/librocksdb*
