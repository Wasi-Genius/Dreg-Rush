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

// Convert a 2D (row, column) grid position into a 1D row-major array index and store a 1-based atlas tile ID (0 leaves the cell empty/transparent)
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

// Convert a (row, column) tile coordinate into world-space center coordinates using the floor map's top-left boundary so entities align cleanly with tiles
Vector2 Level1::getTileCenter(float row, float column) const
{
    return {
        mGameState.map->getLeftBoundary() + (column + 0.5f) * LEVEL_1_TILE_SIZE,
        mGameState.map->getTopBoundary() + (row + 0.5f) * LEVEL_1_TILE_SIZE
    };
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
            // Pseudo-random pattern based on (row, column) so the grass looks organic rather than repeating a rigid diagonal stripe
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
// Uses "TX Tileset Stone Ground.png" (8x8 atlas)
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

// Layer 4: Enclose the arena with top-down stone walls
// Uses "TX Tileset Wall.png" (512x512 px, 32x32 px per tile -> 16 columns x 16 rows).
// Because mWallMap is passed as the collision map to Entity::update, any nonzero tile here also acts as a solid wall barrier for the protagonist.
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

    // Two 3-tile-tall stone pillars in the back corners (columns 3 and 15)
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

// Layer 6: Frame the outside of the arena with bushes and grass tufts
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

// Configure the protagonist Entity with 4-way IDLE, RUN, and ATTACK 1 sprite sheets, top-down movement, foot-aligned collision, and fast-paced dodge settings
void Level1::initProtagonist()
{
    // All protagonist sprite sheets are 768x80 px: 1 row of 8 frames (96x80 px each)
    const std::vector<int> eightFrames = { 0, 1, 2, 3, 4, 5, 6, 7 };
    const Vector2 sheetDimensions = { 1.0f, 8.0f };

    const std::map<Direction, const char *> idlePaths = {
        { UP,    "assets/Protagonist Sprites/IDLE/idle_up.png"    },
        { DOWN,  "assets/Protagonist Sprites/IDLE/idle_down.png"  },
        { LEFT,  "assets/Protagonist Sprites/IDLE/idle_left.png"  },
        { RIGHT, "assets/Protagonist Sprites/IDLE/idle_right.png" }
    };
    const std::map<Direction, const char *> runPaths = {
        { UP,    "assets/Protagonist Sprites/RUN/run_up.png"    },
        { DOWN,  "assets/Protagonist Sprites/RUN/run_down.png"  },
        { LEFT,  "assets/Protagonist Sprites/RUN/run_left.png"  },
        { RIGHT, "assets/Protagonist Sprites/RUN/run_right.png" }
    };
    const std::map<Direction, const char *> attackPaths = {
        { UP,    "assets/Protagonist Sprites/ATTACK 1/attack1_up.png"    },
        { DOWN,  "assets/Protagonist Sprites/ATTACK 1/attack1_down.png"  },
        { LEFT,  "assets/Protagonist Sprites/ATTACK 1/attack1_left.png"  },
        { RIGHT, "assets/Protagonist Sprites/ATTACK 1/attack1_right.png" }
    };

    mGameState.player = new Entity();
    mGameState.player->setEntityType(PLAYER);
    mGameState.player->setTopDown(true);

    // Scale 96x80 frames 2x (192x160) to match the 2x-scaled 64x64 arena tiles
    mGameState.player->setScale({ 192.0f, 160.0f });

    // Place the collision box over the protagonist's feet so hair/shoulders can overlap lower edges of walls and pillar tops naturally in top-down view
    mGameState.player->setColliderDimensions({ 30.0f, 24.0f });
    mGameState.player->setColliderOffset({ 0.0f, 22.0f });
    mGameState.player->setSpeed(285);

    mGameState.player->setDirectionalRunAnimations(
        runPaths, sheetDimensions, eightFrames, 14
    );
    mGameState.player->setDirectionalIdleAnimations(
        idlePaths, sheetDimensions, eightFrames, 8
    );
    mGameState.player->setDirectionalAttackAnimations(
        attackPaths, sheetDimensions, eightFrames, 30
    );

    // Spawn just inside the south arena entrance facing UP toward the central altar
    mGameState.player->setPosition(getTileCenter(9.4f, 9.0f));
    mGameState.player->setDirection(UP);
}

// Create invisible solid collision boxes at pillar bases, lanterns, and the south gate so the protagonist cannot walk through stone props or off the bottom of the screen
void Level1::initObstacles()
{
    // Back-left and back-right intact pillar bases (row 5, cols 3 and 15)
    mObstacles[0].setEntityType(BLOCK);
    mObstacles[0].setPosition(getTileCenter(5.0f, 3.0f));
    mObstacles[0].setColliderDimensions({ 44.0f, 36.0f });

    mObstacles[1].setEntityType(BLOCK);
    mObstacles[1].setPosition(getTileCenter(5.0f, 15.0f));
    mObstacles[1].setColliderDimensions({ 44.0f, 36.0f });

    // Front-left and front-right ruined pillar bases (row 9, cols 3 and 15)
    mObstacles[2].setEntityType(BLOCK);
    mObstacles[2].setPosition(getTileCenter(9.0f, 3.0f));
    mObstacles[2].setColliderDimensions({ 44.0f, 36.0f });

    mObstacles[3].setEntityType(BLOCK);
    mObstacles[3].setPosition(getTileCenter(9.0f, 15.0f));
    mObstacles[3].setColliderDimensions({ 44.0f, 36.0f });

    // South gate stone lantern bases (row 10, cols 7 and 11)
    mObstacles[4].setEntityType(BLOCK);
    mObstacles[4].setPosition(getTileCenter(10.0f, 7.0f));
    mObstacles[4].setColliderDimensions({ 36.0f, 32.0f });

    mObstacles[5].setEntityType(BLOCK);
    mObstacles[5].setPosition(getTileCenter(10.0f, 11.0f));
    mObstacles[5].setColliderDimensions({ 36.0f, 32.0f });

    // Invisible barrier sealing the open south gate (row 11, cols 8..10) during combat
    mObstacles[6].setEntityType(BLOCK);
    mObstacles[6].setPosition(getTileCenter(11.1f, 9.0f));
    mObstacles[6].setColliderDimensions({ 3.0f * LEVEL_1_TILE_SIZE, 36.0f });
}

// Load and prepare all resources owned by the first boss arena
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

    // Initialize solid prop/gate blockers and the protagonist
    initObstacles();
    initProtagonist();

    // Load protagonist combat & movement sound effects owned by this scene
    mAttackSound   = LoadSound("assets/Audio/Protag Attack.wav");
    mDodgeSound    = LoadSound("assets/Audio/Protag Dodge.wav");
    mFootstepSound = LoadSound("assets/Audio/Protag Footstep.wav");
    mFootstepTimer = 0.0f;

    // Load the dodge shader
    mDodgeShader.load("assets/Shaders/dodge.vs", "assets/Shaders/dodge.fs");

    // Load the level music
    mGameState.bgm = LoadMusicStream("assets/Music/Boss Music Level 1.mp3");
    // Set the music to half-volume
    SetMusicVolume(mGameState.bgm, 0.5f);
    // Start music immediately
    PlayMusicStream(mGameState.bgm);
    // Begin this scene without requesting a scene transition
    mGameState.nextSceneID = NO_SCENE;
}

