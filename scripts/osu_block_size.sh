#!/bin/bash

NRANKS=72
NITERATIONS=1000
LOG_PATH="/users/mkhalilo/hear/logs/"
OSU_PATH="/users/mkhalilo/hear/osu-micro-benchmarks-7.0.1/c/mpi/collective/osu_allreduce"
HEAR_PATH="/users/mkhalilo/hear/homomorphic-mpi/lib/libhear_block_size_pipelining.so"
HEAR_MPOOL_PATH="/users/mkhalilo/hear/homomorphic-mpi/lib/libhear_block_size_mpool.so"
SRUN_CMD="srun -A g34 -C mc"

for msg_size in {8388608,16777216}
do
    for trial in {1,2}
    do
	BASELINE_CMD="${SRUN_CMD} ${OSU_PATH} -m ${msg_size}:${msg_size} -f -i ${NITERATIONS} &> ${LOG_PATH}/block_size.baseline.${NRANKS}.${msg_size}.${trial}.log"
	echo "$BASELINE_CMD"
	echo "$BASELINE_CMD" &> ${LOG_PATH}/block_size.baseline.${NRANKS}.${msg_size}.${trial}.cmd
	eval $BASELINE_CMD

	MPOOL_CMD="HEAR_MPOOL_SBUF_LEN=${msg_size} HEAR_ENABLE_AESNI=1 LD_PRELOAD=${HEAR_MPOOL_PATH} ${SRUN_CMD} ${OSU_PATH} -m ${msg_size}:${msg_size} -f -i ${NITERATIONS} &> ${LOG_PATH}/block_size.mpool_only.${NRANKS}.${msg_size}.${trial}.log"
	echo "$MPOOL_CMD"
	echo "$MPOOL_CMD" &> ${LOG_PATH}/block_size.mpool_only.${NRANKS}.${msg_size}.${trial}.cmd
	eval $MPOOL_CMD

	for block_size in {1024,2048,4096,8192,16384,32768,65536,131072,262144,524288,1048576}
	do
	    BLOCK_CMD="HEAR_PIPELINING_BLOCK_SIZE=${block_size} HEAR_MPOOL_SBUF_LEN=${msg_size} HEAR_ENABLE_AESNI=1 LD_PRELOAD=${HEAR_PATH} ${SRUN_CMD} ${OSU_PATH} -m ${msg_size}:${msg_size} -f -i ${NITERATIONS} &> ${LOG_PATH}/block_size.${NRANKS}.${block_size}.${msg_size}.${trial}.log"
	    echo "$BLOCK_CMD"
	    echo "$BLOCK_CMD" &> ${LOG_PATH}/block_size.${NRANKS}.${block_size}.${msg_size}.${trial}.cmd
	    eval $BLOCK_CMD
	done
    done
done