#ifndef LEVEL_1_H
#define LEVEL_1_H
#include "Scene.h"

class Level1 : public Scene
{
private:
    // Grid dimensions of the Level 1 boss arena:
    // 19 columns x 13 rows at 64 px per tile = 1216 x 832 world pixels
    // Centered at ORIGIN (600, 400) to fill entire 1200 x 800 window
    // Gives an odd column/row count so 3x3 altar sits dead center at (row 6, col 9)
    static constexpr int LEVEL_1_MAP_COLUMNS = 19;
    static constexpr int LEVEL_1_MAP_ROWS = 13;

    // Each 32x32 pixel-art tile is scaled 2x to 64x64 pixels on screen
    static constexpr float LEVEL_1_TILE_SIZE = 64.0f;
    static constexpr int LEVEL_1_MAP_CELLS = LEVEL_1_MAP_COLUMNS * LEVEL_1_MAP_ROWS;

    // Flat row-major tile-ID arrays (index = row * LEVEL_1_MAP_COLUMNS + column)
    // Tile ID 0 means empty/transparent; IDs >= 1 are 1-based cells in the atlas
    unsigned int mGrassData[LEVEL_1_MAP_CELLS] = {};
    unsigned int mFloorData[LEVEL_1_MAP_CELLS] = {};
    unsigned int mDaisData[LEVEL_1_MAP_CELLS] = {};
    unsigned int mWallData[LEVEL_1_MAP_CELLS] = {};
    unsigned int mPropsData[LEVEL_1_MAP_CELLS] = {};
    unsigned int mPlantData[LEVEL_1_MAP_CELLS] = {};

    // Layered Map objects drawn back-to-front in render():
    //   1. mGrassMap      -> outer grass border, wildflowers, and cobblestone  path
    //   2. mGameState.map -> main stone courtyard floor inside the arena walls
    //   3. mDaisMap       -> raised platform in the middle of the arena
    //   4. mWallMap       -> enclosing stone & brick arena walls (with south gate gap)
    //   5. mPropsMap      -> central rune altar, corner pillars, lanterns, and rubble
    //   6. mPlantMap      -> outer bushes and grass tufts outside the walls
    
    Map *mGrassMap = nullptr;
    Map *mDaisMap = nullptr;
    Map *mWallMap = nullptr;
    Map *mPropsMap = nullptr;
    Map *mPlantMap = nullptr;

public:

    Level1();

    // Constructor receives the camera origin and background color from main
    Level1(Vector2 origin, const char *bgColourHexCode);
    ~Level1() override;

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;

private:
    void setTile(unsigned int *layerData, int row, int column, unsigned int tileID);
    void buildGrassData();
    void buildFloorData();
    void buildDaisData();
    void buildWallData();
    void buildPropsData();
    void buildPlantData();
};

#endif
