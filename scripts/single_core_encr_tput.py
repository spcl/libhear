import sys
import os
import subprocess
from pathlib import Path

scriptpath = sys.argv[1] + "/bin/encr_perf_test"
logdir = sys.argv[2] + "/logs_single_core_tput/"
expname = "single_core_encr_tput_float"
nranks = "1"
niters = "1000"
bufsizes = [str(2**j) for j in range(1, 22)]
dtypes = ["int", "float"]
ops = ["sum"]
funcs = ["naive", "sha1sse2", "sha1avx2", "aesni", "aesni_unroll"]

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
