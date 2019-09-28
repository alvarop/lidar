from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
import numpy as np
import time
import serial

w_size = 2000
h_size = 2000

NUM_LEDS = 256

leds = []
for i in range(256):
    leds.append(
        [[100 + (i % 64) * 20, 100 + (i // 64) * 20], np.array([0.1, 0.1, 0.1])]
    )

led = 0


def animate():
    global led
    leds[led][1] = np.array([1, 1, 1])
    led += 1

    if led >= len(leds):
        led = 0


def fade():
    for led in leds:
        led[1] = led[1] - np.array([0.1, 0.1, 0.1])


def segments(radius, step, colors=None):
    glBegin(GL_TRIANGLES)
    for angle in range(0, 360, step):
        if colors is None:
            glColor3f(
                0.2 + (0.8 / (360 / step)) * (angle / step),
                0.2 + (0.8 / (360 / step)) * (angle / step),
                0.2 + (0.8 / (360 / step)) * (angle / step),
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


def draw():
    global count
    global scale

    # print(line)

    glBegin(GL_LINES)
    glColor3f(1.0, 0.0, 0.0)
    glVertex2f(w_size / 2 - 10, h_size / 2)
    glVertex2f(w_size / 2 + 10, h_size / 2)
    glVertex2f(w_size / 2, h_size / 2 - 10)
    glVertex2f(w_size / 2, h_size / 2 + 10)
    glEnd()

    glEnable(GL_PROGRAM_POINT_SIZE)
    glPointSize(10.0)

    glBegin(GL_POINTS)
    for (xy, color) in leds:
        glColor3f(color[0], color[1], color[2])
        glVertex2f(xy[0], xy[1])
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
    fade()
    animate()
    segments(200, 10)
    draw()
    glutSwapBuffers()
    time.sleep(0.025)


glutInit()
glutInitDisplayMode(GLUT_RGBA)
glutInitWindowSize(w_size, h_size)
glutInitWindowPosition(0, 0)
wind = glutCreateWindow("OpenGL Coding Practice")
glutDisplayFunc(showScreen)
glutIdleFunc(showScreen)
glutMainLoop()
