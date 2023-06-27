#!/bin/bash -l

#SBATCH --job-name="gpt3"
#SBATCH --account="g34"
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mikhail.khalilov@inf.ethz.ch
#SBATCH --time=420
#SBATCH --nodes=48
#SBATCH --ntasks=384
#SBATCH --ntasks-per-core=1
#SBATCH --ntasks-per-node=8
#SBATCH --ntasks-per-socket=8
#SBATCH --partition=normal
#SBATCH --constraint=mc
#SBATCH --hint=nomultithread
#SBATCH --exclusive

BLOCK_SIZE=262144

BIN_DIR=$1
HEAR_LIB_DIR=$2
LOG_DIR=$3

LOG_PATH="${LOG_DIR}/logs_dnn/"
BINARY_PATH="${BIN_DIR}/gpt3"
HEAR_PATH="${HEAR_LIB_DIR}/libhear_release.so"

# SRUN_CMD="srun --cpu-bind=core"
SRUN_CMD="mpirun -n 384 --hostfile=hostfile"

eval mkdir $LOG_PATH

BASELINE_CMD="${SRUN_CMD} ${BINARY_PATH} &> ${LOG_PATH}/gpt3.baseline.log"
echo "$BASELINE_CMD"
echo "$BASELINE_CMD" &> ${LOG_PATH}/gpt3.baseline.cmd
eval $BASELINE_CMD

HEAR_CMD="HEAR_PIPELINING_BLOCK_SIZE=${BLOCK_SIZE} HEAR_MPOOL_SBUF_LEN=8388608 HEAR_ENABLE_AESNI=1 LD_PRELOAD=${HEAR_PATH} ${SRUN_CMD} ${BINARY_PATH} &> ${LOG_PATH}/gpt3.hear.log"
echo "$HEAR_CMD"
echo "$HEAR_CMD" &> ${LOG_PATH}/gpt3.hear.cmd
eval $HEAR_CMD
