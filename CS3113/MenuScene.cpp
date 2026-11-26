#include "MenuScene.h"

MenuScene::MenuScene(Vector2 origin, const char *bgColourHexCode)
: Scene{ origin, bgColourHexCode } {}

MenuScene::~MenuScene()
{
    shutdown();
}

void MenuScene::initialise()
{
    // Start menu music
    mGameState.bgm = LoadMusicStream("assets/Music/Intro Conext Music.mp3");
    SetMusicVolume(mGameState.bgm, 0.5f);
    PlayMusicStream(mGameState.bgm);
    mGameState.nextSceneID = NO_SCENE;
}

void MenuScene::update(float)
{
    // Keep music playing
    UpdateMusicStream(mGameState.bgm);
}

void MenuScene::render()
{
    // Show the game title and start prompt
    ClearBackground(ColorFromHex(mBGColourHexCode));
    DrawText("Dreg Rush", 190, 220, 120, WHITE);
    DrawText("Press ENTER to start", 190, 360, 60, WHITE);
    DrawText("Use 'WASD' keys to move", 190, 450, 30, WHITE);
    DrawText("Use 'SPACE' key to doge", 190, 490, 30, WHITE);
    DrawText("Use 'J' key to attack", 190, 530, 30, WHITE);
    DrawText("Use 'K' key to pick up/use potion", 190, 570, 30, WHITE);

}

void MenuScene::shutdown()
{
    UnloadMusicStream(mGameState.bgm);
    mGameState.bgm = {};
    mGameState.nextSceneID = NO_SCENE;
}
