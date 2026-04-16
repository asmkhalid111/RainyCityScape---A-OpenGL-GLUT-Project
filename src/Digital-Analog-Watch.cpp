#include <GL/glut.h>
#include <iostream>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <string>
#include <cstdio>

#define PI 3.14159265

// Window dimensions
int windowWidth = 600;
int windowHeight = 600;
int CENTER_X = windowWidth / 2;
int CENTER_Y = windowHeight / 2;

// --- Global State for Keyboard Features ---
long timeOffset = 0;       // Manual adjustment in seconds
bool isPaused = false;     // Pause the clock motion
int colorTheme = 0;        // 0: White, 1: Cyan, 2: Gold, 3: Green, 4: Pink, 5: Orange

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

void drawBox(int x, int y, int w, int h) {
    ddaLine(x, y, x + w, y);
    ddaLine(x + w, y, x + w, y + h);
    ddaLine(x + w, y + h, x, y + h);
    ddaLine(x, y + h, x, y);
}

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
        if (p < 0) p += 2 * x + 1;
        else {
            y--;
            p += 2 * (x - y) + 1;
        }
        plotCirclePoints(xc, yc, x, y);
    }
}

// Sets the drawing color based on the selected theme
void setThemeColor() {
    switch (colorTheme) {
        case 0: glColor3f(1.0, 1.0, 1.0); break; // White
        case 1: glColor3f(0.0, 1.0, 1.0); break; // Cyan
        case 2: glColor3f(1.0, 0.8, 0.0); break; // Gold
        case 3: glColor3f(0.0, 1.0, 0.0); break; // Matrix Green
        case 4: glColor3f(1.0, 0.0, 1.0); break; // Pink/Magenta
        case 5: glColor3f(1.0, 0.5, 0.0); break; // Orange
        default: glColor3f(1.0, 1.0, 1.0);
    }
}

void drawClockFace(struct tm * now) {
    int radius = 220;
    setThemeColor();

    // Face Border
    midpointCircle(CENTER_X, CENTER_Y, radius);
    midpointCircle(CENTER_X, CENTER_Y, radius - 1);

    // Ticks
    for(int i = 0; i < 60; i++) {
        float angle = i * (PI / 30.0);
        int innerRadius = (i % 5 == 0) ? radius - 15 : radius - 5;
        int x1 = CENTER_X + innerRadius * cos(angle);
        int y1 = CENTER_Y + innerRadius * sin(angle);
        int x2 = CENTER_X + radius * cos(angle);
        int y2 = CENTER_Y + radius * sin(angle);
        ddaLine(x1, y1, x2, y2);
    }

    // Numbers
    for(int i = 1; i <= 12; i++) {
        float angle = (PI / 2.0) - i * (PI / 6.0);
        int textRadius = radius - 35;
        int x = CENTER_X + textRadius * cos(angle) - 8;
        int y = CENTER_Y + textRadius * sin(angle) - 6;
        drawString(x, y, std::to_string(i));
    }

    // UI Boxes
    const char* days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    const char* months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

    glColor3f(0.4, 0.4, 0.4); // Box borders slightly dimmer
    drawBox(CENTER_X - 120, CENTER_Y - 15, 45, 30); // Day
    drawBox(CENTER_X - 75, CENTER_Y - 15, 35, 30);  // Date
    drawBox(CENTER_X + 45, CENTER_Y - 15, 55, 30);  // Month
    drawBox(CENTER_X - 30, CENTER_Y - 100, 60, 30); // Year

    setThemeColor();
    drawString(CENTER_X - 115, CENTER_Y - 6, days[now->tm_wday]);
    drawString(CENTER_X - 67, CENTER_Y - 6, std::to_string(now->tm_mday));
    drawString(CENTER_X + 52, CENTER_Y - 6, months[now->tm_mon]);
    drawString(CENTER_X - 22, CENTER_Y - 91, std::to_string(now->tm_year + 1900));

    midpointCircle(CENTER_X, CENTER_Y, 3); // Center hub
}

void handleKey(unsigned char key, int x, int y) {
    if (key >= '1' && key <= '6') {
        colorTheme = key - '1'; // Maps '1'-'6' to 0-5
    }
    switch (key) {
        case 'p': case 'P': isPaused = !isPaused; break;
        case 'r': case 'R': timeOffset = 0; break;
        case 27: exit(0); break;
    }
    glutPostRedisplay();
}

void handleSpecialKey(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:    timeOffset += 3600; break;
        case GLUT_KEY_DOWN:  timeOffset -= 3600; break;
        case GLUT_KEY_RIGHT: timeOffset += 60;   break;
        case GLUT_KEY_LEFT:  timeOffset -= 60;   break;
    }
    glutPostRedisplay();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    static time_t currentDisplayTime;
    if (!isPaused) {
        currentDisplayTime = time(0) + timeOffset;
    }

    struct tm * now = localtime(&currentDisplayTime);
    int h = now->tm_hour;
    int m = now->tm_min;
    int s = now->tm_sec;

    drawClockFace(now);

    // Precise ticking angles (no sub-second fraction)
    float secAngle = (PI / 2.0) - (s / 60.0) * 2.0 * PI;
    float minAngle = (PI / 2.0) - (m / 60.0) * 2.0 * PI - (s / 3600.0) * 2.0 * PI;
    float hourAngle = (PI / 2.0) - ((h % 12) / 12.0) * 2.0 * PI - (m / 720.0) * 2.0 * PI;

    setThemeColor();
    ddaLine(CENTER_X, CENTER_Y, CENTER_X + 110 * cos(hourAngle), CENTER_Y + 110 * sin(hourAngle)); // Hour
    ddaLine(CENTER_X, CENTER_Y, CENTER_X + 160 * cos(minAngle), CENTER_Y + 160 * sin(minAngle));   // Minute

    glColor3f(1.0, 0.0, 0.0); // Second hand stays red
    ddaLine(CENTER_X, CENTER_Y, CENTER_X + 180 * cos(secAngle), CENTER_Y + 180 * sin(secAngle));

    char timeStr[40];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d %s", h, m, s, isPaused ? "[PAUSED]" : "");
    setThemeColor();
    drawString(CENTER_X - 60, CENTER_Y - 260, timeStr);

    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW);
    CENTER_X = w / 2;
    CENTER_Y = h / 2;
}

void timer(int v) {
    glutPostRedisplay();
    glutTimerFunc(100, timer, 0); // 10 FPS is plenty for a ticking clock
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Interactive Analog Clock - DDA & Midpoint");

    glClearColor(0.0, 0.0, 0.0, 1.0);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(handleKey);
    glutSpecialFunc(handleSpecialKey);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}
