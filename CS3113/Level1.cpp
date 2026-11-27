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

// Convert a 2D (row, column) grid position into a 1D row-major array index
// Store a 1-based atlas tile ID (0 leaves the cell empty/transparent)
void Level1::setTile(
    unsigned int *layerData,
    int row,
    int column,
    unsigned int tileID
)
{
    if (row < 0 || row >= LEVEL_1_MAP_ROWS ||
        column < 0 || column >= LEVEL_1_MAP_COLUMNS)
    {
        return;
    }

    layerData[row * LEVEL_1_MAP_COLUMNS + column] = tileID;
}

// Layer 1: Fill the entire background with grass, flowers, and a south stone path
// Uses "TX Tileset Grass.png" (256x256 px, 32x32 px per tile -> 8 columns x 8 rows)
// Atlas ID formula: tileID = atlasRow * 8 + atlasCol + 1
void Level1::buildGrassData()
{
    // Pure grass, tufted grass, and subtle wildflower tiles from rows 0..3 (IDs 1..32)
    const unsigned int grassChoices[] = {
        1, 1, 1, 2, 3, 4, 9, 10, 11, 12, 17, 18, 19, 25, 26, 27, 28, 5, 14, 21, 31
    };
    const int grassChoiceCount =
        static_cast<int>(sizeof(grassChoices) / sizeof(grassChoices[0]));

    for (int row = 0; row < LEVEL_1_MAP_ROWS; row++)
    {
        for (int column = 0; column < LEVEL_1_MAP_COLUMNS; column++)
        {
            // Pseudo-random pattern based on (row, column) so grass looks organic
            const int patternHash =
                (row * 37 + column * 17 + row * column * 7) % grassChoiceCount;
            setTile(mGrassData, row, column, grassChoices[patternHash]);
        }
    }

    // Place a 3-tile-wide cobblestone approach path leading up to the south gate
    // (columns 8..10, rows 11..12) using the grass/stone paver tiles (33..44)
    setTile(mGrassData, 11, 8, 35);
    setTile(mGrassData, 11, 9, 33);
    setTile(mGrassData, 11, 10, 36);

    setTile(mGrassData, 12, 8, 43);
    setTile(mGrassData, 12, 9, 41);
    setTile(mGrassData, 12, 10, 44);
}

// Layer 2: Fill the walkable arena courtyard floor with seamless stone.
// Uses "TX Tileset Stone Ground.png" (256x256 px, 32x32 px per tile -> 8 columns x 8 rows)
void Level1::buildFloorData()
{
    // Start with all cells empty (0) so outer grass shows outside the arena walls
    for (int i = 0; i < LEVEL_1_MAP_CELLS; i++)
        mFloorData[i] = 0;

    // Fill only the courtyard interior (rows 3..10, columns 2..16)
    // Tile 10 (atlas row 1, col 1) is the seamless flat stone interior tile
    for (int row = 3; row <= 10; row++)
    {
        for (int column = 2; column <= 16; column++)
        {
            setTile(mFloorData, row, column, 10);
        }
    }
}

// Layer 3: Place a raised 7x5 stone battle dais in the center of the arena.
// Uses "TX Tileset Stone Ground.png" (8x8 atlas). Tiles 1..3, 9..11, and 17..19

void Level1::buildDaisData()
{
    for (int i = 0; i < LEVEL_1_MAP_CELLS; i++)
        mDaisData[i] = 0;

    // Center the dais on rows 4..8, columns 6..12 around (row 6, col 9)
    const int topRow = 4;
    const int bottomRow = 8;
    const int leftColumn = 6;
    const int rightColumn = 12;

    for (int row = topRow; row <= bottomRow; row++)
    {
        for (int column = leftColumn; column <= rightColumn; column++)
        {
            unsigned int tileID = 10; // Interior fill

            if (row == topRow && column == leftColumn)           tileID = 1;  // Top-left corner
            else if (row == topRow && column == rightColumn)     tileID = 3;  // Top-right corner
            else if (row == bottomRow && column == leftColumn)   tileID = 17; // Bottom-left corner
            else if (row == bottomRow && column == rightColumn)  tileID = 19; // Bottom-right corner
            else if (row == topRow)                              tileID = 2;  // Top edge
            else if (row == bottomRow)                           tileID = 18; // Bottom edge
            else if (column == leftColumn)                       tileID = 9;  // Left edge
            else if (column == rightColumn)                      tileID = 11; // Right edge

            setTile(mDaisData, row, column, tileID);
        }
    }
}

// Layer 4: Enclose the arena with  top-down stone walls.
// Uses "TX Tileset Wall.png" (512x512 px, 32x32 px per tile -> 16 columns x 16 rows).
// Each wall edge has its own orientation tiles:
//   - North wall is 2 rows tall (rows 1..2): caps 21..25 over brick faces 37..41
//   - Grated wall windows: 103 (top) over 119 (bottom)
//   - Left vertical rim: 53 (hugs the right edge of column 1)
//   - Right vertical rim: 57 (hugs the left edge of column 17)
//   - South horizontal rim: 69 (bottom-left), 71/72 (middle), 73 (bottom-right)

