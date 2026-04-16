#include <GL/glut.h>
#include <cmath>

// Angle variables for animation
float angleEarth = 0.0f;
float angleMoon = 0.0f;

// Initialization function
void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black background
    glEnable(GL_DEPTH_TEST);              // Enable depth testing for 3D
}

// Function to handle window resizing
void reshape(int w, int h) {
    if (h == 0) h = 1; // Prevent divide by zero
    float aspect = (float)w / (float)h;

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION); // Switch to projection matrix
    glLoadIdentity();
    gluPerspective(45.0, aspect, 1.0, 100.0); // 3D Perspective projection
}

// Main display function
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers
    glMatrixMode(GL_MODELVIEW); // Switch to modelview matrix
    glLoadIdentity();

    // Camera position (EyeX, EyeY, EyeZ, CenterX, CenterY, CenterZ, UpX, UpY, UpZ)
    gluLookAt(0.0, 5.0, 10.0,  // Camera position
              0.0, 0.0, 0.0,   // Look at point
              0.0, 1.0, 0.0);  // Up vector

    // 1. Draw Sun (Yellow)
    // No transformation needed, it's at (0,0,0)
    glColor3f(1.0f, 1.0f, 0.0f); 
    glutWireSphere(1.0, 20, 20); // Using WireSphere to see the "mesh"

    // 2. Draw Earth (Blue)
    glPushMatrix(); // Save current matrix (at Sun)
        
        // Rotate Earth around Sun
        glRotatef(angleEarth, 0.0f, 1.0f, 0.0f);
        glTranslatef(3.0f, 0.0f, 0.0f); // Move Earth away from Sun

        glColor3f(0.0f, 0.5f, 1.0f);
        glutWireSphere(0.5, 15, 15);

        // 3. Draw Moon (White) relative to Earth
        glPushMatrix(); // Save current matrix (at Earth)
            
            // Rotate Moon around Earth
            glRotatef(angleMoon, 0.0f, 1.0f, 0.0f);
            glTranslatef(1.0f, 0.0f, 0.0f); // Move Moon away from Earth

            glColor3f(1.0f, 1.0f, 1.0f);
            glutWireSphere(0.2, 10, 10);

        glPopMatrix(); // Restore matrix to Earth's position

    glPopMatrix(); // Restore matrix to Sun's position

    glutSwapBuffers(); // Swap the double buffers to display the frame
}

// Timer function for animation updates
void timer(int value) {
    angleEarth += 1.0f; // Earth rotates slowly
    if (angleEarth > 360) angleEarth -= 360;

    angleMoon += 4.0f; // Moon rotates faster
    if (angleMoon > 360) angleMoon -= 360;

    glutPostRedisplay(); // Request a redraw
    glutTimerFunc(16, timer, 0); // Call timer again in ~16ms (60 FPS)
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // Double buffer + Depth buffer
    glutInitWindowSize(800, 600);
    glutCreateWindow("Simple 3D Solar System");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}
