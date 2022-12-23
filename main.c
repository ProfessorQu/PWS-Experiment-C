#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "raylib.h"

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
typedef enum {
    Empty, Water, Sand, Product
} Element;

#define screenWidth 800
#define screenHeight 800

#define cellSize 5

#define gridWidth (screenWidth / cellSize)
#define gridHeight (screenHeight / cellSize)

//----------------------------------------------------------------------------------
// Local Functions Declaration
//----------------------------------------------------------------------------------
static void Update();
static void Draw();

static void Inputs();

void UpdateGridUpwards(int direction);
void UpdateGridDownwards(int direction);

bool Fall(int x, int y, int direction);
bool FallIn(int x, int y, int direction, Element type);

void UpdateWater(int x, int y);
bool Spread(int x, int y, int spread);

void UpdateSand(int x, int y);
bool React(int x, int y, Element with, Element to);

void UpdateProduct(int x, int y);

void SetElement(int x, int y, int size, Element type);

//----------------------------------------------------------------------------------
// Local Variables Definition (local to this module)
//----------------------------------------------------------------------------------
const int gravity = 3;

const int waterSpreadRate = 5;
const int productSpreadRate = 10;

bool left = false;

Element grid[gridWidth][gridHeight];

bool InBounds(int x, int y)
{
    return 0 <= x && x < gridWidth && 0 <= y && y < gridHeight;
}

bool swap(Element *a, Element *b)
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
    Inputs();
        
    // Update Grid
    //---------------------------------------------------------
    if (left)
    {
        UpdateGridUpwards(1);
        UpdateGridDownwards(1);
    }
    else
    {
        UpdateGridUpwards(-1);
        UpdateGridDownwards(-1);
    }
    
    left = !left;
}

//----------------------------------------------------------------------------------
// Handle mouse inputs
//----------------------------------------------------------------------------------
void Inputs()
{
    // Get the mouse position
    int mouseX = GetMouseX() / cellSize;
    int mouseY = GetMouseY() / cellSize;

    // Spawning Cells
    //---------------------------------------------------------
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        SetElement(mouseX, mouseY, 10, Water);
    else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        SetElement(mouseX, mouseY, 5, Sand);
    else if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
        SetElement(mouseX, mouseY, 10, Empty);
    
    // Reset Grid
    //---------------------------------------------------------
    if (IsKeyPressed(KEY_R))
        for (int x = 0; x < gridWidth; x++)
            for (int y = 0; y < gridHeight; y++)
                grid[x][y] = Empty;
}

//----------------------------------------------------------------------------------
// Update grid from bottom to top
//----------------------------------------------------------------------------------
void UpdateGridUpwards(int direction)
{
    if (direction == 1)
        for (int y = gridHeight - 1; y >= 0 ; y--)
            for (int x = 0; x < gridWidth; x++)
            {
                if (grid[x][y] == Water)
                    UpdateWater(x, y);
                else if (grid[x][y] == Sand)
                    UpdateSand(x, y);
            }

    else
        for (int y = gridHeight - 1; y >= 0 ; y--)
            for (int x = gridWidth - 1; x >= 0; x--)
            {
                if (grid[x][y] == Water)
                    UpdateWater(x, y);
                else if (grid[x][y] == Sand)
                    UpdateSand(x, y);
            }
}

//----------------------------------------------------------------------------------
// Update grid from top to bottom
//----------------------------------------------------------------------------------
void UpdateGridDownwards(int direction)
{
    if (direction == 1)
        for (int y = 0; y < gridHeight; y++)
            for (int x = 0; x < gridWidth; x++)
            {
                if (grid[x][y] == Product)
                    UpdateProduct(x, y);
            }

    else
        for (int y = 0; y < gridHeight; y++)
            for (int x = gridWidth - 1; x >= 0; x--)
            {
                if (grid[x][y] == Product)
                    UpdateProduct(x, y);
            }
}

