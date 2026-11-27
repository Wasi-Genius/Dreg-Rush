#ifndef LEVEL_1_H
#define LEVEL_1_H

// Scene supplies the shared level state (mGameState, mOrigin, mBGColourHexCode)
// and the required virtual lifecycle functions (initialise, update, render, shutdown)
#include "Scene.h"
#include "ShaderProgram.h"

class Level1 : public Scene
{
private:
    // Grid dimensions of the Level 1 boss arena:
    // 19 columns x 13 rows at 64 px per tile = 1216 x 832 world pixels.
    // Centered at ORIGIN (600, 400), this fills the entire 1200 x 800 window
    // and gives an odd column/row count so a 3x3 altar sits dead center at (row 6, col 9).
    static constexpr int LEVEL_1_MAP_COLUMNS = 19;
    static constexpr int LEVEL_1_MAP_ROWS = 13;
    // Each 32x32 pixel-art tile is scaled 2x to 64x64 pixels on screen
    static constexpr float LEVEL_1_TILE_SIZE = 64.0f;
    static constexpr int LEVEL_1_MAP_CELLS = LEVEL_1_MAP_COLUMNS * LEVEL_1_MAP_ROWS;

    // 4 pillar bases + 2 lantern bases + 1 south entrance gate blocker
    static constexpr int LEVEL_1_OBSTACLE_COUNT = 7;

    // Flat row-major tile-ID arrays (index = row * LEVEL_1_MAP_COLUMNS + column).
    // Tile ID 0 means empty/transparent; IDs >= 1 are 1-based cells in the atlas.
    unsigned int mGrassData[LEVEL_1_MAP_CELLS] = {};
    unsigned int mFloorData[LEVEL_1_MAP_CELLS] = {};
    unsigned int mDaisData[LEVEL_1_MAP_CELLS] = {};
    unsigned int mWallData[LEVEL_1_MAP_CELLS] = {};
    unsigned int mPropsData[LEVEL_1_MAP_CELLS] = {};
    unsigned int mPlantData[LEVEL_1_MAP_CELLS] = {};

    // Layered Map objects drawn back-to-front in render():
    //   1. mGrassMap      -> outer grass border, wildflowers, and cobblestone entry path
    //   2. mGameState.map -> main stone courtyard floor inside the arena walls
    //   3. mDaisMap       -> raised stone battle platform in the middle of the arena
    //   4. mWallMap       -> enclosing stone & brick arena walls (also used for wall collision)
    //   5. mPropsMap      -> central rune altar, corner pillars, lanterns, and rubble
    //   6. mPlantMap      -> outer bushes and grass tufts outside the walls
    Map *mGrassMap = nullptr;
    Map *mDaisMap = nullptr;
    Map *mWallMap = nullptr;
    Map *mPropsMap = nullptr;
    Map *mPlantMap = nullptr;

    // Invisible solid collision boxes matching pillar bases, lanterns, and gate bounds
    Entity mObstacles[LEVEL_1_OBSTACLE_COUNT];

    // Scene-owned protagonist sound effects (loaded in initialise, unloaded in shutdown)
    Sound mAttackSound{};
    Sound mDodgeSound{};
    Sound mFootstepSound{};
    float mFootstepTimer = 0.0f;

    // Lecture 10 GLSL shader driving the protagonist's phantom-dodge effect
    ShaderProgram mDodgeShader;

public:
    // The default constructor uses the scene origin and a black background
    Level1();
    // This constructor receives the camera origin and background color from main
    Level1(Vector2 origin, const char *bgColourHexCode);
    // The destructor ensures level resources are released safely
    ~Level1() override;

    // Initialise loads the arena maps, protagonist, obstacles, and audio
    void initialise() override;
    // Update processes player movement/combat, collisions, and streaming music
    void update(float deltaTime) override;
    // Render draws every arena layer and the protagonist each frame
    void render() override;
    // Shutdown removes all heap allocations and unloaded Raylib resources
    void shutdown() override;

private:
    // Helper to convert (row, column) into the flat array index and assign a tile ID
    void setTile(unsigned int *layerData, int row, int column, unsigned int tileID);

    // Helper to convert a 0-based (row, column) tile cell into its world-space center
    Vector2 getTileCenter(float row, float column) const;

    // Setup helpers called inside initialise()
    void buildGrassData();
    void buildFloorData();
    void buildDaisData();
    void buildWallData();
    void buildPropsData();
    void buildPlantData();
    void initProtagonist();
    void initObstacles();
};

#endif
