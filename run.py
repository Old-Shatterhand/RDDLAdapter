import subprocess
import os
import argparse


def parse_args():
    pass


if __name__ == "__main__":
    try:
        rddlsim_root = os.environ["RDDLSIM_ROOT"]
    except KeyError:
        print("Error: an environment variable RDDLSIM_ROOT pointing to the rddlsim installation must be setup.")
        sys.exit()

    args = parse_arguments()

