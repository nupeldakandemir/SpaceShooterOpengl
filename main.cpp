#include <GL/freeglut_std.h>
#include <GL/freeglut.h>
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <string>
#include <ctime>
#include <sstream>

#define WINDOW_W 1200
#define WINDOW_H 750
#define FPS 60
#define SCORE_LIMIT 20

using namespace std;

static GLfloat screenBottom = -30.5;

class Bullet {
public:
    GLfloat x;
    GLfloat y;
    GLfloat speed;
    GLfloat size;

    Bullet(GLfloat _x, GLfloat _y, GLfloat _speed, GLfloat _size) : x(_x), y(_y), speed(_speed), size(_size) {}
};

std::vector<Bullet> bullets;

class Stone {
public:
    GLfloat x;
    GLfloat y;
    GLfloat speed;
    GLfloat size;

    Stone(GLfloat _x, GLfloat _y, GLfloat _speed, GLfloat _size) : x(_x), y(_y), speed(_speed), size(_size) {}
};

std::vector<Stone> stones;

GLfloat rocketPosition = 0.0;
GLfloat rocketWidth = 7.0;
GLfloat rocketHeight = 13.0;
GLfloat rocketSpeed = 0.8;
int score = 0;
int lives = 3; // Yıldızlara çarpma hakkı

bool gamePaused = false;
bool gameOver = true;
bool scoreLimitExceeded = false;

void togglePause() {
    gamePaused = !gamePaused;
}

void drawRocket() {
    const int numSegments = 100; // Daha düzgün bir elips için segment sayısını artırabilirsiniz
    const GLfloat radiusX = rocketWidth / 2.0;
    const GLfloat radiusY = rocketHeight / 2.0;

    glBegin(GL_TRIANGLE_FAN);
    glColor3f(0.7f, 0.2f, 0.2f); // Kırmızı renk
    glVertex2f(rocketPosition + radiusX, screenBottom); // Alt merkez noktası
    for (int i = 0; i <= numSegments; i++) {
        GLfloat theta = 2.0 * 3.1415926 * static_cast<GLfloat>(i) / static_cast<GLfloat>(numSegments);
        GLfloat x = radiusX * cos(theta);
        GLfloat y = radiusY * sin(theta);
        glVertex2f(x + rocketPosition + radiusX, y + screenBottom + radiusY);
    }
    glEnd();

    glBegin(GL_QUADS);
    glVertex2f(rocketPosition + rocketWidth * 0.25, screenBottom + rocketHeight); // Üst sol
    glVertex2f(rocketPosition + rocketWidth * 0.75, screenBottom + rocketHeight); // Üst sağ
    glVertex2f(rocketPosition + rocketWidth * 0.6, screenBottom + rocketHeight + rocketHeight * 0.4); // Alt sağ
    glVertex2f(rocketPosition + rocketWidth * 0.4, screenBottom + rocketHeight + rocketHeight * 0.4); // Alt sol
    glEnd();
}

void drawBullet(GLfloat x, GLfloat y, GLfloat size) {
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(0.5f, 0.5f, 1.0f);
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 1.0f, 1.0f); // Beyaz renk
    glVertex2f(0.0f, 0.0f);
    glVertex2f(size / 2, -size * 2);
    glVertex2f(-size / 2, -size * 2);
    glEnd();
    glPopMatrix();
}

void drawStone(GLfloat x, GLfloat y, GLfloat size) {
    const GLfloat angle = 3.14159f * 2.0f / 5.0f;

    glColor3f(0.0f, 0.0f, 1.0f); // Mavi renk
    glBegin(GL_POLYGON);
    for (int i = 0; i < 5; ++i) {
        GLfloat outerX = x + cos(angle * i) * size;
        GLfloat outerY = y + sin(angle * i) * size;
        glVertex2f(outerX, outerY);
    }
    glEnd();
}

void moveRocket(int direction) {
    rocketPosition += direction * rocketSpeed;
    if (rocketPosition < -50.0)
        rocketPosition = -50.0;
    else if (rocketPosition > 40.0)
        rocketPosition = 40.0;
}

void shootBullet() {
    bullets.push_back(Bullet(rocketPosition + rocketWidth / 2, screenBottom + rocketHeight, 1.0f, 2.0f));
}

void drawBullets() {
    for (size_t i = 0; i < bullets.size(); ++i) {
        drawBullet(bullets[i].x, bullets[i].y, bullets[i].size);
    }
}

void moveBullets() {
    for (size_t i = 0; i < bullets.size(); ++i) {
        bullets[i].y += bullets[i].speed;
        if (bullets[i].y > 50.0) {
            bullets.erase(bullets.begin() + i);
            --i;
        }
    }
}

void drawStone() {
    for (size_t i = 0; i < stones.size(); ++i) {
        drawStone(stones[i].x, stones[i].y, stones[i].size);
    }
}

void moveStone() {
    for (size_t i = 0; i < stones.size(); ++i) {
        stones[i].y -= stones[i].speed;
        if (stones[i].y < screenBottom) {
            stones.erase(stones.begin() + i);
            --i;
        }
    }
}

