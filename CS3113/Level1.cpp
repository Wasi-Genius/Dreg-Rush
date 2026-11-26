#include "Level1.h"

// Use scene defaults when no origin or background color is supplied
Level1::Level1() : Scene{ { 0.0f, 0.0f }, "#000000" } {}

// Pass the supplied origin and background color into the base Scene class
Level1::Level1(Vector2 origin, const char *bgColourHexCode)
    : Scene{ origin, bgColourHexCode } {}

// Release resources when the Level1 object is destroyed
Level1::~Level1()
{
    shutdown();
}

// Fill the floor array with tile IDs from the stone ground atlas
void Level1::buildFloorData()
{
    // Row loops move through the map from top to bottom
    for (int row = 0; row < LEVEL_1_MAP_ROWS; row++)
    {
        // Column loops move through the map from left to right
        for (int column = 0; column < LEVEL_1_MAP_COLUMNS; column++)
        {
            // Convert a two-dimensional map position into a flat array index
            const int index = row * LEVEL_1_MAP_COLUMNS + column;
            // Cycle through eight atlas tile IDs to create a varied ground pattern
            mFloorData[index] =
                static_cast<unsigned int>((row + column) % 8 + 1);
        }
    }
}

// Fill the wall array so only the arena border contains wall tiles
void Level1::buildWallData()
{
    // Row loops place wall tiles along both horizontal borders
    for (int row = 0; row < LEVEL_1_MAP_ROWS; row++)
    {
        // Column loops place wall tiles along both vertical borders
        for (int column = 0; column < LEVEL_1_MAP_COLUMNS; column++)
        {
            // Convert the row and column into the matching flat array index
            const int index = row * LEVEL_1_MAP_COLUMNS + column;
            // Mark a position as a border if it is on the top, bottom, left, or right edge
            const bool onBorder =
                row == 0 ||
                row == LEVEL_1_MAP_ROWS - 1 ||
                column == 0 ||
                column == LEVEL_1_MAP_COLUMNS - 1;

            // A border position uses wall tile 1 while an interior position stays empty
            mWallData[index] = onBorder ? 1 : 0;
        }
    }
}

// Load and prepare all resources owned by the first boss arena
void Level1::initialise()
{
    // Populate both the floor and wall arrays before constructing maps
    buildFloorData();
    buildWallData();

    // Create the stone-ground Map used by the arena floor
    mGameState.map = new Map(
        LEVEL_1_MAP_COLUMNS,
        LEVEL_1_MAP_ROWS,
        mFloorData,
        "assets/Tilesets/Grassy Unity Tileset/TX Tileset Stone Ground.png",
        LEVEL_1_TILE_SIZE,
        16,
        16,
        mOrigin
    );

    // Create a separate wall Map so walls can be rendered above the floor
    mWallMap = new Map(
        LEVEL_1_MAP_COLUMNS,
        LEVEL_1_MAP_ROWS,
        mWallData,
        "assets/Tilesets/Grassy Unity Tileset/TX Tileset Wall.png",
        LEVEL_1_TILE_SIZE,
        16,
        16,
        mOrigin
    );

    // Load the level-specific arena music stream
    mGameState.bgm = LoadMusicStream("assets/Music/Boss Music Level 1.mp3");
    // Set the music to a comfortable half-volume
    SetMusicVolume(mGameState.bgm, 0.5f);
    // Start the background music immediately
    PlayMusicStream(mGameState.bgm);
    // Begin this scene without requesting a scene transition
    mGameState.nextSceneID = NO_SCENE;
}

// Keep streamed level music playing during fixed-timestep updates
void Level1::update(float)
{
    UpdateMusicStream(mGameState.bgm);
}

// Draw the arena background, floor, and border walls
void Level1::render()
{
    // Clear the screen to the scene's configured background color
    ClearBackground(ColorFromHex(mBGColourHexCode));
    // Render every nonzero stone floor tile
    mGameState.map->render();
    // Render every nonzero border wall tile above the floor
    mWallMap->render();
}

// Release all heap and Raylib resources owned by Level1
void Level1::shutdown()
{
    // Delete the wall map and clear the pointer to prevent a dangling reference
    delete mWallMap;
    mWallMap = nullptr;

    // Delete the floor map and clear the pointer to prevent a dangling reference
    delete mGameState.map;
    mGameState.map = nullptr;

    // Unload the streamed music and reset its handle
    UnloadMusicStream(mGameState.bgm);
    mGameState.bgm = {};
    // Clear any pending transition before leaving or restarting the scene
    mGameState.nextSceneID = NO_SCENE;
}
