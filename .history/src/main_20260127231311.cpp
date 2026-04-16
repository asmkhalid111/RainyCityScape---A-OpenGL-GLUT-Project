#include <GL/glut.h>

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Front Roof (Red )
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.6f, 0.1f);
    glVertex2f(0.2f, 0.1f);
    glVertex2f(-0.2f, 0.4f);
    glEnd();

    // Side Roof (Orange)
    glColor3f(1.0, 0.6, 0.2); 
    glBegin(GL_QUADS);
    glVertex2f(-0.2f, 0.4f);
    glVertex2f(0.5f, 0.4f);
    glVertex2f(0.9f, 0.1f);
    glVertex2f(0.2f, 0.1f);
    glEnd();

    // Front Wall (Yellow)
    glColor3f(1.0, 1.0, 0.0);
    glBegin(GL_QUADS);
    glVertex2f(-0.6f, 0.1f);
    glVertex2f(0.2f, 0.1f);
    glVertex2f(0.2f, -0.6f);
    glVertex2f(-0.6f, -0.6f);
    glEnd();

    // Side Wall (Blue)
    glColor3f(0.1, 0.5, 0.8); 
    glBegin(GL_QUADS);
    glVertex2f(0.2f, 0.1f);
    glVertex2f(0.9f, 0.1f);
    glVertex2f(0.9f, -0.6f);
    glVertex2f(0.2f, -0.6f);
    glEnd();

    glFlush();
}

void init(void) {
    glClearColor(0.0, 0.0, 0.0, 0.0); 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Using AI
    glOrtho(-1.2, 1.2, -1.2, 1.2, -1.0, 1.0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Shapes");
    init();
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}