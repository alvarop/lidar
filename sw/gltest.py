import sys
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
import numpy as np

import serial

w_size = 1000
h_size = 1000

ser = serial.Serial(sys.argv[1], baudrate=115200)
ser.flush()
count = 0

scale = 1.0 / 20


def reduce(distances, num_segments):
    global last_segment

    if 360 % num_segments != 0:
        raise Exception("360 must be divisible by num_segments")

    step_size = int(360 / num_segments)
    segment = []
    for angle in range(0, 360, step_size):
        segment.append(int(np.mean(distances[angle : (angle + step_size)])))

    draw_segments(500, step_size, np.divide(segment, 10000))


avg_delta = 2

dist = []

def read_data():
    global count
    global scale
    angle_adj = 0
    line = ser.readline().strip()
    # print(line)

    glBegin(GL_LINES)
    glColor3f(1.0, 0.0, 0.0)
    glVertex2f(w_size / 2 - 10, h_size / 2)
    glVertex2f(w_size / 2 + 10, h_size / 2)
    glVertex2f(w_size / 2, h_size / 2 - 10)
    glVertex2f(w_size / 2, h_size / 2 + 10)
    glEnd()

    raw_distances = []
    for angle in range(360):
        value = int(line[(angle * 4) : ((angle * 4) + 4)], 16)
        raw_distances.append(value)

    if int(len(line) / 4) == 360:
        dist.append(raw_distances)

        if len(dist) > avg_delta:
            del dist[0]

        distances = np.mean(dist, axis=0)
        reduce(np.abs(np.subtract(distances, raw_distances)), 180)

        glBegin(GL_POINTS)
        glColor3f(0.224, 1.0, .078)
        for angle in range(360):
            y = distances[angle] * scale * np.cos(np.deg2rad(angle + angle_adj))
            x = distances[angle] * scale * np.sin(np.deg2rad(angle + angle_adj))
            # print(w_size/2 + x, h_size/2 + y)
            glVertex2f(w_size / 2 + x, h_size / 2 + y)

        glEnd()

        # print("----------{}----------".format(count))
        count += 1
    else:
        print("NOOOOOOO")
        print(line)


def draw_segments(radius, step, colors=None):
    glBegin(GL_TRIANGLES)
    for angle in range(0, 360, step):
        if colors is None:
            glColor3f(
                0.2 + (0.8 / (360 / step)) * (angle / step),
                0.2 + (0.8 / (360 / step)) * (angle / step),
                0.2 + (0.8 / (360 / step)) * (angle / step),
            )
        else:
            glColor3f(
                colors[int(angle / step)],
                colors[int(angle / step)],
                colors[int(angle / step)],
            )
        glVertex2f(w_size / 2, h_size / 2)
        glVertex2f(
            w_size / 2 + radius * np.sin(np.deg2rad(angle)),
            h_size / 2 + radius * np.cos(np.deg2rad(angle)),
        )
        glVertex2f(
            w_size / 2 + radius * np.sin(np.deg2rad(angle + step)),
            h_size / 2 + radius * np.cos(np.deg2rad(angle + step)),
        )
    glEnd()


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

    read_data()
    glutSwapBuffers()


glutInit()
glutInitDisplayMode(GLUT_RGBA)
glutInitWindowSize(w_size, h_size)
glutInitWindowPosition(0, 0)
wind = glutCreateWindow("OpenGL Coding Practice")
glutDisplayFunc(showScreen)
glutIdleFunc(showScreen)
glutMainLoop()
