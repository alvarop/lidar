#
# Read pickled data from file and play back at "real speed"
#

import sys
import numpy as np
import time
import argparse
import pickle
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *

# OpenGL window size
w_size = 1000
h_size = 1000

# OpenGL scale pixel/mm
scale = 1.0 / 20

angle_adj = 0

distances = [0] * 360


def playback():
    global data, start_time, distances

    if len(data) == 0:
        sys.exit()
        return

    print((time.time() - start_time) * args.speed)

    if (time.time() - start_time) * args.speed > data[0][0]:
        distances = data[0][1]
        del data[0]

    glBegin(GL_POINTS)
    glColor3f(0.224, 1.0, .078)
    for angle in range(360):
        y = distances[angle] * scale * np.cos(np.deg2rad(angle + args.angle_adj))
        x = distances[angle] * scale * np.sin(np.deg2rad(angle + args.angle_adj))
        # print(w_size/2 + x, h_size/2 + y)
        glVertex2f(w_size / 2 + x, h_size / 2 + y)
    glEnd()


parser = argparse.ArgumentParser()
parser.add_argument("--speed", default=1.0, type=float, help="Playback speed")
parser.add_argument("--angle_adj", default=0, type=float, help="Angle adjustment")
parser.add_argument("filename", help="Dump filename")
args = parser.parse_args()


def iterate():
    glViewport(0, 0, w_size, h_size)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    glOrtho(0.0, w_size, 0.0, h_size, 0.0, 1.0)
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()


def showScreen():
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    glLoadIdentity()
    iterate()
    playback()
    glutSwapBuffers()


print("Loading " + args.filename)
with open(args.filename, "rb") as dump_file:
    data = pickle.load(dump_file)
    start_time = time.time()

    glutInit()
    glutInitDisplayMode(GLUT_RGBA)
    glutInitWindowSize(w_size, h_size)
    glutInitWindowPosition(0, 0)
    wind = glutCreateWindow("OpenGL Coding Practice")
    glutDisplayFunc(showScreen)
    glutIdleFunc(showScreen)
    glutMainLoop()
