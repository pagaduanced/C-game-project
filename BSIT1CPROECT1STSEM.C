#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define MAX_SIZE  41    // supports up to 41x41
#define CELL_SIZE 2.0f  // world units per cell

enum { CELL_WALL = 1, CELL_EMPTY = 0 };

typedef enum { LEVEL_EASY = 0, LEVEL_MEDIUM = 1, LEVEL_HARD = 2 } Level;

// Maze storage (max size)
static int maze[MAX_SIZE][MAX_SIZE];
static unsigned char seen[MAX_SIZE][MAX_SIZE]; // 0 = unseen, 1 = seen

static int mazeW = 21;
static int mazeH = 21;
static int revealRadius = 4; // cells within this radius are revealed

// Player
static Vector3 playerPos;
static float playerYaw = 0.0f;

// Camera
static Camera3D camera3d;

// Game state
static Level currentLevel = LEVEL_EASY;
static double levelTimeRemaining = 300.0; // seconds
static const int levelSizes[3] = { 21, 31, 41 };
static const double levelTimes[3] = { 300.0, 180.0, 90.0 }; // easy, medium, hard

static int showMap = 1;
static int mouseLook = 1;

// forward declarations
void InitGame(void);
void SetLevel(Level L);
void GenerateMazeRecursive(int sx, int sy);
void Shuffle(int *arr, int n);
void RenderMaze(void);
void DrawCellCube(int cx, int cy);
int CellAt(int x, int y);
void SetCell(int x, int y, int v);
int IsInside(int x, int y);
int CheckCollisionWithWalls(Vector3 newPos);
void ResetPlayerToStart(void);

// Utility: clamp
static inline int clampi(int v, int a, int b) { return (v<a)?a: (v>b)?b:v; }