void Level1::buildWallData()
{
    for (int i = 0; i < LEVEL_1_MAP_CELLS; i++)
        mWallData[i] = 0;

    // North-west and north-east 2-tile-tall wall corners
    setTile(mWallData, 1, 1, 21);
    setTile(mWallData, 2, 1, 37);
    setTile(mWallData, 1, 17, 25);
    setTile(mWallData, 2, 17, 41);

    // North brick wall spanning columns 2..16 across rows 1..2
    for (int column = 2; column <= 16; column++)
    {
        if (column == 2)
        {
            setTile(mWallData, 1, column, 22);
            setTile(mWallData, 2, column, 38);
        }
        else if (column == 16)
        {
            setTile(mWallData, 1, column, 24);
            setTile(mWallData, 2, column, 40);
        }
        else if (column == 5 || column == 9 || column == 13)
        {
            // Place three evenly spaced grated wall alcoves along the north wall
            setTile(mWallData, 1, column, 103);
            setTile(mWallData, 2, column, 119);
        }
        else
        {
            // Alternate the two repeatable brick variants to avoid uniform seams
            setTile(mWallData, 1, column, (column % 2 == 0) ? 23u : 24u);
            setTile(mWallData, 2, column, (column % 2 == 0) ? 39u : 40u);
        }
    }

    // West and east vertical wall rims along the sides of the courtyard (rows 3..10)
    for (int row = 3; row <= 10; row++)
    {
        setTile(mWallData, row, 1, 53);
        setTile(mWallData, row, 17, 57);
    }

    // South wall rim along row 11, leaving columns 8..10 open as the arena entrance
    setTile(mWallData, 11, 1, 69);
    setTile(mWallData, 11, 17, 73);
    for (int column = 2; column <= 16; column++)
    {
        if (column >= 8 && column <= 10) continue; // Open south gate
        setTile(mWallData, 11, column, (column % 2 == 0) ? 71u : 72u);
    }
}

// Layer 5: Place props over the stone floor and dais.
// Uses "TX Props.png" (512x512 px, 32x32 px per tile -> 16 columns x 16 rows)
void Level1::buildPropsData()
{
    for (int i = 0; i < LEVEL_1_MAP_CELLS; i++)
        mPropsData[i] = 0;

    // Central 3x3 circular rune altar centered on (row 6, col 9)
    // Atlas block: cols 11..13 (0-based), rows 8..10 (0-based) -> IDs 140..174
    setTile(mPropsData, 5, 8, 140);
    setTile(mPropsData, 5, 9, 141);
    setTile(mPropsData, 5, 10, 142);

    setTile(mPropsData, 6, 8, 156);
    setTile(mPropsData, 6, 9, 157);
    setTile(mPropsData, 6, 10, 158);

    setTile(mPropsData, 7, 8, 172);
    setTile(mPropsData, 7, 9, 173);
    setTile(mPropsData, 7, 10, 174);

    // Two  3-tile-tall stone pillars in the back corners (columns 3 and 15)
    setTile(mPropsData, 3, 3, 92);
    setTile(mPropsData, 4, 3, 108);
    setTile(mPropsData, 5, 3, 124);

    setTile(mPropsData, 3, 15, 92);
    setTile(mPropsData, 4, 15, 108);
    setTile(mPropsData, 5, 15, 124);

    // Two broken 2-tile-tall ruined pillars in the front corners (columns 3 and 15)
    setTile(mPropsData, 8, 3, 110);
    setTile(mPropsData, 9, 3, 126);

    setTile(mPropsData, 8, 15, 110);
    setTile(mPropsData, 9, 15, 126);

    // Two 2-tile-tall stone lanterns flanking the inside of the south entrance
    setTile(mPropsData, 9, 7, 63);
    setTile(mPropsData, 10, 7, 79);

    setTile(mPropsData, 9, 11, 63);
    setTile(mPropsData, 10, 11, 79);

    // Small scattered stone rubble near the ruined front pillars
    setTile(mPropsData, 9, 4, 250);
    setTile(mPropsData, 10, 3, 243);
    setTile(mPropsData, 9, 14, 248);
    setTile(mPropsData, 10, 15, 245);
}

