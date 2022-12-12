/*******************************************************************************************
*
*   raylib [core] example - Basic 3d example
*
*   Welcome to raylib!
*
*   To compile example, just press F5.
*   Note that compiled executable is placed in the same folder as .c file
*
*   You can find all basic examples on C:\raylib\raylib\examples folder or
*   raylib official webpage: www.raylib.com
*
*   Enjoy using raylib. :)
*
*   This example has been created using raylib 1.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2013-2022 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include <stdlib.h>
#include <time.h>
#include "raylib.h"

//----------------------------------------------------------------------------------
// Local Variables Definition (local to this module)
//----------------------------------------------------------------------------------
#define screenWidth 800
#define screenHeight 800

#define cellSize 2

#define gridWidth (screenWidth / cellSize)
#define gridHeight (screenHeight / cellSize)

const int gravity = 3;

const int waterSpreadRate = 10;

enum CellType { Empty, Water, Sand, Product };

enum CellType grid[gridWidth][gridHeight];

bool InBounds(int x, int y)
{
    return 0 <= x && x < gridWidth && 0 <= y && y < gridHeight;
}

bool swap(enum CellType *a, enum CellType *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;

    return true;
}

//----------------------------------------------------------------------------------
// Local Functions Declaration
//----------------------------------------------------------------------------------
static void Update(void);
static void Draw(void);

bool Fall(int x, int y);
bool FallIn(int x, int y, enum CellType type);

void UpdateWater(int x, int y);
bool WaterSpread(int x, int y);

void UpdateSand(int x, int y);
bool SandReact(int x, int y);

void UpdateProduct(int x, int y);

void SpawnCells(int size, enum CellType type);

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main() 
{
    // Initialization
    //---------------------------------------------------------
    srand(time(0));
    InitWindow(screenWidth, screenHeight, "Chemistry Simulation");

    SetTargetFPS(60);
    //----------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        Update();
        Draw();
    }

    // De-Initialization
    //---------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //----------------------------------------------------------

    return 0;
}

void Update()
{
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        SpawnCells(10, Water);
    else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        SpawnCells(5, Sand);
    
    for (int y = gridHeight - 1; y >= 0 ; y--)
        for (int x = 0; x < gridWidth; x++)
        {
            if (grid[x][y] == Water)
                UpdateWater(x, y);
            else if (grid[x][y] == Sand)
                UpdateSand(x, y);
            else if (grid[x][y] == Product)
                UpdateProduct(x, y);
        }
}

void SpawnCells(int size, enum CellType type)
{
    int mouseX = GetMouseX() / cellSize;
    int mouseY = GetMouseY() / cellSize;

    for (int cx = -size; cx <= size; cx++)
        for (int cy = -size; cy <= size; cy++)
            if (InBounds(mouseX + cx, mouseY + cy))
                grid[mouseX + cx][mouseY + cy] = type;
}

void UpdateWater(int x, int y)
{
    if (!Fall(x, y))
        WaterSpread(x, y);
}

bool WaterSpread(int x, int y)
{
    for (int distance = waterSpreadRate; distance > 0; distance--)
    {
        int randomDirection = rand() % 2 - 1;
        int offset = distance * randomDirection;

        if (InBounds(x + offset, y) && grid[x + offset][y] == Empty)
            return swap(&grid[x][y], &grid[x + offset][y]);
        else if (InBounds(x - offset, y) && grid[x - offset][y] == Empty)
            return swap(&grid[x][y], &grid[x - offset][y]);
    }
    
    return false;
}

void UpdateSand(int x, int y)
{
    if (!SandReact(x, y))
        if (!Fall(x, y))
            FallIn(x, y, Water);
}

bool SandReact(int x, int y)
{
    if (InBounds(x, y + 1) && grid[x][y + 1] == Water)
    {
        grid[x][y] = Product;
        grid[x][y + 1] = Empty;
        return swap(&grid[x][y], &grid[x][y + 1]);
    }
    else if (InBounds(x, y - 1) && grid[x][y - 1] == Water)
    {
        grid[x][y] = Product;
        grid[x][y - 1] = Empty;
        return swap(&grid[x][y], &grid[x][y - 1]);
    }

    return false;
}

void UpdateProduct(int x, int y)
{
    if (!Fall(x, y))
        if (!FallIn(x, y, Water))
            FallIn(x, y, Sand);
}

bool Fall(int x, int y)
{
    for (int distance = gravity; distance > 0; distance--)
    {
        if (InBounds(x, y + distance) && grid[x][y + distance] == Empty)
            return swap(&grid[x][y], &grid[x][y + distance]);
        else if (InBounds(x + 1, y + distance) && grid[x + 1][y + distance] == Empty)
            return swap(&grid[x][y], &grid[x + 1][y + distance]);
        else if (InBounds(x - 1, y + distance) && grid[x - 1][y + distance] == Empty)
            return swap(&grid[x][y], &grid[x - 1][y + distance]);
    }
    
    return false;
}

bool FallIn(int x, int y, enum CellType type)
{
    for (int distance = gravity; distance > 0; distance--)
    {
        if (InBounds(x, y + 1) && grid[x][y + distance] == type)
            return swap(&grid[x][y], &grid[x][y + distance]);
        else if (InBounds(x + 1, y + 1) && grid[x + 1][y + distance] == type)
            return swap(&grid[x][y], &grid[x + 1][y + distance]);
        else if (InBounds(x - 1, y + 1) && grid[x - 1][y + distance] == type)
            return swap(&grid[x][y], &grid[x - 1][y + distance]);
    }
    
    return false;
}


void Draw()
{
    // Draw
    //-----------------------------------------------------
    BeginDrawing();
        ClearBackground(RAYWHITE);
        
        for (int x = 0; x < gridWidth; x++)
            for (int y = 0; y < gridHeight; y++)
            {
                if (grid[x][y] == Water)
                    DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, BLUE);
                else if (grid[x][y] == Sand)
                    DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, BROWN);
                else if (grid[x][y] == Product)
                    DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, RED);
            }

        DrawFPS(10, 10);

    EndDrawing();
}