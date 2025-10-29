#include "raylib.h"

typedef enum GameState
{
    STATE_MENU,
    STATE_GAME
} GameState;
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1600;
    const int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60); // Set our game to run at 60 frames-per-second

    //--------------------------------------------------------------------------------------
    GameState state = STATE_MENU;
    int menuIndex = 0; // 0 = "Start", 1 = "Quit"

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        if (state == STATE_MENU)
        {
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
                menuIndex = 1;
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
                menuIndex = 0;
            // --- Mouse setup: recompute the same centered positions as in DRAW ---
            int rw = GetRenderWidth();
            int rh = GetRenderHeight();
            int fontSize = 40;

            int startWidth = MeasureText("START GAME", fontSize);
            int StartX = (rw - startWidth) / 2;
            int StartY = rh / 2;

            int quitWidth = MeasureText("QUIT GAME", fontSize);
            int quitX = (rw - quitWidth) / 2;
            int quitY = StartY + 60;

            const int padX = 24, padY = 10; 
            Rectangle btnStart = (Rectangle){StartX - padX, StartY -padY, startWidth + 2 * padX, fontSize + 2 * padY};
            Rectangle btnQuit = (Rectangle){quitX - padX, quitY -padY, quitWidth + 2 * padX, fontSize + 2 * padY};
            
            Vector2 m = GetMousePosition();
            bool hoverStart = CheckCollisionPointRec(m, btnStart);
            bool hoverQuit = CheckCollisionPointRec(m, btnQuit);

            if (hoverStart) menuIndex = 0;
            if (hoverQuit) menuIndex = 1;

            if (menuIndex == 0)
            {
                if (IsKeyPressed(KEY_ENTER))
                {
                    state = STATE_GAME;
                }
            }
            else if (menuIndex == 1)
            {
                if (IsKeyPressed(KEY_ENTER))
                {
                    break; // this exits the program
                }
            }

            if (hoverStart && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) state = STATE_GAME;
            if (hoverQuit && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) break;
            
        }
        else if (state == STATE_GAME)
        {
            if (IsKeyPressed(KEY_ESCAPE))
            {
                state = STATE_MENU;
            }
        }

        //-----------------------DRAWING AREA OF FUNCTIONS(BACKBONE)---------------------//
        BeginDrawing();

        ClearBackground(RAYWHITE);

        if (state == STATE_MENU)
        {
            int rw = GetRenderWidth();
            int rh = GetRenderHeight();

            int titleSize = 100;
            int titleWidth = MeasureText("OUT OF TOUCH", titleSize);
            int titleX = (rw - titleWidth) / 2;
            DrawText("OUT OF TOUCH", titleX, rh / 4, titleSize, BLACK);

            int fontSize = 40;
            const int padX = 24, padY = 10;

            // START GAME CONFIGURATION
            int startWidth = MeasureText("START GAME", fontSize);
            int StartX = (rw - startWidth) / 2;
            int StartY = rh / 2;
            Rectangle btnStart = (Rectangle){StartX - padX, StartY -padY, startWidth + 2 * padX, fontSize + 2 * padY};

            // QUIT GAME CONFIGURATION
            int quitWidth = MeasureText("QUIT GAME", fontSize);
            int quitX = (rw - quitWidth) / 2;
            int quitY = StartY + 60;
            Rectangle btnQuit = (Rectangle){quitX - padX, quitY -padY, quitWidth + 2 * padX, fontSize + 2 * padY};

            //MOUSE CONFIG
            Vector2 m = GetMousePosition();
            if (CheckCollisionPointRec(m, btnStart)) DrawRectangleRec(btnStart, (Color){220,220,220,255});
            if (CheckCollisionPointRec(m, btnQuit))  DrawRectangleRec(btnQuit,  (Color){220,220,220,255});
            Color startcolor = (menuIndex == 0) ? RED : DARKGRAY;
            Color quitcolor = (menuIndex == 1) ? RED : DARKGRAY;
            
            DrawText("START GAME", StartX, StartY, fontSize, startcolor);
            DrawText("QUIT GAME", quitX, quitY, fontSize, quitcolor);

            int insfontSize = 20;
            int instructionsWidth = MeasureText("USE ARROWS UP AND DOWN TO CHOOSE AND PRESS ENTER TO SELECT", insfontSize);
            int insX = (rw - instructionsWidth) / 2;
            int insY = quitY + 60;
            DrawText("USE ARROWS UP AND DOWN TO CHOOSE AND PRESS ENTER TO SELECT", insX, insY, insfontSize, GRAY);
        }
        else if (state == STATE_GAME)
        {
            // --- DRAW GAME SCREEN ---//
            DrawText("GAME RUNNING!", 680, 200, 40, DARKBLUE);
            DrawText("Press ESC to return to menu", 620, 260, 20, GRAY);
        }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}