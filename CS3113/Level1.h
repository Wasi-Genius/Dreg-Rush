#ifndef LEVEL_1_H
#define LEVEL_1_H

// Scene supplies the shared level state and the required scene functions
#include "Scene.h"

class Level1 : public Scene
{
private:
    // The arena contains 20 columns across and 12 rows down
    static constexpr int LEVEL_1_MAP_COLUMNS = 20;
    static constexpr int LEVEL_1_MAP_ROWS = 12;
    // Each tile occupies 64 pixels in both width and height
    static constexpr float LEVEL_1_TILE_SIZE = 64.0f;

    // This flat array stores the floor tile ID for every arena position
    unsigned int mFloorData[LEVEL_1_MAP_COLUMNS * LEVEL_1_MAP_ROWS] = {};
    // This flat array stores wall tile IDs and zeroes for open arena positions
    unsigned int mWallData[LEVEL_1_MAP_COLUMNS * LEVEL_1_MAP_ROWS] = {};
    // This separate map renders the arena walls over the floor
    Map *mWallMap = nullptr;

public:
    // The default constructor uses the scene origin and a black background
    Level1();
    // This constructor receives the camera origin and background color from main
    Level1(Vector2 origin, const char *bgColourHexCode);
    // The destructor ensures level resources are released safely
    ~Level1() override;

    // Initialise loads the arena maps and background music
    void initialise() override;
    // Update processes frame-based level behavior and streaming music
    void update(float deltaTime) override;
    // Render draws the floor and walls each frame
    void render() override;
    // Shutdown removes all heap allocations and unloaded Raylib resources
    void shutdown() override;

private:
    // Build the repeating stone floor pattern in row-major order
    void buildFloorData();
    // Build the border wall pattern around the arena
    void buildWallData();
};

#endif 