int main(void) {
    srand((unsigned)time(NULL));
    const int screenW = 1280;
    const int screenH = 720;
    InitWindow(screenW, screenH, "3D Maze - raylib (Reveal + Levels)");
    SetTargetFPS(60);

    // Setup camera (modern raylib)
    camera3d.position = (Vector3){ 1.5f, 1.8f, 1.5f }; // will be updated from player
    camera3d.target = (Vector3){ 2.5f, 1.8f, 1.5f };
    camera3d.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera3d.fovy = 60.0f;
    camera3d.projection = CAMERA_PERSPECTIVE; // correct modern field

    if (mouseLook) DisableCursor();

    SetLevel(currentLevel);

    char info[256];

    while (!WindowShouldClose()) {
        // Input: level selection and cycling
        if (IsKeyPressed(KEY_ONE)) { SetLevel(LEVEL_EASY); }
        if (IsKeyPressed(KEY_TWO)) { SetLevel(LEVEL_MEDIUM); }
        if (IsKeyPressed(KEY_THREE)) { SetLevel(LEVEL_HARD); }
        if (IsKeyPressed(KEY_TAB)) { SetLevel((Level)((currentLevel+1)%3)); }

        if (IsKeyPressed(KEY_G)) showMap = !showMap;
        if (IsKeyPressed(KEY_R)) { InitGame(); }
        if (IsKeyPressed(KEY_M)) { mouseLook = !mouseLook; if (mouseLook) DisableCursor(); else EnableCursor(); }

        // Player movement
        Vector3 forward = (Vector3){ sinf(playerYaw), 0, cosf(playerYaw) };
        Vector3 right = (Vector3){ -cosf(playerYaw), 0, -sinf(playerYaw) };
        Vector3 velocity = { 0, 0, 0 };
        float speed = 6.0f * GetFrameTime();
        if (IsKeyDown(KEY_W)) { velocity.x += forward.x * speed; velocity.z += forward.z * speed; }
        if (IsKeyDown(KEY_S)) { velocity.x -= forward.x * speed; velocity.z -= forward.z * speed; }
        if (IsKeyDown(KEY_A)) { velocity.x -= right.x * speed; velocity.z -= right.z * speed; }
        if (IsKeyDown(KEY_D)) { velocity.x += right.x * speed; velocity.z += right.z * speed; }
        if (IsKeyDown(KEY_SPACE)) { velocity.y += speed; }
        if (IsKeyDown(KEY_LEFT_SHIFT)) { velocity.y -= speed; }

        if (mouseLook) {
            Vector2 md = GetMouseDelta();
            playerYaw -= md.x * 0.0025f;
        } else {
            if (IsKeyDown(KEY_LEFT)) playerYaw -= 1.5f * GetFrameTime();
            if (IsKeyDown(KEY_RIGHT)) playerYaw += 1.5f * GetFrameTime();
        }

        // Attempt movement with collision
        Vector3 proposed = playerPos;
        proposed.x += velocity.x;
        proposed.y += velocity.y;
        proposed.z += velocity.z;
        if (!CheckCollisionWithWalls(proposed)) {
            playerPos = proposed;
        } else {
            Vector3 testX = playerPos; testX.x = proposed.x;
            Vector3 testZ = playerPos; testZ.z = proposed.z;
            if (!CheckCollisionWithWalls(testX)) playerPos.x = testX.x;
            else if (!CheckCollisionWithWalls(testZ)) playerPos.z = testZ.z;
        }

        // Update camera to follow player and use raylib's UpdateCamera for FPS behavior
        camera3d.position = (Vector3){ playerPos.x, playerPos.y + 0.5f, playerPos.z };
        camera3d.target = (Vector3){ playerPos.x + sinf(playerYaw), playerPos.y + 0.5f, playerPos.z + cosf(playerYaw) };
        camera3d.position = (Vector3){ playerPos.x, 0.5f, playerPos.z };
        camera3d.target   = (Vector3){ playerPos.x + sinf(playerYaw), 0.5f, playerPos.z + cosf(playerYaw) };
        // Reveal logic: mark cells seen if within revealRadius
        int pcx = (int)roundf(playerPos.x / CELL_SIZE);
        int pcy = (int)roundf(playerPos.z / CELL_SIZE);
        for (int y = pcy - revealRadius; y <= pcy + revealRadius; y++) {
            for (int x = pcx - revealRadius; x <= pcx + revealRadius; x++) {
                if (!IsInside(x,y)) continue;
                int dx = x - pcx; int dy = y - pcy;
                if ((dx*dx + dy*dy) <= (revealRadius*revealRadius)) {
                    seen[y][x] = 1; // once seen remains seen
                }
            }
        }

        // Timer update
        levelTimeRemaining -= GetFrameTime();
        if (levelTimeRemaining <= 0.0) {
            // Time ran out: restart whole game back to EASY
            SetLevel(LEVEL_EASY);
            InitGame();
        }

        // Check goal (reach far corner)
        int gx = mazeW - 2;
        int gy = mazeH - 2;
        float goalX = gx * CELL_SIZE;
        float goalZ = gy * CELL_SIZE;
        float dx = playerPos.x - goalX;
        float dz = playerPos.z - goalZ;
        if ((dx*dx + dz*dz) < (0.5f * CELL_SIZE)*(0.5f * CELL_SIZE)) {
            // advance to next level; if at hard, loop back to easy
            SetLevel((Level)((currentLevel+1)%3));
            InitGame();
        }

        // HUD string
        int minutes = (int)levelTimeRemaining / 60;
        int seconds = (int)levelTimeRemaining % 60;
        snprintf(info, sizeof(info), "Level: %s | Time: %02d:%02d | Pos: (%.1f,%.1f,%.1f)",
                 (currentLevel==LEVEL_EASY)?"EASY":(currentLevel==LEVEL_MEDIUM)?"MEDIUM":"HARD",
                 minutes, seconds, playerPos.x, playerPos.y, playerPos.z);

        // Rendering
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera3d);
            // Ground
            DrawPlane((Vector3){ (mazeW-1)*CELL_SIZE/2.0f, -0.01f, (mazeH-1)*CELL_SIZE/2.0f }, (Vector2){ mazeW*CELL_SIZE, mazeH*CELL_SIZE }, LIGHTGRAY);
            RenderMaze();
            DrawSphere((Vector3){ playerPos.x, 0.25f, playerPos.z }, 0.2f, RED);
            DrawCube((Vector3){ goalX, 0.5f, goalZ }, 0.6f, 1.0f, 0.6f, GREEN);
        EndMode3D();

        DrawText(info, 10, 10, 14, DARKGRAY);
        DrawText("1/2/3: Level | Tab: Cycle | R: Regenerate | G: Minimap | M: Toggle Mouse Look | Esc: Exit", 10, 30, 12, DARKGRAY);

        // Draw minimap if enabled - only draw seen cells
        if (showMap) {
            const int mapSize = 220;
            const int mapX = GetScreenWidth() - mapSize - 20;
            const int mapY = 20;
            DrawRectangle(mapX-2, mapY-2, mapSize+4, mapSize+4, BLACK);
            for (int y = 0; y < mazeH; y++) {
                for (int x = 0; x < mazeW; x++) {
                    if (!seen[y][x]) continue; // only draw seen cells
                    int cell = maze[y][x];
                    int bx = mapX + (x * mapSize) / mazeW;
                    int by = mapY + (y * mapSize) / mazeH;
                    int bw = (mapSize / mazeW) + 1;
                    int bh = (mapSize / mazeH) + 1;
                    Color c = (cell == CELL_WALL) ? DARKGRAY : RAYWHITE;
                    DrawRectangle(bx, by, bw, bh, c);
                }
            }
            int px = (int)(playerPos.x / (mazeW*CELL_SIZE) * mapSize);
            int pz = (int)(playerPos.z / (mazeH*CELL_SIZE) * mapSize);
            DrawCircle(mapX + px, mapY + pz, 4, RED);
            // Goal indicator only if seen
            if (seen[mazeH-2][mazeW-2]) DrawRectangle(mapX + ((mazeW-2) * mapSize) / mazeW, mapY + ((mazeH-2)*mapSize)/mazeH, 6, 6, GREEN);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

// Set the active difficulty level
void SetLevel(Level L) {
    currentLevel = L;
    mazeW = mazeH = levelSizes[L];
    levelTimeRemaining = levelTimes[L];
    // adjust reveal radius slightly with difficulty
    if (currentLevel == LEVEL_EASY) revealRadius = 5; else if (currentLevel == LEVEL_MEDIUM) revealRadius = 4; else revealRadius = 3;
    InitGame();
}

// Initialize game/maze and player
void InitGame(void) {
    // Initialize arrays
    for (int y = 0; y < MAX_SIZE; y++) for (int x = 0; x < MAX_SIZE; x++) { maze[y][x] = CELL_WALL; seen[y][x] = 0; }

    // Make sure dimensions are odd for generation
    if (mazeW % 2 == 0) mazeW--;
    if (mazeH % 2 == 0) mazeH--;
    // Generate
    GenerateMazeRecursive(1,1);
    SetCell(1,1, CELL_EMPTY);
    SetCell(mazeW-2, mazeH-2, CELL_EMPTY);
    ResetPlayerToStart();
}

void GenerateMazeRecursive(int sx, int sy) {
    SetCell(sx, sy, CELL_EMPTY);
    int dirs[4]; for (int i=0;i<4;i++) dirs[i]=i;
    Shuffle(dirs, 4);
    for (int i=0;i<4;i++) {
        int d = dirs[i];
        int nx = sx + ((d==1) - (d==3)) * 2;
        int ny = sy + ((d==2) - (d==0)) * 2;
        if (!IsInside(nx, ny)) continue;
        if (CellAt(nx, ny) == CELL_WALL) {
            int wx = sx + ((d==1) - (d==3));
            int wy = sy + ((d==2) - (d==0));
            SetCell(wx, wy, CELL_EMPTY);
            GenerateMazeRecursive(nx, ny);
        }
    }
}

void Shuffle(int *arr, int n) {
    for (int i = n-1; i>0; i--) {
        int j = rand() % (i+1);
        int t = arr[i]; arr[i] = arr[j]; arr[j] = t;
    }
}

void RenderMaze(void) {
    // Only draw cells that are seen (or within immediate proximity to camera to avoid popping)
    for (int y = 0; y < mazeH; y++) {
        for (int x = 0; x < mazeW; x++) {
            if (!seen[y][x]) continue; // unseen - don't render
            if (maze[y][x] == CELL_WALL) DrawCellCube(x,y);
            else {
                float worldX = x * CELL_SIZE;
                float worldZ = y * CELL_SIZE;
                DrawCube((Vector3){ worldX, -0.01f, worldZ }, CELL_SIZE*0.92f, 0.02f, CELL_SIZE*0.92f, Fade(WHITE, 0.95f));
            }
        }
    }
}

void DrawCellCube(int cx, int cy) {
    float worldX = cx * CELL_SIZE;
    float worldZ = cy * CELL_SIZE;
    DrawCube((Vector3){ worldX, 0.5f, worldZ }, CELL_SIZE*0.9f, 1.0f, CELL_SIZE*0.9f, GRAY);
    DrawCubeWires((Vector3){ worldX, 0.5f, worldZ }, CELL_SIZE*0.9f, 1.0f, CELL_SIZE*0.9f, DARKGRAY);
}

int CellAt(int x, int y) {
    if (!IsInside(x,y)) return CELL_WALL;
    return maze[y][x];
}

void SetCell(int x, int y, int v) {
    if (!IsInside(x,y)) return;
    maze[y][x] = v;
}

int IsInside(int x, int y) {
    return (x >= 0 && x < mazeW && y >= 0 && y < mazeH);
}

int CheckCollisionWithWalls(Vector3 newPos) {
    int cx = (int)roundf(newPos.x / CELL_SIZE);
    int cy = (int)roundf(newPos.z / CELL_SIZE);
    if (cx < 0 || cx >= mazeW || cy < 0 || cy >= mazeH) return 1;
    for (int oy = -1; oy <= 1; oy++) {
        for (int ox = -1; ox <= 1; ox++) {
            int tx = cx + ox;
            int ty = cy + oy;
            if (!IsInside(tx,ty)) continue;
            if (CellAt(tx,ty) == CELL_WALL) {
                float minX = tx*CELL_SIZE - CELL_SIZE*0.5f;
                float maxX = tx*CELL_SIZE + CELL_SIZE*0.5f;
                float minZ = ty*CELL_SIZE - CELL_SIZE*0.5f;
                float maxZ = ty*CELL_SIZE + CELL_SIZE*0.5f;
                float r = 0.25f;
                float px = newPos.x;
                float pz = newPos.z;
                float closestX = (px < minX) ? minX : (px > maxX) ? maxX : px;
                float closestZ = (pz < minZ) ? minZ : (pz > maxZ) ? maxZ : pz;
                float dx = px - closestX;
                float dz = pz - closestZ;
                if ((dx*dx + dz*dz) < r*r) return 1;
            }
        }
    }
    return 0;
}

void ResetPlayerToStart(void) {
    playerPos.x = 1 * CELL_SIZE;
    playerPos.y = 0.0f;
    playerPos.z = 1 * CELL_SIZE;
    playerYaw = 0.0f;
}