//----------------------------------------------------------------------------------
// Spawn cells
//----------------------------------------------------------------------------------
void SetElement(int x, int y, int size, Element type)
{

    // Set cells in a (2*size)*(2*size) square
    for (int cx = -size; cx <= size; cx++)
        for (int cy = -size; cy <= size; cy++)
            if (InBounds(x + cx, y + cy))
                grid[x + cx][y + cy] = type;
}

//----------------------------------------------------------------------------------
// Update Water
//----------------------------------------------------------------------------------
void UpdateWater(int x, int y)
{
    if (!Fall(x, y, -1))
        Spread(x, y, waterSpreadRate);
}

//----------------------------------------------------------------------------------
// Update Sand
//----------------------------------------------------------------------------------
void UpdateSand(int x, int y)
{
    if (!React(x, y, Water, Product))
        if (!Fall(x, y, -1))
            FallIn(x, y, -1, Water);
}

//----------------------------------------------------------------------------------
// Update Product
//----------------------------------------------------------------------------------
void UpdateProduct(int x, int y)
{
    if (!Fall(x, y, 1))
        if (!FallIn(x, y, 1, Water))
            if (!FallIn(x, y, 1, Sand))
                Spread(x, y, productSpreadRate);
}

bool Spread(int x, int y, int spread)
{
    for (int distance = spread; distance > 0; distance--)
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

//----------------------------------------------------------------------------------
// React
//----------------------------------------------------------------------------------
bool React(int x, int y, Element with, Element to)
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

//----------------------------------------------------------------------------------
// Fall
//----------------------------------------------------------------------------------
bool Fall(int x, int y, int direction)
{
    if (direction == -1)
        for (int distance = gravity; distance > 0; distance--)
        {
            if (InBounds(x, y + distance) && grid[x][y + distance] == Empty)
                return swap(&grid[x][y], &grid[x][y + distance]);
            else if (InBounds(x + distance, y + distance) && grid[x + distance][y + distance] == Empty)
                return swap(&grid[x][y], &grid[x + distance][y + distance]);
            else if (InBounds(x - distance, y + distance) && grid[x - distance][y + distance] == Empty)
                return swap(&grid[x][y], &grid[x - distance][y + distance]);
        }
    else
        for (int distance = gravity; distance > 0; distance--)
        {
            if (InBounds(x, y - distance) && grid[x][y - distance] == Empty)
                return swap(&grid[x][y], &grid[x][y - distance]);
            else if (InBounds(x + distance, y - distance) && grid[x + distance][y - distance] == Empty)
                return swap(&grid[x][y], &grid[x + distance][y - distance]);
            else if (InBounds(x - distance, y - distance) && grid[x - distance][y - distance] == Empty)
                return swap(&grid[x][y], &grid[x - distance][y - distance]);
        }

    
    return false;
}

//----------------------------------------------------------------------------------
// Fall In Element
//----------------------------------------------------------------------------------
bool FallIn(int x, int y, int direction, Element type)
{
    if (direction == -1)
        for (int distance = gravity; distance > 0; distance--)
        {
            if (InBounds(x, y + distance) && grid[x][y + distance] == type)
                return swap(&grid[x][y], &grid[x][y + distance]);
            else if (InBounds(x + distance, y + distance) && grid[x + distance][y + distance] == type)
                return swap(&grid[x][y], &grid[x + distance][y + distance]);
            else if (InBounds(x - distance, y + distance) && grid[x - distance][y + distance] == type)
                return swap(&grid[x][y], &grid[x - distance][y + distance]);
        }
    else
        for (int distance = gravity; distance > 0; distance--)
        {
            if (InBounds(x, y - distance) && grid[x][y - distance] == type)
                return swap(&grid[x][y], &grid[x][y - distance]);
            else if (InBounds(x + distance, y - distance) && grid[x + distance][y - distance] == type)
                return swap(&grid[x][y], &grid[x + distance][y - distance]);
            else if (InBounds(x - distance, y - distance) && grid[x - distance][y - distance] == type)
                return swap(&grid[x][y], &grid[x - distance][y - distance]);
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
                    DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, BEIGE);
                else if (grid[x][y] == Product)
                    DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, LIGHTGRAY);
            }

        DrawFPS(10, 10);

    EndDrawing();
}