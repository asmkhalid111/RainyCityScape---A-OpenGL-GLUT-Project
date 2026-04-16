#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <string>
#include <cstdio>

const int WIDTH = 800;
const int HEIGHT = 800;
const float PI = 3.14159265359f;

// Time and Speed controls
float timeMultiplier = 1.0f;
float earthYears = 0.0f;

// Structure to hold planet data
struct Planet {
    std::string name;
    float angle;
    float speed;
    int orbitRadius;
    int radius;
    float r, g, b;
};

// Initialize all 8 planets with their signature colors, relative sizes/orbits, and scattered starting angles (in radians)
Planet planets[8] = {
    {"Mercury", 4.1f, 0.040f, 50,  4,  0.5f, 0.5f, 0.5f}, // Dark Gray
    {"Venus",   2.5f, 0.030f, 80,  6,  0.9f, 0.7f, 0.2f}, // Yellow-Orange
    {"Earth",   1.2f, 0.020f, 115, 7,  0.0f, 0.5f, 1.0f}, // Blue
    {"Mars",    5.8f, 0.015f, 145, 5,  1.0f, 0.2f, 0.0f}, // Red
    {"Jupiter", 0.8f, 0.008f, 200, 16, 0.8f, 0.6f, 0.4f}, // Brown/Beige
    {"Saturn",  3.4f, 0.006f, 260, 14, 0.9f, 0.9f, 0.6f}, // Pale Yellow
    {"Uranus",  1.9f, 0.004f, 320, 10, 0.5f, 0.8f, 0.9f}, // Light Cyan
    {"Neptune", 4.8f, 0.003f, 370, 10, 0.1f, 0.2f, 0.8f}  // Dark Blue
};

// Moon special case for Earth
float angleMoon = 0.0f;
const float speedMoon = 0.08f;

// Helper to draw text using GLUT
void drawText(float x, float y, const std::string& text) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }
}

// 1. DDA Line Drawing Algorithm
void drawLineDDA(float x0, float y0, float x1, float y1) {
    float dx = x1 - x0;
    float dy = y1 - y0;
    int steps = std::max(std::abs(dx), std::abs(dy));
    
    float xInc = dx / (float)steps;
    float yInc = dy / (float)steps;
    
    float x = x0;
    float y = y0;
    
    glBegin(GL_POINTS);
    for (int i = 0; i <= steps; i++) {
        glVertex2i(std::round(x), std::round(y));
        x += xInc;
        y += yInc;
    }
    glEnd();
}

// 2. Midpoint Circle Drawing Algorithm
void drawCircleMidpoint(int xc, int yc, int r) {
    int x = 0;
    int y = r;
    int p = 1 - r;
    
    glBegin(GL_POINTS);
    while (x <= y) {
        glVertex2i(xc + x, yc + y);
        glVertex2i(xc - x, yc + y);
        glVertex2i(xc + x, yc - y);
        glVertex2i(xc - x, yc - y);
        glVertex2i(xc + y, yc + x);
        glVertex2i(xc - y, yc + x);
        glVertex2i(xc + y, yc - x);
        glVertex2i(xc - y, yc - x);
        
        x++;
        if (p < 0) {
            p += 2 * x + 1;
        } else {
            y--;
            p += 2 * (x - y) + 1;
        }
    }
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // --- Draw Sun Rays using DDA ---
    glColor3f(1.0f, 0.5f, 0.0f); // Orange rays
    for (int i = 0; i < 12; i++) {
        float theta = i * (PI / 6.0f);
        drawLineDDA(0, 0, 45 * cos(theta), 45 * sin(theta));
    }

    // --- Draw Sun using Midpoint Circle ---
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow Sun
    drawCircleMidpoint(0, 0, 25);

    // --- Draw Orbits using Midpoint Circle ---
    glColor3f(0.2f, 0.2f, 0.2f); // Dark Grey
    for (int i = 0; i < 8; i++) {
        drawCircleMidpoint(0, 0, planets[i].orbitRadius);
    }

    // --- Draw Planets, Names, and Moon ---
    for (int i = 0; i < 8; i++) {
        float px = planets[i].orbitRadius * cos(planets[i].angle);
        float py = planets[i].orbitRadius * sin(planets[i].angle);

        // Planet body
        glColor3f(planets[i].r, planets[i].g, planets[i].b);
        drawCircleMidpoint(px, py, planets[i].radius);

        // Planet name
        glColor3f(1.0f, 1.0f, 1.0f); // White text
        drawText(px + planets[i].radius + 4, py + 4, planets[i].name);

        // Draw Moon if the planet is Earth
        if (planets[i].name == "Earth") {
            float moonX = px + 15 * cos(angleMoon);
            float moonY = py + 15 * sin(angleMoon);
            glColor3f(0.7f, 0.7f, 0.7f); // Grey Moon
            drawCircleMidpoint(moonX, moonY, 2);
        }
    }

    // --- Draw UI Info (Year and Speed) ---
    glColor3f(1.0f, 1.0f, 1.0f);
    char infoText[100];
    snprintf(infoText, sizeof(infoText), "Earth Years: %.1f  |  Speed: %.1fx  (Press +/- to adjust)", earthYears, timeMultiplier);
    drawText(-WIDTH / 2 + 20, -HEIGHT / 2 + 20, infoText);

    glutSwapBuffers();
}

void timer(int value) {
    // Update angles for planets
    for (int i = 0; i < 8; i++) {
        planets[i].angle += planets[i].speed * timeMultiplier;
        // Keep angles bounded
        if (planets[i].angle > 2 * PI) planets[i].angle -= 2 * PI;
        else if (planets[i].angle < 0) planets[i].angle += 2 * PI;
    }

    // Update Earth Years based on Earth's speed (Earth is index 2)
    earthYears += (planets[2].speed * timeMultiplier) / (2 * PI);

    // Update Moon
    angleMoon += speedMoon * timeMultiplier;
    if (angleMoon > 2 * PI) angleMoon -= 2 * PI;
    else if (angleMoon < 0) angleMoon += 2 * PI;

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); // ~60 FPS
}

// Keyboard controller for speed
void keyboard(unsigned char key, int x, int y) {
    if (key == '+' || key == '=') {
        timeMultiplier += 0.5f;
    } else if (key == '-' || key == '_') {
        timeMultiplier -= 0.5f;
    }
}

void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black background
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Set coordinate system from -400 to 400 (center is 0,0)
    gluOrtho2D(-WIDTH / 2, WIDTH / 2, -HEIGHT / 2, HEIGHT / 2); 
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Solar System - All Planets");

    init();
    
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard); // Register keyboard callback
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}