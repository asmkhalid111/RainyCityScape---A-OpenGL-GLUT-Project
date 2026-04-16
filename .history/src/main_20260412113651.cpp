#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>

// --- Config ---
int WIDTH = 800;
int HEIGHT = 600;
bool isNight = true;

// --- Data Structures ---
struct Window { float x, y; bool lit; };
struct Building { float x, w, h; std::vector<Window> windows; };
struct Drop { float x, y, speed, length; };
struct Ripple { float x, y, r, max_r; bool active; };

std::vector<Building> city;
std::vector<Drop> rain;
std::vector<Ripple> ripples;

// --- Custom Algorithms ---

// 1. DDA Line Algorithm (Used for Rain)
void drawLineDDA(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float steps = std::max(std::abs(dx), std::abs(dy));
    
    float xInc = dx / steps;
    float yInc = dy / steps;
    
    float x = x1;
    float y = y1;
    
    glBegin(GL_POINTS);
    for (int i = 0; i <= steps; i++) {
        glVertex2f(x, y);
        x += xInc;
        y += yInc;
    }
    glEnd();
}

// 2. Midpoint Circle Algorithm (Used for Moon Outline and Rain Ripples)
void drawCircleMidpoint(int xc, int yc, int r) {
    int x = 0;
    int y = r;
    int p = 1 - r;
    
    glBegin(GL_POINTS);
    while (x <= y) {
        glVertex2i(xc + x, yc + y); glVertex2i(xc - x, yc + y);
        glVertex2i(xc + x, yc - y); glVertex2i(xc - x, yc - y);
        glVertex2i(xc + y, yc + x); glVertex2i(xc - y, yc + x);
        glVertex2i(xc + y, yc - x); glVertex2i(xc - y, yc - x);
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

// Filled circle using DDA lines inside Midpoint logic (Used for solid Moon)
void drawFilledCircle(int xc, int yc, int r) {
    int x = 0, y = r, p = 1 - r;
    while (x <= y) {
        drawLineDDA(xc - x, yc + y, xc + x, yc + y);
        drawLineDDA(xc - x, yc - y, xc + x, yc - y);
        drawLineDDA(xc - y, yc + x, xc + y, yc + x);
        drawLineDDA(xc - y, yc - x, xc + y, yc - x);
        x++;
        if (p < 0) { p += 2 * x + 1; } 
        else { y--; p += 2 * (x - y) + 1; }
    }
}

// --- Generators ---
void generateCity() {
    city.clear();
    float currentX = 0;
    while (currentX < WIDTH) {
        Building b;
        b.x = currentX;
        b.w = 40 + rand() % 60;
        b.h = 100 + rand() % 300;
        
        // Generate windows
        for (float wx = b.x + 10; wx < b.x + b.w - 10; wx += 20) {
            for (float wy = 20; wy < b.h - 20; wy += 30) {
                Window win;
                win.x = wx; win.y = wy;
                win.lit = (rand() % 5 == 0); // 20% chance to be lit
                b.windows.push_back(win);
            }
        }
        city.push_back(b);
        currentX += b.w + (rand() % 10); // Small gaps between buildings
    }
}

void initRain(int count) {
    rain.clear();
    for (int i = 0; i < count; i++) {
        Drop d;
        d.x = rand() % WIDTH;
        d.y = HEIGHT + rand() % HEIGHT; // Start staggered above screen
        d.speed = 5 + (rand() % 10);
        d.length = 10 + (rand() % 15);
        rain.push_back(d);
    }
}

void spawnRipple(float x) {
    Ripple r = {x, 10.0f, 1.0f, 10.0f + (rand() % 10), true};
    ripples.push_back(r);
}

// --- Main OpenGL Functions ---
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // 1. Draw Sky Background
    if (isNight) {
        glClearColor(0.05f, 0.05f, 0.15f, 1.0f); // Dark Blue
        glColor3f(0.9f, 0.9f, 0.8f);
        drawFilledCircle(650, 450, 40); // Moon
    } else {
        glClearColor(0.3f, 0.15f, 0.2f, 1.0f); // Dusk Purple/Orange
        glColor3f(1.0f, 0.5f, 0.2f);
        drawFilledCircle(650, 450, 40); // Sun
    }

    // 2. Draw City
    for (size_t i = 0; i < city.size(); i++) {
        // Silhouette
        glColor3f(0.1f, 0.1f, 0.12f); 
        glBegin(GL_QUADS);
            glVertex2f(city[i].x, 0);
            glVertex2f(city[i].x + city[i].w, 0);
            glVertex2f(city[i].x + city[i].w, city[i].h);
            glVertex2f(city[i].x, city[i].h);
        glEnd();

        // Windows
        for (size_t j = 0; j < city[i].windows.size(); j++) {
            if (city[i].windows[j].lit) {
                glColor3f(0.9f, 0.8f, 0.2f); // Lit yellow
            } else {
                glColor3f(0.05f, 0.05f, 0.05f); // Dark
            }
            glBegin(GL_QUADS);
                glVertex2f(city[i].windows[j].x, city[i].windows[j].y);
                glVertex2f(city[i].windows[j].x + 10, city[i].windows[j].y);
                glVertex2f(city[i].windows[j].x + 10, city[i].windows[j].y + 15);
                glVertex2f(city[i].windows[j].x, city[i].windows[j].y + 15);
            glEnd();
        }
    }

    // 3. Draw Rain (using DDA)
    glColor3f(0.4f, 0.6f, 0.9f); // Rain blue
    for (size_t i = 0; i < rain.size(); i++) {
        drawLineDDA(rain[i].x, rain[i].y, rain[i].x, rain[i].y - rain[i].length);
    }

    // 4. Draw Ripples (using Midpoint Circle)
    glColor3f(0.5f, 0.7f, 1.0f);
    for (size_t i = 0; i < ripples.size(); i++) {
        if (ripples[i].active) {
            // Squished circle for perspective
            drawCircleMidpoint(ripples[i].x, ripples[i].y, ripples[i].r);
        }
    }

    glutSwapBuffers();
}

void update(int value) {
    // Move rain
    for (size_t i = 0; i < rain.size(); i++) {
        rain[i].y -= rain[i].speed;
        if (rain[i].y < 0) {
            spawnRipple(rain[i].x);
            rain[i].y = HEIGHT + (rand() % 100);
            rain[i].x = rand() % WIDTH;
        }
    }

    // Grow ripples
    for (size_t i = 0; i < ripples.size(); i++) {
        if (ripples[i].active) {
            ripples[i].r += 0.5f;
            if (ripples[i].r > ripples[i].max_r) ripples[i].active = false;
        }
    }

    // Clean up inactive ripples to save memory
    ripples.erase(std::remove_if(ripples.begin(), ripples.end(), 
                  [](const Ripple& r) { return !r.active; }), ripples.end());

    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // ~60 FPS
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'r': // Regenerate City
        case 'R':
            generateCity();
            break;
        case '+': // Increase Rain
            initRain(rain.size() + 50);
            break;
        case '-': // Decrease Rain
            if (rain.size() >= 50) initRain(rain.size() - 50);
            break;
        case 'n': // Toggle Night/Dusk
        case 'd':
            isNight = !isNight;
            break;
        case 27: // ESC to exit
            exit(0);
            break;
    }
}

void init() {
    glClearColor(0.05f, 0.05f, 0.15f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIDTH, 0, HEIGHT);
}

int main(int argc, char** argv) {
    srand(time(0));
    generateCity();
    initRain(200);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Rainy Cityscape - DDA & Midpoint Algorithms");

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, update, 0);

    // Terminal Instructions
    std::cout << "--- Controls --- \n";
    std::cout << "'r' : Regenerate new city skyline\n";
    std::cout << "'+' : Increase rain intensity\n";
    std::cout << "'-' : Decrease rain intensity\n";
    std::cout << "'n' / 'd' : Toggle Night/Dusk\n";
    std::cout << "ESC : Exit\n";

    glutMainLoop();
    return 0;
}