#include <GL/glut.h>
#include <iostream>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <string>
#include <cstdio>

#define PI 3.14159265

// Window dimensions (Now dynamic)
int windowWidth = 600;
int windowHeight = 600;
int CENTER_X = windowWidth / 2;
int CENTER_Y = windowHeight / 2;

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

// Helper to draw a box using DDA Line
void drawBox(int x, int y, int w, int h) {
    ddaLine(x, y, x + w, y);
    ddaLine(x + w, y, x + w, y + h);
    ddaLine(x + w, y + h, x, y + h);
    ddaLine(x, y + h, x, y);
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

// Draw clock face, markings, and UI boxes
void drawClockFace(struct tm * now) {
    int radius = 220;
    
    // Draw outer circle using Midpoint Algorithm
    glColor3f(1.0, 1.0, 1.0); // White
    midpointCircle(CENTER_X, CENTER_Y, radius);
    midpointCircle(CENTER_X, CENTER_Y, radius - 1); // Thicker border
    midpointCircle(CENTER_X, CENTER_Y, radius - 2); 
    
    // Draw minute/hour tick marks
    for(int i = 0; i < 60; i++) {
        float angle = i * (PI / 30.0);
        int innerRadius = (i % 5 == 0) ? radius - 15 : radius - 5;
        
        glColor3f(1.0, 1.0, 1.0); // All ticks white like the image
        
        int x1 = CENTER_X + innerRadius * cos(angle);
        int y1 = CENTER_Y + innerRadius * sin(angle);
        int x2 = CENTER_X + radius * cos(angle);
        int y2 = CENTER_Y + radius * sin(angle);
        ddaLine(x1, y1, x2, y2);
    }
    
    // Draw hour numbers 1-12
    glColor3f(1.0, 1.0, 1.0); // White numbers
    for(int i = 1; i <= 12; i++) {
        float angle = (PI / 2.0) - i * (PI / 6.0);
        int textRadius = radius - 35; // Position inside tick marks
        int x = CENTER_X + textRadius * cos(angle) - 8;
        int y = CENTER_Y + textRadius * sin(angle) - 6;
        drawString(x, y, std::to_string(i));
    }
    
    // --- UI Data Boxes ---
    const char* days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    const char* months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
    
    glColor3f(0.5, 0.5, 0.5); // Gray for box borders
    
    // Left side: Day & Date Boxes
    drawBox(CENTER_X - 120, CENTER_Y - 15, 45, 30); // Day box
    drawBox(CENTER_X - 75, CENTER_Y - 15, 35, 30);  // Date box
    
    // Right side: Month Box
    drawBox(CENTER_X + 45, CENTER_Y - 15, 55, 30);  // Month box
    
    // Bottom: Year Box
    drawBox(CENTER_X - 30, CENTER_Y - 100, 60, 30); // Year box
    
    glColor3f(1.0, 1.0, 1.0); // White for text
    
    // Draw Text in Boxes
    drawString(CENTER_X - 115, CENTER_Y - 6, days[now->tm_wday]);
    drawString(CENTER_X - 67, CENTER_Y - 6, std::to_string(now->tm_mday));
    drawString(CENTER_X + 52, CENTER_Y - 6, months[now->tm_mon]);
    drawString(CENTER_X - 22, CENTER_Y - 91, std::to_string(now->tm_year + 1900));
    
    // Draw Center Dot
    midpointCircle(CENTER_X, CENTER_Y, 3);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Get current system time
    time_t t = time(0);
    struct tm * now = localtime(&t);
    int h = now->tm_hour;
    int m = now->tm_min;
    int s = now->tm_sec;
    
    drawClockFace(now);
    
    // Calculate angles (12 o'clock is PI/2, moving clockwise)
    float secAngle = (PI / 2.0) - (s / 60.0) * 2.0 * PI;
    float minAngle = (PI / 2.0) - (m / 60.0) * 2.0 * PI - (s / 3600.0) * 2.0 * PI;
    float hourAngle = (PI / 2.0) - ((h % 12) / 12.0) * 2.0 * PI - (m / 720.0) * 2.0 * PI;
    
    // Draw Hour Hand (White, Shortest)
    glColor3f(1.0, 1.0, 1.0);
    ddaLine(CENTER_X, CENTER_Y, CENTER_X + 110 * cos(hourAngle), CENTER_Y + 110 * sin(hourAngle));
    
    // Draw Minute Hand (White, Medium)
    glColor3f(1.0, 1.0, 1.0);
    ddaLine(CENTER_X, CENTER_Y, CENTER_X + 160 * cos(minAngle), CENTER_Y + 160 * sin(minAngle));
    
    // Draw Second Hand (Red, Longest)
    glColor3f(1.0, 0.0, 0.0);
    ddaLine(CENTER_X, CENTER_Y, CENTER_X + 180 * cos(secAngle), CENTER_Y + 180 * sin(secAngle));
    
    // Draw Digital Time at the bottom
    char timeStr[20];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", h, m, s);
    glColor3f(1.0, 1.0, 1.0);
    drawString(CENTER_X - 35, CENTER_Y - 260, timeStr);
    
    glutSwapBuffers();
}

// Handle window resizing
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW);
    
    // Update center coordinates so the clock stays centered
    CENTER_X = w / 2;
    CENTER_Y = h / 2;
}

// Update screen every 1000ms (1 second)
void timer(int) {
    glutPostRedisplay();
    glutTimerFunc(1000, timer, 0);
}

void init() {
    glClearColor(0.0, 0.0, 0.0, 1.0); // Black background
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Analog Clock - DDA & Midpoint Algorithms");
    
    init();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape); // Register the reshape callback
    glutTimerFunc(0, timer, 0);
    
    glutMainLoop();
    return 0;
}