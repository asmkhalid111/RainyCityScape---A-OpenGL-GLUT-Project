#include <GL/glut.h>
#include <iostream>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <string>
#include <cstdio>

#define PI 3.14159265

// Window dimensions
const int WIDTH = 600;
const int HEIGHT = 600;
const int CENTER_X = WIDTH / 2;
const int CENTER_Y = HEIGHT / 2;

// Helper to draw text
void drawString(int x, int y, std::string text) {
    glRasterPos2i(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
}

// --- DDA LINE DRAWING ALGORITHM ---
void ddaLine(int x1, int y1, int x2, int y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float steps = std::max(std::abs(dx), std::abs(dy));
    
    float xInc = dx / steps;
    float yInc = dy / steps;
    
    float x = x1;
    float y = y1;
    
    glBegin(GL_POINTS);
    for (int i = 0; i <= steps; i++) {
        glVertex2i(std::round(x), std::round(y));
        x += xInc;
        y += yInc;
    }
    glEnd();
}

// Helper for Midpoint Circle
void plotCirclePoints(int xc, int yc, int x, int y) {
    glBegin(GL_POINTS);
    glVertex2i(xc + x, yc + y);
    glVertex2i(xc - x, yc + y);
    glVertex2i(xc + x, yc - y);
    glVertex2i(xc - x, yc - y);
    glVertex2i(xc + y, yc + x);
    glVertex2i(xc - y, yc + x);
    glVertex2i(xc + y, yc - x);
    glVertex2i(xc - y, yc - x);
    glEnd();
}

// --- MIDPOINT CIRCLE DRAWING ALGORITHM ---
void midpointCircle(int xc, int yc, int r) {
    int x = 0;
    int y = r;
    int p = 1 - r;
    
    plotCirclePoints(xc, yc, x, y);
    
    while (x < y) {
        x++;
        if (p < 0) {
            p += 2 * x + 1;
        } else {
            y--;
            p += 2 * (x - y) + 1;
        }
        plotCirclePoints(xc, yc, x, y);
    }
}

// Draw clock face and hours markers
void drawClockFace() {
    int radius = 200;
    
    // Draw outer circle using Midpoint Algorithm
    glColor3f(1.0, 1.0, 1.0); // White
    midpointCircle(CENTER_X, CENTER_Y, radius);
    midpointCircle(CENTER_X, CENTER_Y, radius - 1); // Thicker border
    
    // Draw 60 minute/hour tick marks using DDA Algorithm
    for(int i = 0; i < 60; i++) {
        float angle = i * (PI / 30.0);
        int innerRadius = (i % 5 == 0) ? radius - 15 : radius - 5;
        
        if (i % 5 == 0) glColor3f(1.0, 1.0, 1.0); // White for hours
        else glColor3f(0.5, 0.5, 0.5); // Gray for minutes
        
        int x1 = CENTER_X + innerRadius * cos(angle);
        int y1 = CENTER_Y + innerRadius * sin(angle);
        int x2 = CENTER_X + radius * cos(angle);
        int y2 = CENTER_Y + radius * sin(angle);
        ddaLine(x1, y1, x2, y2);
    }
    
    // Draw hour numbers 1-12
    glColor3f(1.0, 1.0, 0.0); // Yellow numbers
    for(int i = 1; i <= 12; i++) {
        float angle = (PI / 2.0) - i * (PI / 6.0);
        int textRadius = radius - 35; // Position inside tick marks
        int x = CENTER_X + textRadius * cos(angle) - 8;
        int y = CENTER_Y + textRadius * sin(angle) - 6;
        drawString(x, y, std::to_string(i));
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    drawClockFace();
    
    // Get current system time
    time_t t = time(0);
    struct tm * now = localtime(&t);
    int h = now->tm_hour;
    int m = now->tm_min;
    int s = now->tm_sec;
    
    // Calculate angles (12 o'clock is PI/2, moving clockwise)
    float secAngle = (PI / 2.0) - (s / 60.0) * 2.0 * PI;
    float minAngle = (PI / 2.0) - (m / 60.0) * 2.0 * PI - (s / 3600.0) * 2.0 * PI;
    float hourAngle = (PI / 2.0) - ((h % 12) / 12.0) * 2.0 * PI - (m / 720.0) * 2.0 * PI;
    
    // Draw Second Hand (Red, Longest)
    glColor3f(1.0, 0.0, 0.0);
    ddaLine(CENTER_X, CENTER_Y, CENTER_X + 180 * cos(secAngle), CENTER_Y + 180 * sin(secAngle));
    
    // Draw Minute Hand (Green, Medium)
    glColor3f(0.0, 1.0, 0.0);
    ddaLine(CENTER_X, CENTER_Y, CENTER_X + 150 * cos(minAngle), CENTER_Y + 150 * sin(minAngle));
    
    // Draw Hour Hand (Blue, Shortest)
    glColor3f(0.3, 0.5, 1.0);
    ddaLine(CENTER_X, CENTER_Y, CENTER_X + 100 * cos(hourAngle), CENTER_Y + 100 * sin(hourAngle));
    
    // Draw Digital Time at the bottom
    char timeStr[20];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", h, m, s);
    glColor3f(1.0, 1.0, 1.0);
    drawString(CENTER_X - 35, CENTER_Y - 250, timeStr);
    
    glutSwapBuffers();
}

// Update screen every 1000ms (1 second)
void timer(int) {
    glutPostRedisplay();
    glutTimerFunc(1000, timer, 0);
}

void init() {
    glClearColor(0.0, 0.0, 0.0, 1.0); // Black background
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0, WIDTH, 0, HEIGHT);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Analog Clock - DDA & Midpoint Algorithms");
    
    init();
    glutDisplayFunc(display);
    glutTimerFunc(0, timer, 0);
    
    glutMainLoop();
    return 0;
}