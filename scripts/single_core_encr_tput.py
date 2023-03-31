import sys
import os
import subprocess
from pathlib import Path

thispath = os.path.dirname(os.path.realpath(__file__))
logdir = thispath + "/logs/"
expname = "single_core_encr_tput"
scriptpath = thispath + "/../encr_perf_test"
nranks = "1"
niters = "100"
bufsizes = [str(2**j) for j in range(1, 22)]
dtypes = ["int"]
ops = ["sum"]
funcs = ["naive", "sha1sse2", "sha1avx2", "aesni"]


if not Path(logdir).is_dir():
    os.mkdir(logdir)


for dtype in dtypes:
    for op in ops:
        for func in funcs:
            for bufsize in bufsizes:
                with open(logdir + expname + "." + dtype + "." + op + "." + func + "." + bufsize + ".log", "w") as log:
                    cmd = [scriptpath, niters, bufsize, nranks, dtype, op, func]
                    print(cmd)
                    subprocess.call(cmd, stdout=log, stderr=log)


with open(logdir + expname + ".csv", "w") as output:
    output.write("dtype,op,func,mode,bufsize,tput\n")
    for dtype in dtypes:
        for op in ops:
            for func in funcs:
                for bufsize in bufsizes:
                    csv_row = dtype + "," + op + "," + func
                    real_bufsize = ""
                    with open(logdir + expname + "." + dtype + "." + op + "." + func + "." + bufsize + ".log", "r") as log:
                        for line in log:
                            if "Buffer size:" in line:
                                real_bufsize = line.split(" ")[-2]
                            if "encryption throughput:" in line:
                                result = ",encryption," + real_bufsize + "," + line.split(" ")[-2]
                                output.write(csv_row + result + "\n")
                            if "decryption throughput:" in line:
                                result = ",decryption," + real_bufsize + "," + line.split(" ")[-2]
                                output.write(csv_row + result + "\n")


                    
