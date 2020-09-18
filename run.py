import subprocess
import os
import argparse
import sys
import time
import signal

from multiprocessing.pool import ThreadPool

from prost.prost import run_prost
from prost.testbed.run_server import run_server


pool = ThreadPool(processes=2)


def parse_args():
    formatter = lambda prog: argparse.ArgumentDefaultsHelpFormatter(
        prog, max_help_position=38
    )
    parser = argparse.ArgumentParser(
        description="Start RiDDLeSim and PROST as needed for the learning.",
        formatter_class=formatter,
    )
    parser.add_argument(
        "-b",
        "--benchmark",
        action="store",
        default="./prost/testbed/benchmarks/elevators-2011/",
        dest="benchmark",
        help="Path to RDDL files."
    )
    parser.add_argument(
        "-i",
        "--instance",
        action="store",
        default="./prost/testbed/benchmarks/elevators-2011/elevators_inst_mdp__1.rddl",
        dest="instance",
        help="RDDL-instance file."
    )
    parser.add_argument(
        "-p",
        "--port",
        action="store",
        default="2323",
        type=str,
        dest="port",
        help="Port number where rddlsim listens.",
    )
    parser.add_argument(
        "-r",
        "--num-rounds",
        action="store",
        default="30",
        type=str,
        dest="rounds",
        help="Number of rounds.",
    )
    parser.add_argument(
        "-s",
        "--seed",
        action="store",
        default="0",
        type=str,
        dest="seed",
        help="Random seed."
    )
    parser.add_argument(
        "-t",
        "--timeout",
        action="store",
        default="0",
        type=str,
        dest="timeout",
        help='Total timeout in seconds. No timeout is used if timeout is "0".',
    )
    parser.add_argument(
        "-l",
        "--log-dir",
        action="store",
        default="./rddlsim_logs",
        dest="log",
        help="Directory where log files are written.",
    )
    parser.add_argument(
        "-Xms",
        "--init-memory",
        action="store",
        default="100",
        dest="init_memory",
        help="Initial amount of memory in MB allocated by the Java VM.",
    )
    parser.add_argument(
        "-Xmx",
        "--max-memory",
        action="store",
        default="500",
        dest="max_memory",
        help="Maximum amount of memory in MB that may be allocated by the Java VM.",
    )
    args = parser.parse_args()
    return args


def run_server_local(args):
    '''
    callstring = "python ./prost/testbed/run-server.py -b {} -p {} -r {} -s {} -t {} -l {} -Xms {} -Xmx {}".format(args.benchmark, args.port, args.rounds, args.seed, args.timeout, args.log, args.init_memory, args.max_memory)
    print(callstring)
    exitcode = subprocess.call(callstring, shell=True)
    return exitcode
    '''
    exitcode = run_server("-b {} -p {} -r {} -s {} -t {} -l {} -Xms {} -Xmx {}".format(args.benchmark, args.port, args.rounds, args.seed, args.timeout, args.log, args.init_memory, args.max_memory).split(" "))
    return exitcode


def run_prost_local(args):
    '''
    callstring = "python ./prost/prost.py"# {} [Prost -s 1 -se [IPC2014]]".format(os.path.basename(args.instance))
    print(callstring)
    exitcode = subprocess.call(callstring, shell=True)
    return exitcode
    '''
    exitcode = run_prost(["prost.py", os.path.basename(args.instance).split(".")[0], "\"[Prost -s 1 -se [IPC2014]]\""])
    return exitcode


def exit_nicely(signum, frame):
    print("Finirshing RiDDLeSim and PROST")
    pool.terminate()
    pool.join()
    sys.exit(0)


if __name__ == "__main__":
    signal.signal(signal.SIGINT, exit_nicely)
    
    try:
        rddlsim_root = os.environ["RDDLSIM_ROOT"]
    except KeyError:
        print("Error: an environment variable RDDLSIM_ROOT pointing to the rddlsim installation must be setup.")
        sys.exit()

    args = parse_args()
    
    tasks = [None, None]
    
    tasks[0] = pool.apply_async(run_server_local, args=[args])
    time.sleep(2)
    tasks[1] = pool.apply_async(run_prost_local, args=[args])

    sys.exit(tasks[0].get() + tasks[1].get())

