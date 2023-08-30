#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "raylib.h"

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
typedef struct {
    int id;
    bool updated;
    Color color;

    int density;
    int spreadRate;

    int numReactions;
    int reactions[3][5];
} Element;

#define numElements 6

Element elements[numElements] = {
    { 0, false, WHITE,   0, 0,                 }, // Air
    { 1, false, BLUE,    1, 10,                }, // Water
    { 2, false, BEIGE,   4, 0,  1, {{1, 3, 3}} }, // Sand
    { 3, false, RED,    -5, 30,                }, // Product
    { 4, false, ORANGE,  6, 1                  },
    { 5, false, PURPLE, -3, 20, }
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

void UpdateGridUpward();
void UpdateGridDownward();

bool Fall(int x, int y);
bool FallDown(int x, int y);
bool FallUp(int x, int y);

void UpdateElement(int x, int y, bool up);

bool Spread(int x, int y, int spread);
bool SpreadDown(int x, int y, int randomDirection, int spread);
bool SpreadUp(int x, int y, int randomDirection, int spread);

bool React(int x, int y, Element with, Element into1, Element into2);

void SetElement(int x, int y, int size, Element type);

//----------------------------------------------------------------------------------
// Local Variables Definition (local to this module)
//----------------------------------------------------------------------------------
const int gravity = 3;
bool left = false;

int selectedId = 1;
int size = 1;

Element grid[gridWidth][gridHeight];

bool InBounds(int x, int y)
{
    return 0 <= x && x < gridWidth && 0 <= y && y < gridHeight;
}

bool swap(Element *a, Element *b)
{
    if (a->updated || b->updated)
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
    // Process inputs
    //---------------------------------------------------------
    Inputs();

    // Update Grid
    //---------------------------------------------------------
    UpdateGridUpward();
    UpdateGridDownward();

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

    for (int i = 0; i < numElements; i++)
        if (IsKeyPressed(KEY_ONE + i))
            selectedId = i + 1;

    size += GetMouseWheelMove();
    if (size < 0)
        size = 0;
    else if (size > 20)
        size = 20;

    // Spawning Cells
    //---------------------------------------------------------
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        SetElement(mouseX, mouseY, size, elements[selectedId]);
    else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        SetElement(mouseX, mouseY, size, elements[0]);
    
    // Reset Grid
    //---------------------------------------------------------
    else if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE))
        ResetGrid();
}

//----------------------------------------------------------------------------------
// Update grid from bottom to top
//----------------------------------------------------------------------------------
void UpdateGridUpward()
{
    if (left)
        for (int y = gridHeight - 1; y >= 0 ; y--)
            for (int x = 0; x < gridWidth; x++)
                UpdateElement(x, y, true);
    else
        for (int y = gridHeight - 1; y >= 0 ; y--)
            for (int x = gridWidth - 1; x >= 0; x--)
                UpdateElement(x, y, true);
}

void UpdateGridDownward()
{
    if (left)
        for (int y = 0; y < gridHeight; y++)
            for (int x = 0; x < gridWidth; x++)
                UpdateElement(x, y, false);
    else
        for (int y = 0; y < gridHeight; y++)
            for (int x = gridWidth - 1; x >= 0; x--)
                UpdateElement(x, y, false);
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

void UpdateElement(int x, int y, bool up)
{
    if (grid[x][y].updated)
        return;
    else if (up && grid[x][y].density < 0)
        return;
    else if (!up && grid[x][y].density > 0)
        return;
    else if (grid[x][y].id == 0)
        return;

    if (grid[x][y].numReactions > 0)
    {
        bool reacted = false;
        for (int i = 0; i < grid[x][y].numReactions; i++)
        {
            int* reaction = grid[x][y].reactions[i];
            Element with = elements[reaction[0]];
            Element into1 = elements[reaction[1]];
            Element into2 = elements[reaction[2]];

            if (React(x, y, with, into1, into2))
            {
                reacted = true;
                break;
            }
        }

        if (!reacted)
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

    if (grid[x][y].density > 0)
        return SpreadDown(x, y, randomDirection, spread);
    
    return SpreadUp(x, y, randomDirection, spread);
}

bool SpreadDown(int x, int y, int randomDirection, int spread)
{
    for (int distance = spread; distance > 0; distance--)
    {
        int offset = distance * randomDirection;

        if (InBounds(x + offset, y) && grid[x][y].density > 0 && grid[x + offset][y].density < grid[x][y].density)
            return swap(&grid[x][y], &grid[x + offset][y]);
        else if (InBounds(x - offset, y) && grid[x][y].density > 0 && grid[x - offset][y].density < grid[x][y].density)
            return swap(&grid[x][y], &grid[x - offset][y]);
    }

    return false;
}

bool SpreadUp(int x, int y, int randomDirection, int spread)
{
    for (int distance = spread; distance > 0; distance--)
    {
        int offset = distance * randomDirection;

        if (InBounds(x + offset, y) && grid[x][y].density < 0 && grid[x + offset][y].density > grid[x][y].density)
            return swap(&grid[x][y], &grid[x + offset][y]);
        else if (InBounds(x - offset, y) && grid[x][y].density < 0 && grid[x - offset][y].density > grid[x][y].density)
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
        if (grid[x][y].updated || grid[x][y + 1].updated)
            return false;
        grid[x][y] = into1;
        grid[x][y + 1] = into2;

        return true;
    }
    else if (InBounds(x, y - 1) && grid[x][y - 1].id == with.id)
    {
        if (grid[x][y].updated || grid[x][y - 1].updated)
            return false;
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
    if (grid[x][y].density < 0) {
        return FallUp(x, y);
    }

    return FallDown(x, y);
}

bool FallDown(int x, int y)
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

bool FallUp(int x, int y)
{
    for (int distance = gravity; distance > 0; distance--)
    {
        if (InBounds(x, y - distance) && grid[x][y - distance].density > grid[x][y].density)
            return swap(&grid[x][y], &grid[x][y - distance]);
        else if (InBounds(x + distance, y - distance) && grid[x + distance][y - distance].density > grid[x][y].density)
            return swap(&grid[x][y], &grid[x + distance][y - distance]);
        else if (InBounds(x - distance, y - distance) && grid[x - distance][y - distance].density > grid[x][y].density)
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
                DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, grid[x][y].color);

        DrawFPS(10, 10);
    EndDrawing();
}