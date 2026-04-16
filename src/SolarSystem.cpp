#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <vector>

int windowWidth = 1000;
int windowHeight = 800;
const float PI = 3.14159265359f;

float timeMultiplier = 1.0f;
float earthYears = 0.0f;

// Camera controls
float cameraRot = 0.0f;
int currentView = -1; // -1 for Solar System, 0-7 for specific planets

struct Moon {
    std::string name;
    float angle;
    float speed;
    int orbitRadius;
    int radius;
    float r, g, b;
};

struct Planet {
    std::string name;
    float angle;
    float speed;
    int orbitRadius;
    int radius;
    float r, g, b;
    int numMoons;
    Moon moons[4]; 
};

Planet planets[8] = {
    {"Mercury", 4.1f, 0.040f, 60,  3,  0.5f, 0.5f, 0.5f, 0, {}}, 
    {"Venus",   2.5f, 0.030f, 100, 4,  0.9f, 0.7f, 0.2f, 0, {}}, 
    {"Earth",   1.2f, 0.020f, 150, 5,  0.0f, 0.5f, 1.0f, 1, {
        {"Moon", 0.0f, 0.080f, 12, 1, 0.7f, 0.7f, 0.7f}
    }}, 
    {"Mars",    5.8f, 0.015f, 190, 4,  1.0f, 0.2f, 0.0f, 2, {
        {"Phobos", 0.0f, 0.090f, 8, 1, 0.6f, 0.6f, 0.6f},
        {"Deimos", 3.1f, 0.070f, 11, 1, 0.5f, 0.5f, 0.5f}
    }}, 
    {"Jupiter", 0.8f, 0.008f, 280, 10, 0.8f, 0.6f, 0.4f, 4, {
        {"Io",       0.0f, 0.120f, 16, 2, 0.8f, 0.8f, 0.2f},
        {"Europa",   1.5f, 0.090f, 20, 2, 0.7f, 0.7f, 0.9f},
        {"Ganymede", 3.0f, 0.060f, 24, 2, 0.6f, 0.6f, 0.6f},
        {"Callisto", 4.5f, 0.040f, 29, 2, 0.4f, 0.4f, 0.4f}
    }}, 
    {"Saturn",  3.4f, 0.006f, 360, 9,  0.9f, 0.9f, 0.6f, 1, {
        {"Titan", 0.0f, 0.050f, 18, 2, 0.8f, 0.6f, 0.2f}
    }}, 
    {"Uranus",  1.9f, 0.004f, 440, 7,  0.5f, 0.8f, 0.9f, 0, {}}, 
    {"Neptune", 4.8f, 0.003f, 500, 7,  0.1f, 0.2f, 0.8f, 0, {}}  
};

struct Star { float x, y, z; };
std::vector<Star> stars;

void drawText3D(float x, float y, float z, const std::string& text) {
    glRasterPos3f(x, y, z);
    for (char c : text) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
}

void drawText2D(float x, float y, const std::string& text, void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2f(x, y);
    for (char c : text) glutBitmapCharacter(font, c);
}

// 1. DDA Line Drawing (Updated for 3D Plane)
void drawLineDDA(float x0, float y0, float x1, float y1) {
    float dx = x1 - x0, dy = y1 - y0;
    int steps = std::max(std::abs(dx), std::abs(dy));
    float xInc = dx / (float)steps, yInc = dy / (float)steps;
    float x = x0, y = y0;
    
    glBegin(GL_POINTS);
    for (int i = 0; i <= steps; i++) {
        glVertex3f(std::round(x), std::round(y), 0.0f);
        x += xInc; y += yInc;
    }
    glEnd();
}

