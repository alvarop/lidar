#
# Record RPLidar data over serial port and save pickled data in file
#
import sys
import numpy as np
import time
import serial
import argparse
import pickle
import os

# 2 bytes as hex strings is 4 characters
LINE_LEN = 360 * 4

start_time = time.time()

data = []


def read_data():
    line = ser.readline().strip()
    if len(line) != LINE_LEN:
        return

    raw_distances = []
    for angle in range(360):
        value = int(line[(angle * 4) : ((angle * 4) + 4)], 16)
        raw_distances.append(value)

    sample_time = time.time() - start_time

    data.append((sample_time, raw_distances))
    sys.stdout.write("\b" * 20)  # Clear the previous line
    sys.stdout.write("[{}] {:0.3f}s".format(len(data), sample_time))
    sys.stdout.flush()


parser = argparse.ArgumentParser()

parser.add_argument(
    "--duration", default=10.0, type=float, help="Log duration in seconds"
)
parser.add_argument(
    "--baud_rate", default=115200, type=int, help="Serial Port Baud Rate"
)
parser.add_argument("--outfile", help="Output filename")
parser.add_argument("serial_port", help="Serial Port")
parser.add_argument("--force", action="store_true", help="Allow file overwriting")

args = parser.parse_args()

if args.outfile and os.path.isfile(args.outfile) and not args.force:
    print("ERROR: File exists. Use --force to overwrite")
    sys.exit(-1)

ser = serial.Serial(args.serial_port, baudrate=args.baud_rate)

while (time.time() - start_time) < args.duration:
    read_data()

sys.stdout.write("\n")

if args.outfile:
    print("Saving dump to " + args.outfile)

    with open(args.outfile, "wb") as dump_file:
        pickle.dump(data, dump_file)
