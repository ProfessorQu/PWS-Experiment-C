#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "raylib.h"

#define MAX(i, j) (((i) > (j)) ? (i) : (j))

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
typedef struct {
    int id;
    Color color;

    int density;
    int spreadRate;

    int numReactions;
    int reactions[3][5];
} Substance;

#define numSubstances 16

Substance substances[numSubstances] = {
    { 0,  WHITE,     0,    0  },                                 // Air
    { 1,  BLUE,      997,  10 },                                 // H2O (l)
    //                              with
    //                              |   into
    //                              |  |  |
    { 2,  BEIGE,     2160, 0,  1, {{1, 9, 8}} },                 // NaCl (s)
    { 3,  DARKGRAY,  -1,   30 },                                 // CO2 (g)
    { 4,  ORANGE,    -4,   37, 1, {{1, 8, 3}}},                  // CH4 (g)
    { 5,  RED,       -3,   40, 2, {{1, 15, 15}, {3, 1, 14}}},    // NH3 (g)
    { 6,  PURPLE,    1513, 10, 2, {{2, 10, 11}, {5, 8, 12}}},    // HNO3 (l)
    { 7,  GRAY,      1738, 0,  1, {{1, 8, 13}}},                 // Mg (s)
    { 8,  PINK,      -5,   50 },                                 // H2 (g)
    { 9,  GOLD,      1110, 10 },                                 // NaOCl (l)
    { 10, GREEN,     -2,   30 },                                 // HCl (g)
    { 11, BROWN,     2260, 0  },                                 // NaNO3 (s)
    { 12, LIME,      1730, 0  },                                 // NH4NO3 (s)
    { 13, MAGENTA,   3580, 0  },                                 // MgO (s)
    { 14, LIGHTGRAY, 1335, 0  },                                 // NH2CONH2 (s)
    { 15, VIOLET,    900,  10 },                                 // NH4OH (aq)
};

char* substanceNames[numSubstances] = {
    "Air (g)",
    "H2O (l)",
    "NaCl (s)",
    "CO2 (g)",
    "CH4 (g)",
    "NH3 (g)",
    "HNO3 (l)",
    "Mg (s)",
    "H2 (g)",
    "NaOCl (l)",
    "HCl (g)",
    "NaNO3 (s)",
    "NH4NO3 (s)",
    "MgO (s)",
    "NH2CONH2 (s)",
    "NH4OH (aq)",
};

#define screenWidth 900
#define screenHeight 900

#define cellSize 5

#define gridWidth (screenWidth / cellSize)
#define gridHeight (screenHeight / cellSize)

#define fontSize 40
#define screenMargin 10

//----------------------------------------------------------------------------------
// Local Functions Declaration
//----------------------------------------------------------------------------------
void ResetGrid();

void Update();

void Inputs();
void SetSubstance(int x, int y, int size, Substance type);

void UpdateGridUpward();
void UpdateGridDownward();

bool Fall(int x, int y);
bool FallDown(int x, int y);
bool FallUp(int x, int y);

void UpdateSubstance(int x, int y, bool up);

bool Spread(int x, int y, int spread);
bool SpreadDown(int x, int y, int randomDirection, int spread);
bool SpreadUp(int x, int y, int randomDirection, int spread);

bool React(int x, int y, Substance with, Substance into1, Substance into2);

void Draw();
void DrawSubstanceGrid();
void DrawInfo();

//----------------------------------------------------------------------------------
// Local Variables Definition (local to this module)
//----------------------------------------------------------------------------------
const int gravity = 3;
bool left = false;

int selectedId = 1;
int size = 1;

Substance grid[gridWidth][gridHeight];

bool InBounds(int x, int y)
{
    return 0 <= x && x < gridWidth && 0 <= y && y < gridHeight;
}

bool swap(Substance *a, Substance *b)
{
    Substance temp = *a;
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
    CloseWindow();

    return 0;
}

void ResetGrid()
{
    for (int x = 0; x < gridWidth; x++)
        for (int y = 0; y < gridHeight; y++)
            grid[x][y] = substances[0];
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

    for (int i = 0; i < numSubstances - 1; i++)
        if (IsKeyPressed(KEY_ONE + i))
            selectedId = i + 1;

    int movement = GetMouseWheelMove();
    // Update brush size
    if (IsKeyDown(KEY_LEFT_SHIFT)) {
        size += movement;
        if (size < 0)
            size = 0;
        else if (size > 20)
            size = 20;
    // Update selected element
    } else {
        if (movement < 0) {
            selectedId -= 1;
            if (selectedId < 0) {
                selectedId = numSubstances - 1;
            }
        } else if (movement > 0) {
            selectedId += 1;
        }

        selectedId %= numSubstances;
        if (selectedId == 0) {
            if (movement > 0) {
                selectedId = 1;
            } else {
                selectedId = numSubstances - 1;
            }
        }
    }

    // Spawning Cells
    //---------------------------------------------------------
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        SetSubstance(mouseX, mouseY, size, substances[selectedId]);
    else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        SetSubstance(mouseX, mouseY, size, substances[0]);
    
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
                UpdateSubstance(x, y, true);
    else
        for (int y = gridHeight - 1; y >= 0 ; y--)
            for (int x = gridWidth - 1; x >= 0; x--)
                UpdateSubstance(x, y, true);
}

