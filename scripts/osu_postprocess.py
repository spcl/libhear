import os
import sys
from pathlib import Path

def parse_osu_measurements_from_file(filename):
    with open(filename, "r") as log:
        row = log.readlines()[-1]
        splitted_row = row.split()
        assert len(splitted_row) == 5
        msg_size = int(splitted_row[0])
        avg_lat = float(splitted_row[1])
        avg_tput = round(msg_size / avg_lat * 1e-3, 3)
        min_lat = float(splitted_row[2])
        min_tput = round(msg_size / min_lat * 1e-3, 3)
        max_lat = float(splitted_row[3])
        max_tput = round(msg_size / max_lat * 1e-3, 3)

    # bytes,us,gigabytes,us,gigabytes,us,gigabytes
    return f"{avg_lat},{avg_tput},{min_lat},{min_tput},{max_lat},{max_tput}"

def parse_cycles_measurements_from_file(filename):
    malloc = 0
    encrypt = 0
    comm = 0
    decrypt = 0
    free = 0

    with open(filename, "r") as log:
        for line in log:
            if "malloc" in line:
                malloc = int(line.split("=")[-1])
            if "encrypt" in line:
                encrypt = int(line.split("=")[-1])
            if "comm" in line:
                comm = int(line.split("=")[-1])
            if "decrypt" in line:
                decrypt = int(line.split("=")[-1])
            if "free" in line:
                free = int(line.split("=")[-1])

    return f"{malloc},{encrypt},{comm},{decrypt},{free}"

def postprocess_critical_path(logdir, dtype, op, modes, msgsizes, ntrials):
    with open(f"{logdir}/critical_path.csv", "w") as csv_out:
        csv_out.write("dtype,op,mode,msgsize,trial,malloc,encrypt,comm,decrypt,free\n")

        for mode in modes:
            for msg_size in msgsizes:
                for trial in range(ntrials):
                    filename = f"{logdir}/critical_path.{mode}.{msg_size}.{trial+1}.log"
                    print(filename)
                    data = parse_cycles_measurements_from_file(filename)
                    csv_out.write(f"{dtype},{op},{mode},{msg_size},{trial+1},{data}\n")

def postprocess_block_size(logdir, dtype, op, modes, msgsizes, ntrials, ranks, block_sizes):
    with open(f"{logdir}/block_size.csv", "w") as csv_out:
        csv_out.write("dtype,op,mode,nranks,block_size,msgsize,trial,avg_lat,avg_tput,min_lat,min_tput,max_lat,max_tput\n")

        for nranks in ranks:
            for mode in modes:
                for msg_size in msgsizes:
                    for trial in range(ntrials):
                        if mode != "optimized":
                            filename = f"{logdir}/block_size.{mode}.{nranks}.{msg_size}.{trial+1}.log"
                            print(filename)
                            data = parse_osu_measurements_from_file(filename)
                            csv_out.write(f"{dtype},{op},{mode},{nranks},0,{msg_size},{trial+1},{data}\n")
                        else:
                            for block_size in block_sizes:
                                filename = f"{logdir}/block_size.{nranks}.{block_size}.{msg_size}.{trial+1}.log"
                                print(filename)
                                data = parse_osu_measurements_from_file(filename)
                                csv_out.write(f"{dtype},{op},{mode},{nranks},{block_size},{msg_size},{trial+1},{data}\n")

def postprocess_osu_allreduce(logdir, dtype, op, modes, small_msgsizes, large_msgsizes, ntrials, ranks, block_sizes):
    with open(f"{logdir}/osu_allreduce.csv", "w") as csv_out:
        csv_out.write("dtype,op,mode,nranks,block_size,msgsize,trial,avg_lat,avg_tput,min_lat,min_tput,max_lat,max_tput\n")

        for nranks in ranks:
            for mode in modes:
                for msg_size in small_msgsizes + large_msgsizes:
                    for trial in range(ntrials):
                        filename = f"{logdir}/osu_allreduce.{mode}.{nranks}.{msg_size}.{trial+1}.log"

                        if not Path(filename).is_file():
                                continue

                        print(filename)
                        data = parse_osu_measurements_from_file(filename)
                        csv_out.write(f"{dtype},{op},{mode},{nranks},0,{msg_size},{trial+1},{data}\n")

        for nranks in ranks:
            for mode in modes:
                for msg_size in small_msgsizes + large_msgsizes:
                    for block_size in block_sizes:
                        for trial in range(ntrials):
                            filename = f"{logdir}/osu_allreduce.{mode}.{nranks}.{msg_size}.{block_size}.{trial+1}.log"

                            if not Path(filename).is_file():
                                continue

                            print(filename)
                            data = parse_osu_measurements_from_file(filename)
                            csv_out.write(f"{dtype},{op},{mode},{nranks},{block_size},{msg_size},{trial+1},{data}\n")

postprocess_critical_path(
    logdir="/users/mkhalilo/hear/logs/",
    dtype="int",
    op="sum",
    modes=["baseline", "naive", "optimized"],
    msgsizes=[8,16],
    ntrials=2)

postprocess_block_size(
    logdir="/users/mkhalilo/hear/logs/",
    dtype="int",
    op="sum",
    modes=["baseline", "mpool_only", "optimized"],
    msgsizes=[8388608,16777216],
    ntrials=2,
    ranks=[72],
    block_sizes=[1024,2048,4096,8192,16384,32768,65536,131072,262144,524288,1048576]
)

postprocess_osu_allreduce(
    logdir="/users/mkhalilo/hear/logs/",
    dtype="int",
    op="sum",
    modes=["baseline", "optimized"],
    small_msgsizes=[8,16],
    large_msgsizes=[8388608,16777216],
    ntrials=2,
    ranks=[2,4,8,18,36,72],
    block_sizes=[32768,65536,131072]
)
