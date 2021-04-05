# (A) Optimized mode
 OPT ?= -O2 -DNDEBUG
# (B) Debug mode
# OPT ?= -g -O0

TARGET=librockskv.so

HOME=$(shell pwd)
CC=gcc
INCLUDES=-I$(HOME)/include
LIBS=-L$(HOME)/libs -Wl,-rpath,$(HOME)/libs -lrt -lpthread -lrocksdb -lkvssd -ltbb
CXXFLAG=-fPIC -w -march=native -std=c++11 $(OPT)

DB_SRCS=$(HOME)/src/db_impl.cc $(HOME)/src/db_iter.cc $(HOME)/src/hash.cc 
UTIL_SRCS=$(HOME)/util/comparator.cc
SRCS=$(DB_SRCS) $(UTIL_SRCS)

all: kvssd rocksdb rockskv

kvssd:
	$(CC) -shared -o $(HOME)/libs/libkvssd.so $(INCLUDES) $(KVSSD_SRCS) -L$(HOME)/libs -Wl,-rpath,$(HOME)/libs -lkvapi -lnuma $(CXXFLAG)

rocksdb:
	make -C $(HOME)/src/rocksdb/ shared_lib -j8
	cp $(HOME)/src/rocksdb/librocksdb* $(HOME)/libs -P

rockskv:
	$(CC) -shared -o $(HOME)/libs/$(TARGET) $(SRCS) $(INCLUDES) $(LIBS) $(CXXFLAG)

clean:
	make -C $(HOME)/src/rocksdb/ clean
	rm -rf $(HOME)/libs/$(TARGET) $(HOME)/libs/libkvssd.so $(HOME)/libs/librocksdb*
