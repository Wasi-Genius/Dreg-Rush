#ifndef LEVEL_1_H
#define LEVEL_1_H

#include "Scene.h"

// Level 1 owns its map, assets, and exit logic
class LevelA : public Scene
{
private:
    // Teal Rock (melee boss) animation states
    enum TealRockAnimation
    {

    };

    static constexpr int LEVEL_1_MAP_COLUMNS = 84,
                         LEVEL_1_MAP_ROWS = 18;
    static constexpr float LEVEL_1_TILE_SIZE = 32.0f;

    unsigned int mLevelData[LEVEL_1_MAP_COLUMNS * LEVEL_1_MAP_ROWS] = {};

    // Level and character textures
  
    // Enemies and level positions
  

public:
    LevelA();
    LevelA(Vector2 origin, const char *bgColourHexCode);
    ~LevelA() override;

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;

private:
    void buildLevelData();
    void renderHuntress();
    void renderHealthBar();
    void loseLife();
};

#endif 