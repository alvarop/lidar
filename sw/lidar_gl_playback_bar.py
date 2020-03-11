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
h_size = 600

# OpenGL scale pixel/mm
scale = 1.0 / 20

angle_adj = 0

num_div = 30

def draw_circle(x,y,r,steps=360):
    glBegin(GL_LINE_STRIP)
    glColor3f(1, 1, 1)
    
    for angle in range(steps + 1):
        x_p = x + r * np.sin(2 * np.pi * angle / steps)
        y_p = y + r * np.cos(2 * np.pi * angle / steps)
        glVertex2f(x_p, y_p)

    glEnd()

def draw_bargraph(distances, box_width=2, max_height=500, color=(0.2,0,0.25)):
    
    glBegin(GL_QUADS)
    glColor3f(*color)
    for angle in range(360):
        x_l = angle * box_width
        x_r = angle * box_width + box_width
        h = distances[angle] / 5000 * max_height

        glVertex2f(x_l, 0)
        glVertex2f(x_r, 0)
        glVertex2f(x_r, h)
        glVertex2f(x_l, h)
    glEnd()

last_samples = [0] * 360
def process_data(samples):
    global last_samples

    distances = []
    for index in range(len(samples)):
        if samples[index] == 0:
            distances.append(last_samples[index])
        else:
            distances.append(samples[index])
    # distances = data[0][1]

    last_samples = samples

    return distances
    
distances = [0] * 360
last_distances = [0] * 360
def playback():
    global data, start_time, distances, last_distances

    if len(data) == 0:
        sys.exit()
        return

    current_time = (time.time() - start_time) * args.speed

    sys.stdout.write("\b" * 10)  # Clear the previous line
    sys.stdout.write("{:0.3f}".format(current_time))
    sys.stdout.flush()

    if current_time > data[0][0]:
        distances = process_data(data[0][1])
        del data[0] 

    glBegin(GL_LINES)
    glColor3f(0.4, 0.0, 0.0)
    glVertex2f(w_size / 2 - 10, h_size / 2)
    glVertex2f(w_size / 2 + 10, h_size / 2)
    glVertex2f(w_size / 2, h_size / 2 - 10)
    glVertex2f(w_size / 2, h_size / 2 + 10)
    glEnd()

    draw_bargraph(distances)

    diff = []
    for index in range(len(distances)):
        diff.append(abs(distances[index]-last_distances[index]))
    draw_bargraph(diff,color=(1,1,1))
    
    last_distances = distances

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
