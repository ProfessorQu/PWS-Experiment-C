#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "raylib.h"

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
typedef enum {
    Empty, Water, Sand, Product
} CellType;

#define screenWidth 800
#define screenHeight 800

#define cellSize 5

#define gridWidth (screenWidth / cellSize)
#define gridHeight (screenHeight / cellSize)

//----------------------------------------------------------------------------------
// Local Functions Declaration
//----------------------------------------------------------------------------------
static void Update(void);
static void Draw(void);

void UpdateGridLeft();
void UpdateGridRight();

bool Fall(int x, int y);
bool FallIn(int x, int y, CellType type);

void UpdateEmpty(int x, int y);

void UpdateWater(int x, int y);
bool WaterSpread(int x, int y);

void UpdateSand(int x, int y);
bool React(int x, int y, CellType with, CellType to);

void UpdateProduct(int x, int y);

void SpawnCells(int size, CellType type);

//----------------------------------------------------------------------------------
// Local Variables Definition (local to this module)
//----------------------------------------------------------------------------------
const int gravity = 3;

const int waterSpreadRate = 5;

bool left = false;

CellType grid[gridWidth][gridHeight];

bool InBounds(int x, int y)
{
    return 0 <= x && x < gridWidth && 0 <= y && y < gridHeight;
}

bool swap(CellType *a, CellType *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;

    return true;
}

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

//----------------------------------------------------------------------------------
// Update everything
//----------------------------------------------------------------------------------
void Update()
{
    // Spawning Cells
    //---------------------------------------------------------
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        SpawnCells(10, Water);
    else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        SpawnCells(5, Sand);
    else if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
        SpawnCells(10, Empty);
    
    // Reset Grid
    //---------------------------------------------------------
    if (IsKeyPressed(KEY_R))
        for (int x = 0; x < gridWidth; x++)
            for (int y = 0; y < gridHeight; y++)
                grid[x][y] = Empty;
        
    // Update Grid
    //---------------------------------------------------------
    if (left)
        UpdateGridLeft();
    else
        UpdateGridRight();
    
    left = !left;
}

//----------------------------------------------------------------------------------
// Update grid left to right
//----------------------------------------------------------------------------------
void UpdateGridLeft()
{
    for (int y = gridHeight - 1; y >= 0 ; y--)
    {
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
}

//----------------------------------------------------------------------------------
// Update grid right to left
//----------------------------------------------------------------------------------
void UpdateGridRight()
{
    for (int y = gridHeight - 1; y >= 0 ; y--)
    {
        for (int x = gridWidth - 1; x >= 0; x--)
        {
            if (grid[x][y] == Water)
                UpdateWater(x, y);
            else if (grid[x][y] == Sand)
                UpdateSand(x, y);
            else if (grid[x][y] == Product)
                UpdateProduct(x, y);
        }
    }
}

void SpawnCells(int size, CellType type)
{
    // Get the mouse position
    int mouseX = GetMouseX() / cellSize;
    int mouseY = GetMouseY() / cellSize;

    // Set cells in a (2*size)*(2*size) square
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
        int randomDirection = rand() % 3 - 1;
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
    if (!React(x, y, Water, Product))
        if (!Fall(x, y))
            FallIn(x, y, Water);
}

void UpdateProduct(int x, int y)
{
    if (!Fall(x, y))
        if (!FallIn(x, y, Water))
            FallIn(x, y, Sand);
}

bool React(int x, int y, CellType with, CellType to)
{
    if (InBounds(x, y + 1) && grid[x][y + 1] == with)
    {
        grid[x][y] = to;
        grid[x][y + 1] = Empty;
        return swap(&grid[x][y], &grid[x][y + 1]);
    }
    else if (InBounds(x, y - 1) && grid[x][y - 1] == with)
    {
        grid[x][y] = to;
        grid[x][y - 1] = Empty;
        return swap(&grid[x][y], &grid[x][y - 1]);
    }

    return false;
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

bool FallIn(int x, int y, CellType type)
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

//----------------------------------------------------------------------------------
// Draw everything
//----------------------------------------------------------------------------------
void Draw()
{
    // Draw
    //-----------------------------------------------------
    BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Draw grid
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