void UpdateGridDownward()
{
    if (left)
        for (int y = 0; y < gridHeight; y++)
            for (int x = 0; x < gridWidth; x++)
                UpdateSubstance(x, y, false);
    else
        for (int y = 0; y < gridHeight; y++)
            for (int x = gridWidth - 1; x >= 0; x--)
                UpdateSubstance(x, y, false);
}

//----------------------------------------------------------------------------------
// Spawn cells
//----------------------------------------------------------------------------------
void SetSubstance(int x, int y, int size, Substance type)
{
    // Set cells in a (2*size)x(2*size) square
    for (int cx = -size; cx <= size; cx++)
        for (int cy = -size; cy <= size; cy++)
            if (InBounds(x + cx, y + cy))
                grid[x + cx][y + cy] = type;
}

void UpdateSubstance(int x, int y, bool up)
{
    // Preconditions
    if (up && grid[x][y].density < 0)
        return;
    else if (!up && grid[x][y].density > 0)
        return;
    else if (grid[x][y].id == 0)
        return;

    // React
    //---------------------------------------------------------
    if (grid[x][y].numReactions > 0)
    {
        bool reacted = false;
        for (int i = 0; i < grid[x][y].numReactions; i++)
        {
            int* reaction = grid[x][y].reactions[i];
            Substance with = substances[reaction[0]];
            Substance into1 = substances[reaction[1]];
            Substance into2 = substances[reaction[2]];

            if (React(x, y, with, into1, into2))
            {
                reacted = true;
                break;
            }
        }

        if (!reacted)
            // Fall
            //---------------------------------------------------------
            if (!Fall(x, y))
                // Spread
                //---------------------------------------------------------
                Spread(x, y, grid[x][y].spreadRate);
    }
    // Fall
    //---------------------------------------------------------
    else if (!Fall(x, y))
        // Spread
        //---------------------------------------------------------
        Spread(x, y, grid[x][y].spreadRate);
}

//----------------------------------------------------------------------------------
// Spread
//----------------------------------------------------------------------------------
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
bool React(int x, int y, Substance with, Substance into1, Substance into2)
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
        
        DrawSubstanceGrid();
        DrawInfo();
    EndDrawing();
}

void DrawSubstanceGrid() {
    for (int x = 0; x < gridWidth; x++)
        for (int y = 0; y < gridHeight; y++)
            DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, grid[x][y].color);
}

void DrawInfo() {
    // Get mouse position
    int mouseX = GetMouseX() / cellSize;
    int mouseY = GetMouseY() / cellSize;

    // Display Hovered Over substance
    //-----------------------------------------------------
    int hoveredOverId = 0;
    int hoveredOverTextWidth = 0;
    if (InBounds(mouseX, mouseY)) {
        hoveredOverId = grid[mouseX][mouseY].id;
        hoveredOverTextWidth = MeasureText(substanceNames[hoveredOverId], fontSize);
    }
    
    // Calculate Selected substance text width
    int selectedTextWidth = MeasureText(substanceNames[selectedId], fontSize);

    // Draw Info Rectangle
    //-----------------------------------------------------
    int rectTextWidth = MAX(hoveredOverTextWidth, selectedTextWidth);

    Color rectColor = RAYWHITE;
    rectColor.a = 175;
    DrawRectangle(
        screenWidth - rectTextWidth - screenMargin * 2, 0,
        rectTextWidth + screenMargin * 2, fontSize * 3 + screenMargin * 2,
        rectColor
    );

    // Draw Selected substance name
    DrawText(
        substanceNames[selectedId],
        screenWidth - selectedTextWidth - screenMargin,
        screenMargin, fontSize, substances[selectedId].color
    );

    // Draw Hovered Over substance name
    if (hoveredOverTextWidth != 0) {
        // Make Air (g) colors' black
        Color hoveredColor = (hoveredOverId == 0) ? BLACK : substances[hoveredOverId].color;
        DrawText(
            substanceNames[hoveredOverId],
            screenWidth - hoveredOverTextWidth - screenMargin,
            screenMargin + fontSize, fontSize,
            hoveredColor
        );
    }

    // Show the size of the brush
    //-----------------------------------------------------
    char sizeText[10];
    itoa(size, sizeText, 10);
    int sizeTextWidth = MeasureText(sizeText, fontSize);

    DrawText(
        sizeText,
        screenWidth - sizeTextWidth - screenMargin,
        screenMargin + fontSize * 2, fontSize,
        BLACK 
    );

    // Draw FPS
    DrawFPS(screenMargin, screenMargin);
}
