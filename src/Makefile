CC=gcc -std=c99 #-D_XOPEN_SOURCE=500
CFLAGS = -ggdb3 -W -Wall -Wextra -Werror -O3
LDFLAGS =
LIBS =

default: main

%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)

main: buffer.o functions.o bloom.o lsm_tree.o main.o murmurhash2.o
	[ ! -d data ] && mkdir data
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

workloadgenerator: workload_generator.cpp
	g++ -g -std=c++0x -o $@ $^


clean:
	rm -rf functions bloom murmurhash2 main data *.o
	rm workloadgenerator
