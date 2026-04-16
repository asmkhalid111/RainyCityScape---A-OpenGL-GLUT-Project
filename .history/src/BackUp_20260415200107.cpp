#include<windows.h>
#ifdef _APPLE_
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <stdlib.h>
#include <math.h>

float p = -10.0;
float speed = 0.05;
int paused = 0;

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_QUADS);
    glColor3f(1.0, 1.0, 1.0);
    glVertex2f(p, 3);
    glVertex2f(p + 3, 3);
    glVertex2f(p + 3, -3);
    glVertex2f(p, -3);
    glEnd();
    glFlush();
}

void timer(int value)
{
    if (!paused) {
        p += speed;
    }
    if (p > 10)
        p = -13;
    if (p < -13)
        p = 10;
    
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'p':
        paused = !paused;
        break;
    case 'r':
        p = -10;
        break;
    case 'a':
        p -= 0.3;
        break;
    case 'd':
        p += 0.3;
        break;
    case '+':
        speed += 0.01;
        break;
    case '-':
        speed -= 0.01;
        if (speed < 0) speed = 0;
        break;
    default:
        break;
    }
}

void init(void)
{
    glClearColor(1.0, 0.0, 0.0, 0.0);
    glOrtho(-10, 10, -10, 10, -10, 10);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Moving Object");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, timer, 0);
    glutMainLoop();
    return 0;
}