// Layer 6: Frame the outside of the arena with bushes and grass tufts.
// Uses "TX Plant.png" (512x512 px, 32x32 px per tile -> 16 columns x 16 rows)
void Level1::buildPlantData()
{
    for (int i = 0; i < LEVEL_1_MAP_CELLS; i++)
        mPlantData[i] = 0;

    // Bushes (IDs 98, 100, 102, 108) along the outer grass border
    setTile(mPlantData, 0, 0, 108);
    setTile(mPlantData, 0, 4, 100);
    setTile(mPlantData, 0, 14, 102);
    setTile(mPlantData, 0, 18, 108);

    setTile(mPlantData, 4, 0, 102);
    setTile(mPlantData, 9, 0, 100);
    setTile(mPlantData, 4, 18, 102);
    setTile(mPlantData, 9, 18, 98);

    setTile(mPlantData, 12, 2, 100);
    setTile(mPlantData, 12, 6, 98);
    setTile(mPlantData, 12, 12, 102);
    setTile(mPlantData, 12, 16, 108);

    // Small decorative grass tufts (IDs 193..196)
    setTile(mPlantData, 0, 7, 194);
    setTile(mPlantData, 0, 11, 196);
    setTile(mPlantData, 11, 4, 193);
    setTile(mPlantData, 11, 14, 195);
}

// Load and prepare all resources for the level
void Level1::initialise()
{
    // Populate every layer's 1D tile array before constructing its Map object
    buildGrassData();
    buildFloorData();
    buildDaisData();
    buildWallData();
    buildPropsData();
    buildPlantData();

    // Grass & Stone Ground sheets are 256x256 with 32x32 tiles -> 8 columns x 8 rows
    mGrassMap = new Map(
        LEVEL_1_MAP_COLUMNS,
        LEVEL_1_MAP_ROWS,
        mGrassData,
        "assets/Tilesets/Grassy Unity Tileset/TX Tileset Grass.png",
        LEVEL_1_TILE_SIZE,
        8,
        8,
        mOrigin
    );

    mGameState.map = new Map(
        LEVEL_1_MAP_COLUMNS,
        LEVEL_1_MAP_ROWS,
        mFloorData,
        "assets/Tilesets/Grassy Unity Tileset/TX Tileset Stone Ground.png",
        LEVEL_1_TILE_SIZE,
        8,
        8,
        mOrigin
    );

    mDaisMap = new Map(
        LEVEL_1_MAP_COLUMNS,
        LEVEL_1_MAP_ROWS,
        mDaisData,
        "assets/Tilesets/Grassy Unity Tileset/TX Tileset Stone Ground.png",
        LEVEL_1_TILE_SIZE,
        8,
        8,
        mOrigin
    );

    // Wall, Props, and Plant sheets are 512x512 with 32x32 tiles -> 16 columns x 16 rows
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

    mPropsMap = new Map(
        LEVEL_1_MAP_COLUMNS,
        LEVEL_1_MAP_ROWS,
        mPropsData,
        "assets/Tilesets/Grassy Unity Tileset/TX Props.png",
        LEVEL_1_TILE_SIZE,
        16,
        16,
        mOrigin
    );

    mPlantMap = new Map(
        LEVEL_1_MAP_COLUMNS,
        LEVEL_1_MAP_ROWS,
        mPlantData,
        "assets/Tilesets/Grassy Unity Tileset/TX Plant.png",
        LEVEL_1_TILE_SIZE,
        16,
        16,
        mOrigin
    );

    // Load music
    mGameState.bgm = LoadMusicStream("assets/Music/Boss Music Level 1.mp3");
    // Set the music to half-volume
    SetMusicVolume(mGameState.bgm, 0.5f);
    // Start music immediately
    PlayMusicStream(mGameState.bgm);
    // Begin this scene without requesting a scene transition
    mGameState.nextSceneID = NO_SCENE;
}

// Keep music playing during fixed-timestep updates
void Level1::update(float)
{
    UpdateMusicStream(mGameState.bgm);
}

// Draw every arena layer back-to-front so each layer sits cleanly on the one below
void Level1::render()
{
    // Clear the screen to the scene's configured background color
    ClearBackground(ColorFromHex(mBGColourHexCode));

    mGrassMap->render();      // 1. Grass & cobblestone entry path
    mGameState.map->render(); // 2. Stone courtyard floor
    mDaisMap->render();       // 3. Raised central stone battle platform
    mWallMap->render();       // 4. Enclosing brick & stone walls
    mPropsMap->render();      // 5. Rune altar, pillars, lanterns, and rubble
    mPlantMap->render();      // 6. Outer bushes and grass tufts
}

// Release resources owned by Level1 
void Level1::shutdown()
{
    // Delete every Map and clear its pointer so repeated shutdown() calls are safe
    delete mPlantMap;
    mPlantMap = nullptr;

    delete mPropsMap;
    mPropsMap = nullptr;

    delete mWallMap;
    mWallMap = nullptr;

    delete mDaisMap;
    mDaisMap = nullptr;

    delete mGameState.map;
    mGameState.map = nullptr;

    delete mGrassMap;
    mGrassMap = nullptr;

    // Unload the music
    UnloadMusicStream(mGameState.bgm);
    mGameState.bgm = {};
    
    // Clear any pending transition before leaving or restarting the scene
    mGameState.nextSceneID = NO_SCENE;
}
