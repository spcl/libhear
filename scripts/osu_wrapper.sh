#!/bin/bash

NRANKS=72
NITERATIONS=1000
LOG_PATH="/users/mkhalilo/hear/logs/"
OSU_PATH="/users/mkhalilo/hear/osu-micro-benchmarks-7.0.1/c/mpi/collective/osu_allreduce"
#HEAR_NAIVE_PATH="/users/mkhalilo/hear/homomorphic-mpi/lib/libhear_naive.so"
HEAR_OPTIMIZED_PATH="/users/mkhalilo/hear/homomorphic-mpi/lib/libhear_release.so"
SRUN_CMD="srun -A g34 -C mc"

for msg_size in {8,16,8388608,16777216}
do
    for trial in {1,2}
    do
	BASELINE_CMD="${SRUN_CMD} ${OSU_PATH} -m ${msg_size}:${msg_size} -f -i ${NITERATIONS} &> ${LOG_PATH}/osu_allreduce.baseline.${NRANKS}.${msg_size}.${trial}.log"
	echo "$BASELINE_CMD"
	echo "$BASELINE_CMD" &> ${LOG_PATH}/osu_allreduce.baseline.${NRANKS}.${msg_size}.${trial}.cmd
	eval $BASELINE_CMD

	#NAIVE_CMD="LD_PRELOAD=${HEAR_NAIVE_PATH} ${SRUN_CMD} ${OSU_PATH} -m ${msg_size}:${msg_size} -f -i ${NITERATIONS} &> ${LOG_PATH}/osu_allreduce.naive.${NRANKS}.${msg_size}.${trial}.log"
	#echo "$NAIVE_CMD"
	#echo "$NAIVE_CMD" &> ${LOG_PATH}/osu_allreduce.naive.${NRANKS}.${msg_size}.${trial}.cmd
	#eval $NAIVE_CMD
    done
done

for block_size in {32768,65536,131072} # empirically tuned with scripts/osu_block_size.sh
do
    for msg_size in {8,16,8388608,16777216}
    do
	for trial in {1,2}
	do
	    OPTIMIZED_CMD="HEAR_PIPELINING_BLOCK_SIZE=${block_size} HEAR_MPOOL_SBUF_LEN=2097152 HEAR_ENABLE_AESNI=1 LD_PRELOAD=${HEAR_OPTIMIZED_PATH} ${SRUN_CMD} ${OSU_PATH} -m ${msg_size}:${msg_size} -f -i ${NITERATIONS} &> ${LOG_PATH}/osu_allreduce.optimized.${NRANKS}.${msg_size}.${block_size}.${trial}.log"
	    echo "$OPTIMIZED_CMD"
	    echo "$OPTIMIZED_CMD" &> ${LOG_PATH}/osu_allreduce.optimized.${NRANKS}.${msg_size}.${block_size}.${trial}.cmd
	    eval $OPTIMIZED_CMD
	done
    done
done
