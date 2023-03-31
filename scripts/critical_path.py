import os
import sys

dtype = "int"
op = "sum"
bufsize = "8"
expname = "critical_path_breakdown"
logsdir = "./logs/"

def parse_log(logname):
    malloc = 0
    encrypt = 0
    comm = 0
    decrypt = 0
    free = 0

    with open(logname, "r") as log:
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

with open(logsdir + expname + ".csv", "w") as output:
    baseline = parse_log(f"{logsdir}baseline.{bufsize}.log")
    naive = parse_log(f"{logsdir}naive.{bufsize}.log")
    optimized = parse_log(f"{logsdir}optimized.{bufsize}.log")

    print(baseline)
    print(naive)
    print(optimized)

    output.write("dtype,op,mode,bufsize,malloc,encrypt,comm,decrypt,free\n")
    output.write(f"{dtype},{op},baseline,{bufsize},{baseline}\n")
    output.write(f"{dtype},{op},naive,{bufsize},{naive}\n")
    output.write(f"{dtype},{op},optimized,{bufsize},{optimized}\n")
