import os
import sys
from pathlib import Path

logs_directory = sys.argv[1]

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
    with open(f"{logs_directory}/critical_path.csv", "w") as csv_out:
        csv_out.write("dtype,op,mode,msgsize,trial,malloc,encrypt,comm,decrypt,free\n")

        for mode in modes:
            for msg_size in msgsizes:
                for trial in range(ntrials):
                    filename = f"{logdir}/critical_path.{mode}.{msg_size}.{trial+1}.log"
                    print(filename)
                    data = parse_cycles_measurements_from_file(filename)
                    csv_out.write(f"{dtype},{op},{mode},{msg_size},{trial+1},{data}\n")


def postprocess_block_size(logdir, dtype, op, modes, msgsizes, ntrials, ranks, block_sizes):
    with open(f"{logs_directory}/block_size.csv", "w") as csv_out:
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
    with open(f"{logs_directory}/allreduce_{dtype}_scaling.csv", "w") as csv_out:
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
                for msg_size in large_msgsizes:
                    for block_size in block_sizes:
                        for trial in range(ntrials):
                            filename = f"{logdir}/osu_allreduce.{mode}.{nranks}.{msg_size}.{block_size}.{trial+1}.log"

                            if not Path(filename).is_file():
                                continue

                            print(filename)
                            data = parse_osu_measurements_from_file(filename)
                            csv_out.write(f"{dtype},{op},{mode},{nranks},{block_size},{msg_size},{trial+1},{data}\n")


def postprocess_single_core_tput(logdir, bufsizes, dtypes, ops, funcs):
    expname = "single_core_encr_tput"
    with open(logs_directory + "/" + expname + ".csv", "w") as output:
        output.write("dtype,op,func,mode,bufsize,tput\n")
        for dtype in dtypes:
            for op in ops:
                for func in funcs:
                    for bufsize in bufsizes:
                        csv_row = dtype + "," + op + "," + func
                        real_bufsize = ""
                        path = logdir + expname + "." + dtype + "." + op + "." + func + "." + bufsize + ".log"
                        print(path)
                        if not Path(path).is_file():
                            continue
                        with open(path, "r") as log:
                            for line in log:
                                if "Buffer size:" in line:
                                    real_bufsize = line.split(" ")[-2]
                                if "encryption throughput:" in line:
                                    result = ",encryption," + real_bufsize + "," + line.split(" ")[-2]
                                    output.write(csv_row + result + "\n")
                                if "decryption throughput:" in line:
                                    result = ",decryption," + real_bufsize + "," + line.split(" ")[-2]
                                    output.write(csv_row + result + "\n")


postprocess_critical_path(
    logdir=f"{logs_directory}/logs_critical_path/",
    dtype="int",
    op="sum",
    modes=["baseline", "naive", "optimized"],
    msgsizes=[8,16],
    ntrials=5)

postprocess_block_size(
    logdir=f"{logs_directory}/logs_block_size/",
    dtype="int",
    op="sum",
    modes=["baseline", "mpool_only", "optimized"],
    msgsizes=[8388608,16777216],
    ntrials=2,
    ranks=[72],
    block_sizes=[1024,2048,4096,8192,16384,32768,65536,131072,262144,524288,1048576]
)

postprocess_osu_allreduce(
    logdir=f"{logs_directory}/logs_allreduce_int_scaling/",
    dtype="int",
    op="sum",
    modes=["baseline", "optimized"],
    small_msgsizes=[8,16],
    large_msgsizes=[8388608,16777216],
    ntrials=2,
    ranks=[2,4,8,18,36,72,144,288,576,1152],
    block_sizes=[8192,16384,32768,65536,131072,262144,524288]
)

postprocess_osu_allreduce(
    logdir=f"{logs_directory}/logs_allreduce_float_scaling/",
    dtype="float",
    op="sum",
    modes=["baseline", "optimized"],
    small_msgsizes=[8,16],
    large_msgsizes=[8388608,16777216],
    ntrials=2,
    ranks=[2,4,8,18,36,72,144,288,576,1152],
    block_sizes=[8192,16384,32768,65536,131072,262144,524288]
)

postprocess_single_core_tput(
    logdir=f"{logs_directory}/logs_single_core_tput/",
    bufsizes = [str(2**j) for j in range(1, 22)],
    dtypes = ["int", "float"],
    ops = ["sum"],
    funcs = ["naive", "sha1sse2", "sha1avx2", "aesni", "aesni_unroll"]
)
