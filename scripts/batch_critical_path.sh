#!/bin/bash -l

#SBATCH --job-name="critical_path"
#SBATCH --account="g34"
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mikhail.khalilov@inf.ethz.ch
#SBATCH --time=0:40:00
#SBATCH --nodes=2
#SBATCH --ntasks-per-core=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --partition=normal
#SBATCH --constraint=mc
#SBATCH --hint=nomultithread
#SBATCH --exclusive

NITERATIONS=100000

OSU_DIR=$1
HEAR_LIB_DIR=$2
LOG_DIR=$3

LOG_PATH="${LOG_DIR}/logs_critical_path/"
OSU_PATH="${OSU_DIR}/osu_allreduce_int"

HEAR_BASELINE_PATH="${HEAR_LIB_DIR}/libhear_critical_path_baseline.so"
HEAR_NAIVE_PATH="${HEAR_LIB_DIR}/libhear_critical_path_naive.so"
HEAR_OPTIMIZED_PATH="${HEAR_LIB_DIR}/libhear_critical_path_mpool.so"

#SRUN_CMD="srun --cpu-bind=core"
SRUN_CMD="mpirun -n 2 --hostfile=hostfile"

eval mkdir $LOG_PATH

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
