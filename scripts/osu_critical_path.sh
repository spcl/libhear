#!/bin/bash

NITERATIONS=10000
LOG_PATH="/users/mkhalilo/hear/logs/"
OSU_PATH="/users/mkhalilo/hear/osu-micro-benchmarks-7.0.1/c/mpi/collective/osu_allreduce"
HEAR_BASELINE_PATH="/users/mkhalilo/hear/homomorphic-mpi/lib/libhear_critical_path_baseline.so"
HEAR_NAIVE_PATH="/users/mkhalilo/hear/homomorphic-mpi/lib/libhear_critical_path_naive.so"
HEAR_OPTIMIZED_PATH="/users/mkhalilo/hear/homomorphic-mpi/lib/libhear_critical_path_optimized.so"
SRUN_CMD="srun -A g34 -C mc"

for msg_size in {8,16}
do
    for trial in {1,2,3,4,5}
    do
	BASELINE_CMD="LD_PRELOAD=${HEAR_BASELINE_PATH} ${SRUN_CMD} ${OSU_PATH} -m ${msg_size}:${msg_size} -f -i ${NITERATIONS} &> ${LOG_PATH}/critical_path.baseline.${msg_size}.${trial}.log"
	echo "$BASELINE_CMD"
	echo "$BASELINE_CMD" &> ${LOG_PATH}/critical_path.baseline.${msg_size}.${trial}.cmd
	eval $BASELINE_CMD

	NAIVE_CMD="LD_PRELOAD=${HEAR_NAIVE_PATH} ${SRUN_CMD} ${OSU_PATH} -m ${msg_size}:${msg_size} -f -i ${NITERATIONS} &> ${LOG_PATH}/critical_path.naive.${msg_size}.${trial}.log"
	echo "$NAIVE_CMD"
	echo "$NAIVE_CMD" &> ${LOG_PATH}/critical_path.naive.${msg_size}.${trial}.cmd
	eval $NAIVE_CMD

	OPTIMIZED_CMD="HEAR_ENABLE_AESNI=1 LD_PRELOAD=${HEAR_OPTIMIZED_PATH} ${SRUN_CMD} ${OSU_PATH} -m ${msg_size}:${msg_size} -f -i ${NITERATIONS} &> ${LOG_PATH}/critical_path.optimized.${msg_size}.${trial}.log"
	echo "$OPTIMIZED_CMD"
	echo "$OPTIMIZED_CMD" &> ${LOG_PATH}/critical_path.optimized.${msg_size}.${trial}.cmd
	eval $OPTIMIZED_CMD
    done
done