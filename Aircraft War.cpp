#include <graphics.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <tchar.h>

#define WIN_WIDTH   400
#define WIN_HEIGHT  600
#define MAX_ENEMY   50
#define MAX_BULLET  100
#define MAX_ENEMY_BULLET 200
#define UNICODE
#define _UNICODE

// Define a structure for game objects
struct GameObject {
    int x, y;
    bool active;
    int shootCooldown; // Add this line
};

// Game objects
GameObject myPlane = { 200, 590, true };
GameObject enemyPlanes[MAX_ENEMY];
GameObject bullets[MAX_BULLET];
GameObject enemyBullets[MAX_ENEMY_BULLET];
GameObject boss = { 200, 50, false };

// Game state
int playerHealth = 100;
int bossHealth = 500;
int score = 0;
bool gameOver = false;

// New global variables
bool missionComplete = false;
int bossDirection = 1;  // 1 for right, -1 for left
bool exitGame = false;  // New variable to control game exit

// Global variables for firing control
bool canShoot = true;
int shootCooldown = 0;

// Function prototypes
void InitGame();
void ProcessInput();
void UpdateGame();
void RenderGame();
void AddEnemy();
void AddBullet();
void AddEnemyBullet();
void AddBossBullet();
void CheckCollisions();
void ResetGame();

int main() {
    initgraph(WIN_WIDTH, WIN_HEIGHT);
    srand((unsigned)time(NULL));

    InitGame();

    BeginBatchDraw();

    while (!exitGame) {
        if (!gameOver) {
            ProcessInput();
            UpdateGame();
        }
        else {
            // Handle input for game over screen
            if (_kbhit()) {
                int ch = _getch();
                if (ch == 13) {  // Enter key
                    ResetGame();
                }
                else if (ch == 27) {  // ESC key
                    exitGame = true;
                }
            }
        }
        RenderGame();

        FlushBatchDraw();
        Sleep(16);  // Approx. 60 FPS
    }

    EndBatchDraw();

    closegraph();
    return 0;
}

void InitGame() {
    for (int i = 0; i < MAX_ENEMY; i++) {
        enemyPlanes[i].active = false;
        enemyPlanes[i].shootCooldown = 0; // Initialize cooldown
    }
    // ... (rest of the code remains unchanged)
    for (int i = 0; i < MAX_BULLET; i++) {
        bullets[i].active = false;
    }
    for (int i = 0; i < MAX_ENEMY_BULLET; i++) {
        enemyBullets[i].active = false;
    }
    boss.active = false;
    myPlane = { 200, 590, true };
    playerHealth = 100;
    bossHealth = 500;
    score = 0;
    gameOver = false;
    missionComplete = false;
}

void ResetGame() {
    InitGame();
}

void ProcessInput() {
    // Movement keys (arrow keys)
    if (GetAsyncKeyState(VK_UP)) {
        if (myPlane.y > 0) myPlane.y -= 6;
    }
    if (GetAsyncKeyState(VK_DOWN)) {
        if (myPlane.y < WIN_HEIGHT - 20) myPlane.y += 6;
    }
    if (GetAsyncKeyState(VK_LEFT)) {
        if (myPlane.x > 0) myPlane.x -= 6;
    }
    if (GetAsyncKeyState(VK_RIGHT)) {
        if (myPlane.x < WIN_WIDTH - 32) myPlane.x += 6;
    }

    // Fire bullet (space bar)
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
        if (canShoot && shootCooldown == 0) {
            AddBullet();
            shootCooldown = 15;
        }
    }

    // Decrease cooldown timer if space is held down
    if (shootCooldown > 0) {
        shootCooldown--;
    }
}

void AddBullet() {
    for (int i = 0; i < MAX_BULLET; i++) {
        if (!bullets[i].active) {
            bullets[i].x = myPlane.x + 16;
            bullets[i].y = myPlane.y - 10;
            bullets[i].active = true;
            break;
        }
    }
}



