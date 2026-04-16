#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/glut.h>
#include <cmath>

// Relative Scale Constants
const float SUN_RADIUS = 1.5f;
const float EARTH_RADIUS = 0.4f;    // Earth is much smaller than Sun
const float MOON_RADIUS = 0.12f;   // Moon is ~1/4 size of Earth
const float EARTH_ORBIT_DIST = 5.0f;
const float MOON_ORBIT_DIST = 0.8f;

float angleEarth = 0.0f;
float angleMoon = 0.0f;

void init() {
    glClearColor(0.02f, 0.02f, 0.05f, 1.0f); // Very dark blue (Space)
    glEnable(GL_DEPTH_TEST);
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    float aspect = (float)w / (float)h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Pushed Z-far to 100.0 to ensure nothing gets clipped
    gluPerspective(45.0, aspect, 1.0, 100.0);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Pulled camera back to see the larger orbit
    gluLookAt(0.0, 8.0, 15.0,  
              0.0, 0.0, 0.0,   
              0.0, 1.0, 0.0);

    // 1. Draw Sun (Yellow)
    glColor3f(1.0f, 0.8f, 0.0f); 
    glutWireSphere(SUN_RADIUS, 30, 30); 

    // 2. Draw Earth (Blue)
    glPushMatrix(); 
        glRotatef(angleEarth, 0.0f, 1.0f, 0.0f); // Orbit rotation
        glTranslatef(EARTH_ORBIT_DIST, 0.0f, 0.0f); 

        // Earth's own rotation (optional detail)
        glPushMatrix();
            glRotatef(angleEarth * 2, 0.0f, 1.0f, 0.0f); 
            glColor3f(0.2f, 0.4f, 1.0f);
            glutWireSphere(EARTH_RADIUS, 20, 20);
        glPopMatrix();

        // 3. Draw Moon (Light Grey) relative to Earth
        glPushMatrix(); 
            glRotatef(angleMoon, 0.0f, 1.0f, 0.0f); // Moon orbit
            glTranslatef(MOON_ORBIT_DIST, 0.0f, 0.0f); 

            glColor3f(0.8f, 0.8f, 0.8f);
            glutWireSphere(MOON_RADIUS, 10, 10);
        glPopMatrix(); 

    glPopMatrix(); 

    glutSwapBuffers();
}

void timer(int value) {
    angleEarth += 0.5f; // Slower, more majestic movement
    if (angleEarth > 360) angleEarth -= 360;

    angleMoon += 2.0f; // Moon still moves faster than Earth
    if (angleMoon > 360) angleMoon -= 360;

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 700);
    glutCreateWindow("Scalable 3D Solar System");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}