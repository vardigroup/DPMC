from __future__ import print_function
import fileinput
import sys

"""
A simple tool to process the output of DMC
"""

for line in fileinput.input():
    if line.startswith("c seconds"):
        print("Total Time: " + line.split()[-1])
        sys.stdout.flush()
    elif line.startswith("s wmc"):
        print("Count: " + line.split()[-1])
        sys.stdout.flush()

    print(line, file=sys.stderr, end='')
    sys.stderr.flush()