// Update music, protagonist movement/actions, and arena collisions each fixed step

void Level1::update(float deltaTime)
{
    UpdateMusicStream(mGameState.bgm);

    if (mGameState.player == nullptr) return;

    // Pass mWallMap (solid walls) and mObstacles (pillars/lanterns/gate) as colliders

    // mGameState.map is the walkable floor so it is NOT passed for solid collision

    mGameState.player->update(
        deltaTime,
        nullptr,
        mWallMap,
        mObstacles,
        LEVEL_1_OBSTACLE_COUNT
    );

    // Play one-shot SFX when an attack or dodge started on input

    if (mGameState.player->consumeAttackSoundPending())
        PlaySound(mAttackSound);

    if (mGameState.player->consumeDodgeSoundPending())
        PlaySound(mDodgeSound);

    // Play paced footstep sounds only while actively running (not dodging or swinging)

    const bool isRunning =
        !mGameState.player->isDodging() &&
        !mGameState.player->isAttacking() &&
        GetLength(mGameState.player->getMovement()) > 0.0f;

    if (isRunning)
    {
        mFootstepTimer -= deltaTime;
        if (mFootstepTimer <= 0.0f)
        {
            PlaySound(mFootstepSound);
            mFootstepTimer = 0.25f;
        }
    }
    else
    {
        mFootstepTimer = 0.0f;
    }
}

