#
# Read pickled data from file and play back at "real speed"
#

import sys
import numpy as np
import time
import argparse
import pickle

# 2 bytes as hex strings is 4 characters
LINE_LEN = 360 * 4

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

parser.add_argument("filename", help="Dump filename")

args = parser.parse_args()

print("Opening " + args.filename)
with open(args.filename, 'rb') as dump_file:
    data = pickle.load(dump_file)

    start_time = time.time()
    while len(data) > 0:
        sample_time, line = data[0]
        while (time.time() - start_time) < sample_time:
            time.sleep(0.01)
        del data[0]
        print(sample_time, line)