// 2. Midpoint Circle Drawing (Used for orbits to satisfy requirements)
void drawCircleMidpoint(int xc, int yc, int r) {
    int x = 0, y = r, p = 1 - r;
    glBegin(GL_POINTS);
    while (x <= y) {
        glVertex3f(xc + x, yc + y, 0.0f); glVertex3f(xc - x, yc + y, 0.0f);
        glVertex3f(xc + x, yc - y, 0.0f); glVertex3f(xc - x, yc - y, 0.0f);
        glVertex3f(xc + y, yc + x, 0.0f); glVertex3f(xc - y, yc + x, 0.0f);
        glVertex3f(xc + y, yc - x, 0.0f); glVertex3f(xc - y, yc - x, 0.0f);
        x++;
        if (p < 0) p += 2 * x + 1;
        else { y--; p += 2 * (x - y) + 1; }
    }
    glEnd();
}

// 3. High-resolution solid circle (Used for planets to prevent low-poly look on zoom)
void drawSolidCircle(float xc, float yc, float r) {
    int segments = 64; // High segment count for smooth edges
    glBegin(GL_POLYGON);
    for (int i = 0; i < segments; i++) {
        float theta = 2.0f * PI * float(i) / float(segments);
        glVertex3f(xc + r * cos(theta), yc + r * sin(theta), 0.0f);
    }
    glEnd();
}

void set2DOverlay() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, windowHeight, 0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

void restore3D() {
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // 3D Camera Setup
    if (currentView == -1) {
        // Main Solar System View
        gluLookAt(0.0f, -500.0f, 350.0f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f);
        glRotatef(cameraRot, 0, 0, 1);
    } else {
        // Specific Planet View
        float px = planets[currentView].orbitRadius * cos(planets[currentView].angle);
        float py = planets[currentView].orbitRadius * sin(planets[currentView].angle);
        float dist = planets[currentView].radius * 5.0f + 30.0f; // Distance behind planet
        
        // Trail slightly behind the planet based on its orbital path
        float camX = px - dist * cos(planets[currentView].angle - 0.2f);
        float camY = py - dist * sin(planets[currentView].angle - 0.2f);
        
        gluLookAt(camX, camY, 25.0f,  px, py, 0.0f,  0.0f, 0.0f, 1.0f);
    }

    // --- Draw Starfield ---
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_POINTS);
    for (auto& s : stars) glVertex3f(s.x, s.y, s.z);
    glEnd();

    // --- Draw Sun Rays (DDA) ---
    glColor3f(0.8f, 0.6f, 0.1f);
    for (int i = 0; i < 12; i++) {
        float theta = i * (PI / 6.0f);
        drawLineDDA(0, 0, 30 * cos(theta), 30 * sin(theta));
    }

    // --- Draw Sun ---
    glColor3f(1.0f, 0.9f, 0.0f);
    drawSolidCircle(0, 0, 15);
    drawText3D(20, 0, 0, "Sun");

    // --- Draw Orbits & Planets ---
    for (int i = 0; i < 8; i++) {
        // Orbit (Midpoint Circle)
        glColor3f(planets[i].r * 0.4f, planets[i].g * 0.4f, planets[i].b * 0.4f);
        drawCircleMidpoint(0, 0, planets[i].orbitRadius);

        // Planet
        float px = planets[i].orbitRadius * cos(planets[i].angle);
        float py = planets[i].orbitRadius * sin(planets[i].angle);
        
        glColor3f(planets[i].r, planets[i].g, planets[i].b);
        drawSolidCircle(px, py, planets[i].radius);

        glColor3f(0.8f, 0.8f, 0.8f);
        drawText3D(px + planets[i].radius + 5, py + 5, 0, planets[i].name);

        // --- Draw Moons ---
        for (int j = 0; j < planets[i].numMoons; j++) {
            float mx = px + planets[i].moons[j].orbitRadius * cos(planets[i].moons[j].angle);
            float my = py + planets[i].moons[j].orbitRadius * sin(planets[i].moons[j].angle);
            
            // Orbit for Moon (Midpoint Circle)
            glColor3f(0.3f, 0.3f, 0.3f);
            glPushMatrix();
            glTranslatef(px, py, 0);
            drawCircleMidpoint(0, 0, planets[i].moons[j].orbitRadius);
            glPopMatrix();

            // Moon body
            glColor3f(planets[i].moons[j].r, planets[i].moons[j].g, planets[i].moons[j].b);
            drawSolidCircle(mx, my, planets[i].moons[j].radius);
        }
    }

    // --- Draw 2D UI Overlay ---
    set2DOverlay();
    glColor3f(1.0f, 1.0f, 1.0f);
    
    // View Controls Info
    char viewText[100];
    if (currentView == -1) snprintf(viewText, sizeof(viewText), "View: Solar System (Press 1-8 for planets)");
    else snprintf(viewText, sizeof(viewText), "View: %s (Press 0 for Solar System)", planets[currentView].name.c_str());
    drawText2D(20, 30, viewText, GLUT_BITMAP_HELVETICA_12);

    // Date Info
    const char* monthNames[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    int totalMonths = (int)(earthYears * 12);
    int currentMonth = (2 + totalMonths) % 12; // Starting in March (index 2)
    int currentYear = 2026 + (2 + totalMonths) / 12;
    
    char dateText[100];
    snprintf(dateText, sizeof(dateText), "%s %d", monthNames[currentMonth], currentYear);
    drawText2D(windowWidth / 2 - 40, 40, dateText, GLUT_BITMAP_HELVETICA_18);
    
    // Speed Info
    char infoText[100];
    snprintf(infoText, sizeof(infoText), "Speed: %.1fx (Press +/-)", timeMultiplier);
    drawText2D(windowWidth / 2 - 60, windowHeight - 30, infoText, GLUT_BITMAP_HELVETICA_12);
    
    restore3D();
    glutSwapBuffers();
}

