#!/bin/bash

HOME_DIR=$(pwd)
wget https://mvapich.cse.ohio-state.edu/download/mvapich/osu-micro-benchmarks-7.0.1.tar.gz
tar -xzf osu-micro-benchmarks-7.0.1.tar.gz
rm osu-micro-benchmarks-7.0.1.tar.gz
cd osu-micro-benchmarks-7.0.1/
./configure CC=cc CXX=mpic++
cd c/mpi/collective/
make
cp ./osu_allreduce $HOME_DIR/osu_allreduce_float
make clean
cp $HOME_DIR/osu_patch/osu_allreduce.c ./
make
cp osu_allreduce $HOME_DIR/osu_allreduce_int
cd $HOME_DIR
rm -rf osu-micro-benchmarks-7.0.1

cd dnn-proxies/
mpic++ ./gpt3.cpp -O3 -ffast-math -o gpt3
mpic++ ./dlrm.cpp -O3 -ffast-math -o dlrm
mpic++ ./cosmoflow.cpp -O3 -ffast-math -o cosmoflow
mpic++ ./resnet152.cpp -O3 -ffast-math -o resnet

cd $HOME_DIR

mkdir build/
mkdir build/bin/
mkdir build/lib/

make encr_perf_test
mv encr_perf_test build/bin
make clean

make hear_release_aes
mv libhear.so build/lib/libhear_release.so
make clean

make hear_mpool_only
mv libhear.so build/lib/libhear_release_mpool.so
make clean

make hear_baseline_tsc
mv libhear.so build/lib/libhear_critical_path_baseline.so
make clean

make hear_naive_tsc
mv libhear.so build/lib/libhear_critical_path_naive.so
make clean

make hear_mpool_only_tsc
mv libhear.so build/lib/libhear_critical_path_mpool.so
make clean

make security
mv security build/bin
make clean

make accuracy
mv accuracy_addition build/bin
mv accuracy_multiplication build/bin
make clean

make correctness
mv hfloat_correctness build/bin
mv integer_correctness build/bin
make clean
