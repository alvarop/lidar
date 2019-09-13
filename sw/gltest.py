from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
import numpy as np

import serial

w_size = 2000
h_size = 2000

ser = serial.Serial("/dev/ttyACM1", baudrate=115200)
ser.flush()
count = 0

scale = 1.0/10

def read_data():
    global count
    global scale
    angle_adj = 90
    line = ser.readline().strip()
    # print(line)

    glBegin(GL_LINES)
    glColor3f(1.0, 0.0, 0.0)
    glVertex2f(w_size/2 - 10, h_size/2)
    glVertex2f(w_size/2 + 10, h_size/2)
    glVertex2f(w_size/2, h_size/2 - 10)
    glVertex2f(w_size/2, h_size/2 + 10)
    glEnd()

    if int(len(line)/4) == 360:
        glBegin(GL_POINTS)
        glColor3f(1.0, 1.0, 1.0)
        for angle in range(360):
            distance = int(line[(angle*4):((angle*4)+4)], 16)
            y = distance * scale * np.cos(np.deg2rad(angle + angle_adj))
            x = distance * scale * np.sin(np.deg2rad(angle + angle_adj))
            # print(w_size/2 + x, h_size/2 + y)
            glVertex2f(w_size/2 + x, h_size/2 + y)

        glEnd()

        print("----------{}----------".format(count))
        count += 1
    else:
        print("NOOOOOOO")
        print(line)



def iterate():
    glViewport(0, 0, w_size, h_size)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    glOrtho(0.0, w_size, 0.0, h_size, 0.0, 1.0)
    glMatrixMode (GL_MODELVIEW)
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