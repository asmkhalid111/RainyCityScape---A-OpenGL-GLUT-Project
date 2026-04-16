#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>

float p = -10.0; [cite: 455]
float speed = 0.05; [cite: 456]
int paused = 0; [cite: 457]

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT); [cite: 461]
    glBegin(GL_QUADS); [cite: 462]
    glColor3f(1.0, 1.0, 1.0); [cite: 463]
    glVertex2f(p, 3); [cite: 464]
    glVertex2f(p + 3, 3); [cite: 465]
    glVertex2f(p + 3, -3); [cite: 466]
    glVertex2f(p, -3); [cite: 467]
    glEnd(); [cite: 468]
    glFlush(); [cite: 469]
}

void timer(int value) {
    if (!paused) { [cite: 472]
        p += speed; [cite: 474, 475]
    }
    if (p > 10) [cite: 477]
        p = -13; [cite: 478]
    if (p < -13) [cite: 479]
        p = 10; [cite: 480]

    glutPostRedisplay(); [cite: 481]
    glutTimerFunc(16, timer, 0); [cite: 482]
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) { [cite: 485]
        case 'p': [cite: 488]
            paused = !paused; [cite: 489]
            break;
        case 'r': [cite: 491]
            p = -10; [cite: 492]
            break;
        case 'a': [cite: 494]
            p -= 0.3; [cite: 495]
            break;
        case 'd': [cite: 497]
            p += 0.3; [cite: 498]
            break;
        case '+': [cite: 500]
            speed += 0.01; [cite: 501]
            break;
        case '-': [cite: 503]
            speed -= 0.01; [cite: 504]
            if (speed < 0) speed = 0; [cite: 505]
            break;
    }
}

void init(void) {
    glClearColor(1.0, 0.0, 0.0, 0.0); [cite: 512]
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-10, 10, -10, 10, -10, 10); [cite: 513]
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); [cite: 517]
    glutInitWindowSize(600, 600); [cite: 518]
    glutInitWindowPosition(100, 100); [cite: 519]
    glutCreateWindow("Moving Object"); [cite: 520]
    init(); [cite: 521]
    glutDisplayFunc(display); [cite: 522]
    glutKeyboardFunc(keyboard); [cite: 523]
    glutTimerFunc(0, timer, 0); [cite: 524]
    glutMainLoop(); [cite: 525]
    return 0; [cite: 526]
}