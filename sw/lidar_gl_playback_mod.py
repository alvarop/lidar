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
w_size = 600
h_size = 600

# OpenGL scale pixel/mm
scale = 1.0 / 20

angle_adj = 0

distances = [0] * 360

num_div = 30

def draw_circle(x,y,r,steps=360):
    glBegin(GL_LINE_STRIP)
    glColor3f(1, 1, 1)
    
    for angle in range(steps + 1):
        x_p = x + r * np.sin(2 * np.pi * angle / steps)
        y_p = y + r * np.cos(2 * np.pi * angle / steps)
        glVertex2f(x_p, y_p)

    glEnd()

def playback():
    global data, start_time, distances

    if len(data) == 0:
        sys.exit()
        return

    current_time = (time.time() - start_time) * args.speed

    sys.stdout.write("\b" * 10)  # Clear the previous line
    sys.stdout.write("{:0.3f}".format(current_time))
    sys.stdout.flush()

    if current_time > data[0][0]:
        distances = data[0][1]
        del data[0]

    glBegin(GL_LINES)
    glColor3f(0.4, 0.0, 0.0)
    glVertex2f(w_size / 2 - 10, h_size / 2)
    glVertex2f(w_size / 2 + 10, h_size / 2)
    glVertex2f(w_size / 2, h_size / 2 - 10)
    glVertex2f(w_size / 2, h_size / 2 + 10)
    glEnd()

    glBegin(GL_LINES)
    glColor3f(0.25, 0.25, 0.25)
    for x in range(-int(w_size/2)+int(w_size/num_div), int(w_size/2), int(w_size/num_div)):
        glVertex2f(w_size/2 + x, 0)
        glVertex2f(w_size/2 + x, w_size)

    for y in range(-int(h_size/2)+int(h_size/num_div), int(h_size/2), int(h_size/num_div)):
        glVertex2f(0, h_size/2 + y)
        glVertex2f(w_size, h_size/2 + y)
    glEnd()

    for r in range(int(w_size/num_div),int(w_size/2), int(w_size/num_div)):
        draw_circle(w_size/2, h_size/2, r, steps = 90)

    glBegin(GL_LINES)
    glColor3f(0.5, 0.5, 0.5)
    for angle in range(num_div*2):
        x_p = w_size/2 + w_size/2 * np.sin(2 * np.pi * angle / (num_div*2))
        y_p = h_size/2 + h_size/2 * np.cos(2 * np.pi * angle / (num_div*2))        
        glVertex2f(w_size/2, h_size/2)
        glVertex2f(x_p, y_p)


    glEnd()

    glEnable(GL_PROGRAM_POINT_SIZE)
    glPointSize(2.5)
    glBegin(GL_POINTS)
    glColor3f(0.224, 1.0, 0.078)
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
