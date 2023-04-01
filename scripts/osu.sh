#!/bin/bash

MSG_SIZE=16777216
LOG_PATH="/users/mkhalilo/hear/homomorphic-mpi/scripts/logs/"
OSU_PATH="/users/mkhalilo/hear/osu-micro-benchmarks-7.0.1/c/mpi/collective/osu_allreduce"
HEAR_PATH="/users/mkhalilo/hear/homomorphic-mpi/libhear.so"
HEAR_MPOOL_PATH="/users/mkhalilo/hear/homomorphic-mpi/libhear_mpool.so"
SRUN_CMD="srun -A g34 -C mc"

echo "${SRUN_CMD} ${OSU_PATH} -m ${MSG_SIZE}:${MSG_SIZE}" &> ${LOG_PATH}/block_size.baseline.${MSG_SIZE}.1.cmd
echo "${SRUN_CMD} ${OSU_PATH} -m ${MSG_SIZE}:${MSG_SIZE}"
${SRUN_CMD} ${OSU_PATH} -m ${MSG_SIZE}:${MSG_SIZE} -i 1000 &> ${LOG_PATH}/block_size.baseline.${MSG_SIZE}.1.log

echo "HEAR_MPOOL_SBUF_LEN=${MSG_SIZE} HEAR_ENABLE_AESNI=1 LD_PRELOAD=${HEAR_MPOOL_PATH} ${SRUN_CMD} ${OSU_PATH} -m ${MSG_SIZE}:${MSG_SIZE}" &> ${LOG_PATH}/block_size.mpool_only.${MSG_SIZE}.1.cmd
echo "HEAR_MPOOL_SBUF_LEN=${MSG_SIZE} HEAR_ENABLE_AESNI=1 LD_PRELOAD=${HEAR_MPOOL_PATH} ${SRUN_CMD} ${OSU_PATH} -m ${MSG_SIZE}:${MSG_SIZE}"
HEAR_MPOOL_SBUF_LEN=${MSG_SIZE} HEAR_ENABLE_AESNI=1 LD_PRELOAD=${HEAR_MPOOL_PATH} ${SRUN_CMD} ${OSU_PATH} -m ${MSG_SIZE}:${MSG_SIZE} -i 1000 &> ${LOG_PATH}/block_size.mpool_only.${MSG_SIZE}.1.log

for block_size in {1024,2048,4096,8192,16384,32768,65536,131072,262144,524288,1048576}
do
    echo "HEAR_PIPELINING_BLOCK_SIZE=${block_size} HEAR_ENABLE_AESNI=1 LD_PRELOAD=${HEAR_PATH} ${SRUN_CMD} ${OSU_PATH} -m ${MSG_SIZE}:${MSG_SIZE}" &> ${LOG_PATH}/block_size.${block_size}.${MSG_SIZE}.1.cmd
    echo "HEAR_PIPELINING_BLOCK_SIZE=${block_size} HEAR_ENABLE_AESNI=1 LD_PRELOAD=${HEAR_PATH} ${SRUN_CMD} ${OSU_PATH} -m ${MSG_SIZE}:${MSG_SIZE}"
    HEAR_PIPELINING_BLOCK_SIZE=${block_size} HEAR_ENABLE_AESNI=1 LD_PRELOAD=${HEAR_PATH} ${SRUN_CMD} ${OSU_PATH} -m ${MSG_SIZE}:${MSG_SIZE} -i 1000 &> ${LOG_PATH}/block_size.${block_size}.${MSG_SIZE}.1.log
done
