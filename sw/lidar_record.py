#
# Record RPLidar data over serial port and save pickled data in file
#
import sys
import numpy as np
import time
import serial
import argparse
import pickle

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

    data.append(((time.time() - start_time), raw_distances))
    print(time.time() - start_time)


parser = argparse.ArgumentParser()

parser.add_argument(
    "--duration", default=10.0, type=float, help="Log duration in seconds"
)
parser.add_argument(
    "--baud_rate", default=115200, type=int, help="Serial Port Baud Rate"
)
parser.add_argument("--outfile", help="Output filename")
parser.add_argument("serial_port", help="Serial Port")

args = parser.parse_args()

ser = serial.Serial(args.serial_port, baudrate=args.baud_rate)

while (time.time() - start_time) < args.duration:
    read_data()

if args.outfile:
    print("Saving dump to " + args.outfile)
    with open(args.outfile, 'wb') as dump_file:
        pickle.dump(data, dump_file)