void UpdateGame() {
    static int frameCount = 0;
    frameCount++;

    // Spawn enemies
    if (frameCount % 60 == 0) {
        AddEnemy();
    }

    // Update enemy positions and firing
    for (int i = 0; i < MAX_ENEMY; i++) {
        if (enemyPlanes[i].active) {
            enemyPlanes[i].y += 2;

            // Manage enemy shooting
            if (enemyPlanes[i].shootCooldown > 0) {
                enemyPlanes[i].shootCooldown--; 
            }
            else {
                AddEnemyBullet(); // Call to fire bullet
                enemyPlanes[i].shootCooldown = 80; // Reset cooldown (e.g., fire every 30 frames)
            }

            if (enemyPlanes[i].y > WIN_HEIGHT) {
                enemyPlanes[i].active = false;
            }
        }
    }
     

    // Update bullet positions
    for (int i = 0; i < MAX_BULLET; i++) {
        if (bullets[i].active) {
            bullets[i].y -= 5;
            if (bullets[i].y < 0) {
                bullets[i].active = false;
            }
        }
    }

    // Enemy bullets
    if (frameCount % 30 == 0) {
        AddEnemyBullet();
    }

    for (int i = 0; i < MAX_ENEMY_BULLET; i++) {
        if (enemyBullets[i].active) {
            enemyBullets[i].y += 3;
            if (enemyBullets[i].y > WIN_HEIGHT) {
                enemyBullets[i].active = false;
            }
        }
    }

    // Boss logic
    if (score >= 500 && !boss.active && !missionComplete) {
        boss.active = true;
        bossHealth = 500;
    }

    if (boss.active) {
        boss.x += bossDirection * 2;
        if (boss.x <= 0 || boss.x >= WIN_WIDTH - 100) {
            bossDirection *= -1;
        }

        if (frameCount % 30 == 0) {
            AddBossBullet();
        }
    }

    CheckCollisions();

    // Slow down scoring
    if (frameCount % 10 == 0) {
        score++;
    }

    if (playerHealth <= 0) {
        gameOver = true;
    }
}

void AddBossBullet() {
    for (int j = 0; j < MAX_ENEMY_BULLET; j += 3) {
        if (!enemyBullets[j].active && !enemyBullets[j + 1].active && !enemyBullets[j + 2].active) {
            for (int k = 0; k < 3; k++) {
                enemyBullets[j + k].x = boss.x + 25 + k * 25;
                enemyBullets[j + k].y = boss.y + 60;
                enemyBullets[j + k].active = true;
            }
            break;
        }
    }
}

