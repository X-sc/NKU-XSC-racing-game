#include "raylib.h"
#include "game_def.h"

void GameInit();
void GameUpdate();
void GameDraw();

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "NKU Racing Game");
    SetTargetFPS(60);

    GameInit();

    while (!WindowShouldClose())
    {
        GameUpdate();
        GameDraw();
    }
 
    CloseWindow();
    return 0;
}