# `LSMfree`: an LSM tree implementation
`LSMfree` is an implementation of the *Log-Structured Merge tree* data structure. Log-Structured Merge Trees have gained immense popularity in the recent years. The amount of data that companies have to deal with today, and the rate with which they are generated, have created the need for heavily optimized structures that are able to *i*. provide vast write throughput and, at the same time, *ii*. provide efficient read performance. LSM trees are such a structure; they are able to provide asynchronous and highly efficient writes, while having special indexing structures (in particular, *bloom filters* and *fence pointers*), in order to provide efficient reads. We present an optimized implementation of such a structure, which we call `LSMfree`.

The files necessary for running this code can be compiled through the Makefile by executing the command
```c
make
```
or if necessary, 
```c
make clean
```

`main.c` contains the setting of the hyperparameters of the system. Currently they are set so that buffer size is equal to 1024, depth of the tree is 2000, and the fan-out or size ratio of the tree is 2. However, these can be altered through this file. 
