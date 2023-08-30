#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "raylib.h"

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
typedef struct {
    int id;
    int updated;
    Color color;

    int density;
    int spreadRate;

    int withId;
    int into1Id;
    int into2Id;
} Element;

#define maxUpdates 10

#define numElements 4

Element elements[numElements] = {
    { 0, 0, GRAY,  2, 30  },        // Air
    { 1, 0, BLUE,  3, 5,  },        // Water
    { 2, 0, BEIGE, 4, 0, 1, 3, 3 }, // Sand
    { 3, 0, RED,   1, 0, }         // Product
};

#define screenWidth 800
#define screenHeight 800

#define cellSize 5

#define gridWidth (screenWidth / cellSize)
#define gridHeight (screenHeight / cellSize)

//----------------------------------------------------------------------------------
// Local Functions Declaration
//----------------------------------------------------------------------------------
void ResetGrid();

static void Update();
static void Draw();

static void Inputs();

void UpdateGrid();

bool Fall(int x, int y);

void UpdateElement(int x, int y);

bool Spread(int x, int y, int spread);
bool React(int x, int y, Element with, Element into1, Element into2);

void SetElement(int x, int y, int size, Element type);

//----------------------------------------------------------------------------------
// Local Variables Definition (local to this module)
//----------------------------------------------------------------------------------
const int gravity = 3;

bool left = false;

Element grid[gridWidth][gridHeight];

bool InBounds(int x, int y)
{
    return 0 <= x && x < gridWidth && 0 <= y && y < gridHeight;
}

bool swap(Element *a, Element *b)
{
    if (a->updated > maxUpdates || b->updated > maxUpdates)
        return true;

    Element temp = *a;
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

    ResetGrid();

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

void ResetGrid()
{
    for (int x = 0; x < gridWidth; x++)
        for (int y = 0; y < gridHeight; y++)
            grid[x][y] = elements[0];
}

//----------------------------------------------------------------------------------
// Update everything
//----------------------------------------------------------------------------------
void Update()
{
    for (int x = 0; x < gridWidth; x++)
        for (int y = 0; y < gridHeight; y++)
            grid[x][y].updated = 0;

    // Process inputs
    //---------------------------------------------------------
    Inputs();

        
    // Update Grid
    //---------------------------------------------------------
    if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE))
        UpdateGrid();
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
        SetElement(mouseX, mouseY, 3, elements[1]);
    else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        SetElement(mouseX, mouseY, 5, elements[2]);
    // else if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
    //     SetElement(mouseX, mouseY, 10, elements[0]);
    
    // Reset Grid
    //---------------------------------------------------------
    if (IsKeyPressed(KEY_R))
        ResetGrid();
}

//----------------------------------------------------------------------------------
// Update grid from bottom to top
//----------------------------------------------------------------------------------
void UpdateGrid()
{
    if (left)
        for (int y = gridHeight - 1; y >= 0 ; y--)
            for (int x = 0; x < gridWidth; x++)
                UpdateElement(x, y);
    else
        for (int y = gridHeight - 1; y >= 0 ; y--)
            for (int x = gridWidth - 1; x >= 0; x--)
                UpdateElement(x, y);

    left = !left;
}

//----------------------------------------------------------------------------------
// Spawn cells
//----------------------------------------------------------------------------------
void SetElement(int x, int y, int size, Element type)
{
    // Set cells in a (2*size)x(2*size) square
    for (int cx = -size; cx <= size; cx++)
        for (int cy = -size; cy <= size; cy++)
            if (InBounds(x + cx, y + cy))
                grid[x + cx][y + cy] = type;
}

void UpdateElement(int x, int y) {
    if (grid[x][y].withId != 0)
    {
        Element with = elements[grid[x][y].withId];
        Element into1 = elements[grid[x][y].into1Id];
        Element into2 = elements[grid[x][y].into2Id];

        if (!React(x, y, with, into1, into2)) {
            if (!Fall(x, y))
            {
                if (Spread(x, y, grid[x][y].spreadRate))
                    grid[x][y].updated += 1;
            }
            else
                grid[x][y].updated += 1;
        }
        else
            grid[x][y].updated += 1;
    }
    else if (!Fall(x, y))
    {
        if (Spread(x, y, grid[x][y].spreadRate))
            grid[x][y].updated += 1;
    }
    else
        grid[x][y].updated += 1;
}

bool Spread(int x, int y, int spread)
{
    // int randomDirection = rand() % 2;
    // if (randomDirection == 0)
    //     randomDirection = -1;

    for (int distance = spread; distance > 0; distance--)
    {
        int offset = distance;

        if (InBounds(x + offset, y) && grid[x + offset][y].density < grid[x][y].density)
            return swap(&grid[x][y], &grid[x + offset][y]);
        else if (InBounds(x - offset, y) && grid[x - offset][y].density < grid[x][y].density)
            return swap(&grid[x][y], &grid[x - offset][y]);
    }
    
    return false;
}

//----------------------------------------------------------------------------------
// React
//----------------------------------------------------------------------------------
bool React(int x, int y, Element with, Element into1, Element into2)
{
    if (InBounds(x, y + 1) && grid[x][y + 1].id == with.id)
    {
        if (grid[x][y].updated > maxUpdates || grid[x][y + 1].updated > maxUpdates)
            return true;

        grid[x][y] = into1;
        grid[x][y + 1] = into2;

        return true;
    }
    else if (InBounds(x, y - 1) && grid[x][y - 1].id == with.id)
    {
        if (grid[x][y].updated > maxUpdates || grid[x][y - 1].updated > maxUpdates)
            return true;
        
        grid[x][y] = into1;
        grid[x][y - 1] = into2;

        return true;
    }

    return false;
}

//----------------------------------------------------------------------------------
// Fall
//----------------------------------------------------------------------------------
bool Fall(int x, int y)
{
    for (int distance = gravity; distance > 0; distance--)
    {
        if (InBounds(x, y + distance) && grid[x][y + distance].density < grid[x][y].density)
            return swap(&grid[x][y], &grid[x][y + distance]);
        else if (InBounds(x + distance, y + distance) && grid[x + distance][y + distance].density < grid[x][y].density)
            return swap(&grid[x][y], &grid[x + distance][y + distance]);
        else if (InBounds(x - distance, y + distance) && grid[x - distance][y + distance].density < grid[x][y].density)
            return swap(&grid[x][y], &grid[x - distance][y + distance]);
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
                DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, grid[x][y].color);

        DrawFPS(10, 10);
    EndDrawing();
}