void generateStone() {
    if (rand() % 40 == 0) {
        for (int i = 0; i < 2; ++i) {
            GLfloat x = static_cast<GLfloat>(rand() % 101) - 50.0f;
            GLfloat y = 50.0f;
            GLfloat speed = static_cast<GLfloat>(rand() % 6) / 10.0f + 0.5f;
            GLfloat size = static_cast<GLfloat>(rand() % 3) + 1.0f;
            stones.push_back(Stone(x, y, speed, size));
        }
    }
}

void checkCollisions() {
    for (size_t i = 0; i < stones.size(); ++i) {
        GLfloat distX = rocketPosition - stones[i].x;
        GLfloat distY = screenBottom - stones[i].y;
        GLfloat distance = sqrt(distX * distX + distY * distY);
        if (distance < stones[i].size) {
            lives--; // Çarpma hakkını bir azalt
            if (lives == 0) {
                gameOver = true; // Eğer çarpma hakkınız sıfırsa oyunu kaybet
            }
            else {
                stones.erase(stones.begin() + i); // Yıldızı listeden kaldır
                --i; // İndeksi azalt
            }
            break;
        }
    }
}

void checkBulletStoneCollisions() {
    for (size_t i = 0; i < bullets.size(); ++i) {
        for (size_t j = 0; j < stones.size(); ++j) {
            GLfloat distX = bullets[i].x - stones[j].x;
            GLfloat distY = bullets[i].y - stones[j].y;
            GLfloat distance = sqrt(distX * distX + distY * distY);
            if (distance < stones[j].size) {
                bullets.erase(bullets.begin() + i); // Mermiyi listeden kaldır
                stones.erase(stones.begin() + j); // Yıldızı listeden kaldır
                score++; // Puanı artır
                break;
            }
        }
    }
}

void drawScore() {
    glPushMatrix();
    glLoadIdentity();
    GLfloat x = -49.0f;
    GLfloat y = 25.0f;
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);
    std::stringstream ss;
    ss << "Score: " << score;
    std::string scoreStr = ss.str();
    for (int i = 0; i < scoreStr.length(); i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, scoreStr[i]);
    }
    glPopMatrix();
}

void drawLives() {
    glPushMatrix();
    glLoadIdentity();
    GLfloat x = 40.0f;
    GLfloat y = 25.0f;
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);
    std::stringstream ss;
    ss << "Lives: " << lives;
    std::string livesStr = ss.str();
    for (int i = 0; i < livesStr.length(); i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, livesStr[i]);
    }
    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawRocket();
    drawBullets();
    drawStone();
    drawScore();
    drawLives();

    if (gameOver) {
        glPushMatrix();
        glLoadIdentity();

        GLfloat x = -10.0f;
        GLfloat y = 0.0f;
        glColor3f(1.0f, 1.0f, 1.0f);
        glRasterPos2f(x, y);

        std::string gameOverStr;
        if (score == 0)
            gameOverStr = "WELCOME TO NUPI'S UNIVERSE!*** Press Enter";
        else
            gameOverStr = "Game Over! Your Score: " + std::to_string(score);

        for (int i = 0; i < gameOverStr.length(); i++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, gameOverStr[i]);
        }

        if (score > 0) {
            glColor3f(1.0f, 1.0f, 1.0f);
            glRasterPos2f(x, y - 2.0f);
            std::string restartMsg = " You are the evil alien Press ENTER to fight across the Galaxy";
            for (int i = 0; i < restartMsg.length(); i++) {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, restartMsg[i]);
            }
        }

        glPopMatrix();
    }

    glutSwapBuffers();
}

void update(int value) {
    if (!gameOver) {
        generateStone();
        moveStone();
        moveBullets();
        checkCollisions();
        checkBulletStoneCollisions();
    }

    glutPostRedisplay();
    glutTimerFunc(1000 / FPS, update, 0);
}

void processNormalKeys(unsigned char key, int x, int y) {
    if (key == 27) { // Escape key
        exit(0);
    }
    else if (key == ' ') {
        if (!gamePaused && !gameOver) {
            shootBullet();
        }
    }
    else if (key == 13) { // Enter key
        if (gameOver) {
            gameOver = false;
            score = 0;
            lives = 3; // Oyunu yeniden başlatırken çarpma hakkınızı resetler
            stones.clear(); // Oyunu yeniden başlatırken yıldız listesini temizler
        }
    }
}

void processSpecialKeys(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_LEFT:
        moveRocket(-1);
        break;
    case GLUT_KEY_RIGHT:
        moveRocket(1);
        break;
    }
}

void reshape(GLsizei width, GLsizei height) {
    if (height == 0) height = 1;
    GLfloat aspect = (GLfloat)width / (GLfloat)height;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (width >= height) {
        gluOrtho2D(-50.0 * aspect, 50.0 * aspect, screenBottom, 50.0);
    }
    else {
        gluOrtho2D(-50.0, 50.0, screenBottom / aspect, 50.0 / aspect);
    }
}

void initGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

int main(int argc, char** argv) {
    srand(static_cast<unsigned int>(time(NULL)));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE);
    glutInitWindowSize(WINDOW_W, WINDOW_H);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("Space Shooter Game");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(processNormalKeys);
    glutSpecialFunc(processSpecialKeys);
    glutTimerFunc(0, update, 0);
    initGL();
    glutMainLoop();
    return 0;
}
