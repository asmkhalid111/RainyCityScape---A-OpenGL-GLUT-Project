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
bool enableSplashes = true;
bool enableClouds = true;
int lightningTimer = 0;
bool pixelatedMoon = true;
bool isPaused = false;

// --- Data Structures ---
struct Window { float x, y; bool lit; };
struct Building { float x, w, h; std::vector<Window> windows; };
struct Drop { float x, y, speed, length; };
struct Ripple { float x, y, r, max_r; bool active; };
struct Cloud { float x, y, speed, scale; int type; };
struct Point { float x, y; };

std::vector<Building> city;
std::vector<Drop> rain;
std::vector<Ripple> ripples;
std::vector<Cloud> clouds;
std::vector<Point> lightningPath;

// --- Custom Algorithms ---

// 1. DDA Line Algorithm (Used for Rain & Lightning)
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

// Filled circle using DDA lines inside Midpoint logic (Used for solid Moon and Clouds)
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

// Smooth filled circle using standard OpenGL polygons
void drawSmoothFilledCircle(float cx, float cy, float r, int num_segments) {
    glBegin(GL_POLYGON);
    for(int ii = 0; ii < num_segments; ii++) {
        float theta = 2.0f * 3.1415926f * float(ii) / float(num_segments);
        float x = r * cos(theta);
        float y = r * sin(theta);
        glVertex2f(x + cx, y + cy);
    }
    glEnd();
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

void initClouds(int count) {
    clouds.clear();
    for (int i = 0; i < count; i++) {
        Cloud c;
        c.x = rand() % WIDTH;
        c.y = HEIGHT - 40 - (rand() % 60);
        c.speed = 0.2f + (rand() % 10) / 10.0f;
        c.scale = 0.8f + (rand() % 10) / 10.0f;
        c.type = rand() % 2; // 2 different shape variations
        clouds.push_back(c);
    }
}

// --- Main OpenGL Functions ---
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // 1. Draw Sky Background with Lightning Flash
    if (isNight) {
        if (lightningTimer > 0 && enableClouds) {
            glClearColor(0.7f, 0.7f, 0.9f, 1.0f); // Lightning flash
        } else {
            glClearColor(0.05f, 0.05f, 0.15f, 1.0f); // Dark Blue
        }
        glColor3f(0.9f, 0.9f, 0.8f);
        if (pixelatedMoon) drawFilledCircle(650, 450, 40); // Pixelated Moon
        else drawSmoothFilledCircle(650, 450, 40, 100);    // Smooth Moon
    } else {
        if (lightningTimer > 0 && enableClouds) {
            glClearColor(1.0f, 0.9f, 0.7f, 1.0f); // Lightning flash
        } else {
            glClearColor(0.3f, 0.15f, 0.2f, 1.0f); // Dusk Purple/Orange
        }
        glColor3f(1.0f, 0.5f, 0.2f);
        if (pixelatedMoon) drawFilledCircle(650, 450, 40); // Pixelated Sun
        else drawSmoothFilledCircle(650, 450, 40, 100);    // Smooth Sun
    }

    // 1.5 Draw Animated Lightning Line
    if (lightningTimer > 0 && enableClouds && lightningPath.size() > 1) {
        glColor3f(1.0f, 1.0f, 0.9f); // Bright yellow-white
        for (size_t i = 0; i < lightningPath.size() - 1; i++) {
            // Draw thicker line by offsetting x
            drawLineDDA(lightningPath[i].x, lightningPath[i].y, lightningPath[i+1].x, lightningPath[i+1].y);
            drawLineDDA(lightningPath[i].x + 1, lightningPath[i].y, lightningPath[i+1].x + 1, lightningPath[i+1].y);
            drawLineDDA(lightningPath[i].x - 1, lightningPath[i].y, lightningPath[i+1].x - 1, lightningPath[i+1].y);
        }
    }

    // 2. Draw Clouds
    if (enableClouds) {
        if (lightningTimer > 0) glColor3f(0.5f, 0.5f, 0.6f); // Bright clouds during flash
        else glColor3f(0.15f, 0.15f, 0.2f); // Dark thunder clouds

        for (size_t i = 0; i < clouds.size(); i++) {
            float cx = clouds[i].x;
            float cy = clouds[i].y;
            float s = clouds[i].scale;
            
            if (clouds[i].type == 0) {
                // Classic cluster
                drawFilledCircle(cx, cy, 40 * s);
                drawFilledCircle(cx - 35 * s, cy - 10 * s, 30 * s);
                drawFilledCircle(cx + 35 * s, cy - 10 * s, 30 * s);
                drawFilledCircle(cx, cy + 20 * s, 25 * s);
            } else {
                // Long, flat cloud
                drawFilledCircle(cx, cy, 30 * s);
                drawFilledCircle(cx - 40 * s, cy, 25 * s);
                drawFilledCircle(cx + 40 * s, cy, 25 * s);
                drawFilledCircle(cx - 70 * s, cy - 5 * s, 20 * s);
                drawFilledCircle(cx + 70 * s, cy - 5 * s, 20 * s);
            }
        }
    }

    // 3. Draw City
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

    // 4. Draw Rain (using DDA)
    glColor3f(0.4f, 0.6f, 0.9f); // Rain blue
    for (size_t i = 0; i < rain.size(); i++) {
        drawLineDDA(rain[i].x, rain[i].y, rain[i].x, rain[i].y - rain[i].length);
    }

    // 5. Draw Ripples (using Midpoint Circle)
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
    if (!isPaused) {
        // Move clouds and handle lightning
        if (enableClouds) {
            for (size_t i = 0; i < clouds.size(); i++) {
                clouds[i].x += clouds[i].speed;
                if (clouds[i].x > WIDTH + 100) clouds[i].x = -100;
            }
            
            if (rand() % 150 == 0) {
                lightningTimer = 10; // Trigger flash for 10 frames
                
                // Generate a jagged lightning path
                lightningPath.clear();
                float lx = rand() % WIDTH;
                float ly = HEIGHT;
                lightningPath.push_back({lx, ly});
                while (ly > HEIGHT / 3) { // Strike down to middle/lower screen
                    lx += (rand() % 60) - 30; // Random x spread
                    ly -= (rand() % 50) + 20; // Step down
                    lightningPath.push_back({lx, ly});
                }
            }
            
            if (lightningTimer > 0) lightningTimer--;
        } else {
            lightningTimer = 0;
        }

        // Move rain
        for (size_t i = 0; i < rain.size(); i++) {
            rain[i].y -= rain[i].speed;
            if (rain[i].y < 0) {
                if (enableSplashes) spawnRipple(rain[i].x);
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
    }

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
        case 's': // Toggle Splashes
        case 'S':
            enableSplashes = !enableSplashes;
            if (!enableSplashes) ripples.clear(); // Instantly clear active splashes
            break;
        case 'c': // Toggle Clouds
        case 'C':
            enableClouds = !enableClouds;
            break;
        case 'm': // Toggle Moon/Sun Style
        case 'M':
            pixelatedMoon = !pixelatedMoon;
            break;
        case ' ': // Pause/Play Toggle
            isPaused = !isPaused;
            break;
        case 27: // ESC to exit
            exit(0);
            break;
    }
}

// --- Window Resizing Handler ---
void reshape(int w, int h) {
    if (h == 0) h = 1;
    float targetAspectRatio = (float)WIDTH / (float)HEIGHT;
    float windowAspectRatio = (float)w / (float)h;
    
    int viewportWidth = w;
    int viewportHeight = h;
    int viewportX = 0;
    int viewportY = 0;
    
    if (windowAspectRatio > targetAspectRatio) {
        // Window is wider than target: pillarbox
        viewportWidth = (int)(h * targetAspectRatio);
        viewportX = (w - viewportWidth) / 2;
    } else {
        // Window is taller than target: letterbox
        viewportHeight = (int)(w / targetAspectRatio);
        viewportY = (h - viewportHeight) / 2;
    }
    
    glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIDTH, 0, HEIGHT);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void init() {
    glClearColor(0.05f, 0.05f, 0.15f, 1.0f);
}

int main(int argc, char** argv) {
    srand(time(0));
    generateCity();
    initRain(200);
    initClouds(6); // Add 6 clouds

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Rainy Cityscape - DDA & Midpoint Algorithms");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape); // Register resize handler
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, update, 0);

    // Terminal Instructions
    std::cout << "--- Controls --- \n";
    std::cout << "'r' : Regenerate new city skyline\n";
    std::cout << "'+' : Increase rain intensity\n";
    std::cout << "'-' : Decrease rain intensity\n";
    std::cout << "'n' / 'd' : Toggle Night/Dusk\n";
    std::cout << "'s' : Toggle rain splashes\n";
    std::cout << "'c' : Toggle animated thunder clouds\n";
    std::cout << "'m' : Toggle pixelated/smooth moon and sun\n";
    std::cout << "SPACE : Pause/Play simulation\n";
    std::cout << "ESC : Exit\n";

    glutMainLoop();
    return 0;
}