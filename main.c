#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "raylib.h"

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
typedef struct {
    int id;
    Color color;

    int density;
    int spreadRate;

    int withId;
    int into1Id;
    int into2Id;
} Element;

const int numElements = 4;

Element elements[] = {
    { 0, RAYWHITE,  3, 30 },            // Air
    { 1, BLUE,      5, 5, 1, 3, 3  },            // Water
    { 2, BEIGE,     7, 0  },    // Sand
    { 3, RED,       1, 30, 1, 2, 2 }             // Product
};

#define screenWidth 800
#define screenHeight 800

#define cellSize 5

#define gridWidth (screenWidth / cellSize)
#define gridHeight (screenHeight / cellSize)

//----------------------------------------------------------------------------------
// Local Functions Declaration
//----------------------------------------------------------------------------------
void InitGrid();

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

    SetTargetFPS(60);
    //----------------------------------------------------------

    InitGrid();

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

void InitGrid()
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
    Inputs();
        
    // Update Grid
    //---------------------------------------------------------
    UpdateGrid();
    
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
        SetElement(mouseX, mouseY, 10, elements[1]);
    else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        SetElement(mouseX, mouseY, 5, elements[2]);
    else if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
        SetElement(mouseX, mouseY, 10, elements[0]);
    
    // Reset Grid
    //---------------------------------------------------------
    if (IsKeyPressed(KEY_R))
        for (int x = 0; x < gridWidth; x++)
            for (int y = 0; y < gridHeight; y++)
                grid[x][y] = elements[0];
}

//----------------------------------------------------------------------------------
// Update grid from bottom to top
//----------------------------------------------------------------------------------
void UpdateGrid()
{
    if (left)
        for (int y = gridHeight - 1; y >= 0 ; y--)
            for (int x = 0; x < gridWidth; x++)
            {
                UpdateElement(x, y);
            }
    else
        for (int y = gridHeight - 1; y >= 0 ; y--)
            for (int x = gridWidth - 1; x >= 0; x--)
            {
                UpdateElement(x, y);
            }
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

        if (!React(x, y, with, into1, into2))
            if (!Fall(x, y))
                Spread(x, y, grid[x][y].spreadRate);
    }
    else if (!Fall(x, y))
        Spread(x, y, grid[x][y].spreadRate);
}

bool Spread(int x, int y, int spread)
{
    int randomDirection = rand() % 2;
    if (randomDirection == 0)
        randomDirection = -1;

    for (int distance = spread; distance > 0; distance--)
    {
        int offset = distance * randomDirection;

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
        grid[x][y] = into1;
        grid[x][y + 1] = into2;

        return true;
    }
    else if (InBounds(x, y - 1) && grid[x][y - 1].id == with.id)
    {
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