import subprocess
import os
import argparse


def parse_args():
    formatter = lambda prog: argparse.ArgumentDefaultsHelpFormatter(
        prog, max_help_position=38
    )
    parser = argparse.ArgumentParser(
        description="Start RiDDLeSim and PROST as needed for the learning.",
        formatter_class=formatter,
    )
    parser.add_argument(
        "--all-ipc-benchmarks",
        action="store_true",
        default=False,
        help="Run all benchmarks.",
    )
    parser.add_argument(
        "-b", "--benchmark", action="store", default=None, help="Path to RDDL files."
    )
	parser.add_argument(
		"-i", "--instance", action="store", default=None, help="RDDL-instance file."
	)
    parser.add_argument(
        "-p",
        "--port",
        action="store",
        default="2323",
        type=str,
        help="Port number where rddlsim listens.",
    )
    parser.add_argument(
        "-r",
        "--num-rounds",
        action="store",
        default="30",
        type=str,
        help="Number of rounds.",
    )
    parser.add_argument(
        "-s", "--seed", action="store", default="0", type=str, help="Random seed."
    )
    parser.add_argument(
        "--separate-session",
        action="store_true",
        default=False,
        help="rddlsim terminates after a separate session with a client finishes.",
    )
    parser.add_argument(
        "-t",
        "--timeout",
        action="store",
        default="0",
        type=str,
        help='Total timeout in seconds. No timeout is used if timeout is "0".',
    )
    parser.add_argument(
        "-l",
        "--log-dir",
        action="store",
        default="./rddlsim_logs",
        help="Directory where log files are written.",
    )
    parser.add_argument(
        "--monitor-execution",
        action="store_true",
        default=False,
        help="Force client to specify if a round is considered.",
    )
    parser.add_argument(
        "-Xms",
        "--init-memory",
        action="store",
        default="100",
        help="Initial amount of memory in MB allocated by the Java VM.",
    )
    parser.add_argument(
        "-Xmx",
        "--max-memory",
        action="store",
        default="500",
        help="Maximum amount of memory in MB that may be allocated by the Java VM.",
    )
    args = parser.parse_args()
    return args


if __name__ == "__main__":
    try:
        rddlsim_root = os.environ["RDDLSIM_ROOT"]
    except KeyError:
        print("Error: an environment variable RDDLSIM_ROOT pointing to the rddlsim installation must be setup.")
        sys.exit()

    args = parse_arguments()