void RenderGame() {
    cleardevice();

    if (!gameOver) {
        // Draw player
        setfillcolor(BLUE);
        POINT playerPlane[3] = { {myPlane.x, myPlane.y}, {myPlane.x + 32, myPlane.y}, {myPlane.x + 16, myPlane.y - 20} };
        solidpolygon(playerPlane, 3);

        // Draw enemies
        setfillcolor(RED);
        for (int i = 0; i < MAX_ENEMY; i++) {
            if (enemyPlanes[i].active) {
                POINT enemyPlane[3] = { {enemyPlanes[i].x, enemyPlanes[i].y}, {enemyPlanes[i].x + 30, enemyPlanes[i].y}, {enemyPlanes[i].x + 15, enemyPlanes[i].y + 20} };
                solidpolygon(enemyPlane, 3);
            }
        }

        // Draw bullets
        setfillcolor(YELLOW);
        for (int i = 0; i < MAX_BULLET; i++) {
            if (bullets[i].active) {
                solidcircle(bullets[i].x, bullets[i].y, 3);
            }
        }

        // Draw enemy bullets
        setfillcolor(GREEN);
        for (int i = 0; i < MAX_ENEMY_BULLET; i++) {
            if (enemyBullets[i].active) {
                solidcircle(enemyBullets[i].x, enemyBullets[i].y, 2);
            }
        }

        // Draw boss
        if (boss.active) {
            setfillcolor(MAGENTA);
            POINT bossPlane[3] = { {boss.x, boss.y}, {boss.x + 100, boss.y}, {boss.x + 50, boss.y + 60} };
            solidpolygon(bossPlane, 3);

            // Boss health bar
            setfillcolor(GREEN);
            solidrectangle(boss.x, boss.y - 10, boss.x + bossHealth / 5, boss.y - 5);
        }

        // Draw player health
        setfillcolor(GREEN);
        solidrectangle(10, 10, 10 + playerHealth, 20);

        // Draw score
        TCHAR scoreText[20];
        swprintf_s(scoreText, 20, _T("Score: %d"), score);
        settextcolor(WHITE);
        outtextxy(10, 30, scoreText);

        // Display "Mission Complete" when boss is defeated
        if (missionComplete) {
            settextcolor(GREEN);
            RECT r = { 0, WIN_HEIGHT / 2 - 20, WIN_WIDTH, WIN_HEIGHT / 2 + 20 };
            drawtext(_T("MISSION COMPLETE"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
    }
    else {
        // Game over screen
        settextcolor(RED);
        RECT r = { 0, WIN_HEIGHT / 2 - 60, WIN_WIDTH, WIN_HEIGHT / 2 - 20 };
        drawtext(_T("GAME OVER"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        // Display final score
        TCHAR finalScoreText[50];
        swprintf_s(finalScoreText, 50, _T("Final Score: %d"), score);
        settextcolor(WHITE);
        r = { 0, WIN_HEIGHT / 2 - 20, WIN_WIDTH, WIN_HEIGHT / 2 + 20 };
        drawtext(finalScoreText, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        // Display options
        settextcolor(GREEN);
        r = { 0, WIN_HEIGHT / 2 + 20, WIN_WIDTH, WIN_HEIGHT / 2 + 60 };
        drawtext(_T("Press Enter to Restart"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        r = { 0, WIN_HEIGHT / 2 + 60, WIN_WIDTH, WIN_HEIGHT / 2 + 100 };
        drawtext(_T("Press ESC to Exit"), &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
}

void AddEnemy() {
    for (int i = 0; i < MAX_ENEMY; i++) {
        if (!enemyPlanes[i].active) {
            enemyPlanes[i].x = rand() % (WIN_WIDTH - 30);
            enemyPlanes[i].y = 0;
            enemyPlanes[i].active = true;
            break;
        }
    }
}


void AddEnemyBullet() {
    for (int i = 0; i < MAX_ENEMY; i++) {
        if (enemyPlanes[i].active) {
            // Fire bullet if the enemy is active and has cooldown completed
            for (int j = 0; j < MAX_ENEMY_BULLET; j++) {
                if (!enemyBullets[j].active) {
                    enemyBullets[j].x = enemyPlanes[i].x + 15; // Position of the bullet
                    enemyBullets[j].y = enemyPlanes[i].y + 20; // Fire from enemy position
                    enemyBullets[j].active = true;
                    break;
                }
            }
        }
    }
}

void CheckCollisions() {
    // Player bullets vs Enemies
    for (int i = 0; i < MAX_BULLET; i++) {
        if (bullets[i].active) {
            for (int j = 0; j < MAX_ENEMY; j++) {
                if (enemyPlanes[j].active) {
                    if (bullets[i].x >= enemyPlanes[j].x && bullets[i].x <= enemyPlanes[j].x + 30 &&
                        bullets[i].y >= enemyPlanes[j].y && bullets[i].y <= enemyPlanes[j].y + 20) {
                        bullets[i].active = false;
                        enemyPlanes[j].active = false;
                        score += 10;
                        break;
                    }
                }
            }

            // Player bullets vs Enemy bullets
            for (int k = 0; k < MAX_ENEMY_BULLET; k++) {
                if (enemyBullets[k].active) {
                    if (bullets[i].x >= enemyBullets[k].x - 2 && bullets[i].x <= enemyBullets[k].x + 2 &&
                        bullets[i].y >= enemyBullets[k].y - 2 && bullets[i].y <= enemyBullets[k].y + 2) {
                        bullets[i].active = false; // Destroy player's bullet
                        enemyBullets[k].active = false; // Destroy enemy bullet
                        break;
                    }
                }
            }
        }
    }

    // Enemy bullets vs Player
    for (int i = 0; i < MAX_ENEMY_BULLET; i++) {
        if (enemyBullets[i].active) {
            if (enemyBullets[i].x >= myPlane.x && enemyBullets[i].x <= myPlane.x + 32 &&
                enemyBullets[i].y >= myPlane.y && enemyBullets[i].y <= myPlane.y + 20) {
                enemyBullets[i].active = false;
                playerHealth -= 10;
                if (playerHealth < 0) playerHealth = 0;
            }
        }
    }

    // Player vs Enemies (collision with planes)
    for (int i = 0; i < MAX_ENEMY; i++) {
        if (enemyPlanes[i].active) {
            if (myPlane.x < enemyPlanes[i].x + 30 && myPlane.x + 32 > enemyPlanes[i].x &&
                myPlane.y < enemyPlanes[i].y + 20 && myPlane.y + 20 > enemyPlanes[i].y) {
                enemyPlanes[i].active = false;
                playerHealth -= 20;
                if (playerHealth < 0) playerHealth = 0;
            }
        }
    }

    // Player bullets vs Boss
    if (boss.active) {
        for (int i = 0; i < MAX_BULLET; i++) {
            if (bullets[i].active) {
                if (bullets[i].x >= boss.x && bullets[i].x <= boss.x + 100 &&
                    bullets[i].y >= boss.y && bullets[i].y <= boss.y + 60) {
                    // Destroy the bullet and reduce boss health
                    bullets[i].active = false;
                    bossHealth -= 10;

                    // Make sure the boss dies only when health reaches zero
                    if (bossHealth <= 0) {
                        boss.active = false;
                        bossHealth = 0;  // Ensure boss health doesn't go negative
                        missionComplete = true;  // Set mission complete flag
                        // Increase score once when boss is defeated
                        score += 100;
                    }
                }
            }
        }
    }
}