// Draw every arena layer back-to-front, followed by the protagonist, using the dodge shader when the protagonist is dashing

void Level1::render()
{
    // Clear the screen to the scene's configured background color
    ClearBackground(ColorFromHex(mBGColourHexCode));

    const bool hasPlayer = mGameState.player != nullptr;
    const int isDodging = (hasPlayer && mGameState.player->isDodging()) ? 1 : 0;

    if (mDodgeShader.isLoaded())
    {
        mDodgeShader.setInt("isDodging", isDodging);
        if (hasPlayer)
        {
            mDodgeShader.setVector2("lightPosition",  mGameState.player->getPosition());
            mDodgeShader.setVector2("dodgeOrigin",    mGameState.player->getDodgeOrigin());
            mDodgeShader.setVector2("dodgeDirection", mGameState.player->getDodgeDirection());
            mDodgeShader.setFloat("dodgeProgress",    mGameState.player->getDodgeProgress());
        }

        // Pass 1: Arena world layers (isPlayer = 0 -> focus-drain + shockwave ring)

        mDodgeShader.setInt("isPlayer", 0);
        mDodgeShader.begin();
    }

    mGrassMap->render();      // 1. Grass & cobblestone entry path
    mGameState.map->render(); // 2. Stone courtyard floor
    mDaisMap->render();       // 3. Raised central stone battle platform
    mWallMap->render();       // 4. Enclosing brick & stone walls
    mPropsMap->render();      // 5. Rune altar, pillars, lanterns, and rubble
    mPlantMap->render();      // 6. Outer bushes and grass tufts

    if (mDodgeShader.isLoaded())
    {
        // End shader mode to flush the arena draw batch before switching isPlayer = 1
        mDodgeShader.end();

        if (hasPlayer)
        {
            // Pass 2: Protagonist (isPlayer = 1 -> smear + phase-shift)

            mDodgeShader.setInt("isPlayer", 1);
            mDodgeShader.begin();
            mGameState.player->render();
            mDodgeShader.end();
        }
    }
    else if (hasPlayer)
    {
        mGameState.player->render();
    }
}

// Release all heap and Raylib resources owned by Level1
void Level1::shutdown()
{
    // Unload the GPU dodge shader safely
    mDodgeShader.unload();

    // Delete the protagonist and clear the pointer so repeated shutdown() calls are safe

    delete mGameState.player;
    mGameState.player = nullptr;

    // Delete every Map and clear its pointer

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

    // Unload sound effects and music

    UnloadSound(mAttackSound);
    mAttackSound = {};
    UnloadSound(mDodgeSound);
    mDodgeSound = {};
    UnloadSound(mFootstepSound);
    mFootstepSound = {};

    UnloadMusicStream(mGameState.bgm);
    mGameState.bgm = {};
    // Clear any pending transition before leaving or restarting the scene
    mGameState.nextSceneID = NO_SCENE;
}