void timer(int value) {
    for (int i = 0; i < 8; i++) {
        planets[i].angle += planets[i].speed * timeMultiplier;
        if (planets[i].angle > 2 * PI) planets[i].angle -= 2 * PI;
        else if (planets[i].angle < 0) planets[i].angle += 2 * PI;

        // Update moon angles
        for (int j = 0; j < planets[i].numMoons; j++) {
            planets[i].moons[j].angle += planets[i].moons[j].speed * timeMultiplier;
            if (planets[i].moons[j].angle > 2 * PI) planets[i].moons[j].angle -= 2 * PI;
            else if (planets[i].moons[j].angle < 0) planets[i].moons[j].angle += 2 * PI;
        }
    }
    earthYears += (planets[2].speed * timeMultiplier) / (2 * PI);
    cameraRot += 0.05f * timeMultiplier; // Gentle camera pan

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); 
}

void keyboard(unsigned char key, int x, int y) {
    if (key == '+' || key == '=') timeMultiplier += 0.5f;
    else if (key == '-' || key == '_') timeMultiplier -= 0.5f;
    else if (key == '0') currentView = -1;
    else if (key >= '1' && key <= '8') currentView = (key - '0') - 1;
}

// Reshape function to handle window resizing without aspect ratio distortion
void reshape(int w, int h) {
    if (h == 0) h = 1; // Prevent divide by zero
    windowWidth = w;
    windowHeight = h;
    
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)w / (double)h, 1.0, 2000.0);
    glMatrixMode(GL_MODELVIEW);
}

void init() {
    glClearColor(0.02f, 0.02f, 0.03f, 1.0f); // Deep space background
    glEnable(GL_DEPTH_TEST);
    
    // Generate Stars
    for (int i = 0; i < 800; i++) {
        stars.push_back({
            (float)(rand() % 2000 - 1000), 
            (float)(rand() % 2000 - 1000), 
            (float)(rand() % 1000 - 500)
        });
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("NASA's Eyes Style - 3D Solar System");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape); // Register the reshape callback
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, timer, 0);
    glutMainLoop();
    return 0;
}