# Homomorphic MPI


## Getting started

```
make
mpicxx test.cpp -o test
LD_PRELOAD=$(pwd)/libhear.so mpirun -np 2 ./test
```
