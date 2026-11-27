/**

* Author: Wasi Genius

* Assignment: Dreg Rush

* Date due: [Your presentation date]

* I pledge that I have completed this assignment without

* collaborating with anyone else, in conformance with the

* NYU School of Engineering Policies and Procedures on

* Academic Misconduct.

**/


#include "CS3113/Level1.h"
#include "CS3113/LoseScene.h"
#include "CS3113/MenuScene.h"
#include "CS3113/WinScene.h"

constexpr int SCREEN_WIDTH = 1200,
              SCREEN_HEIGHT = 800,
              FPS = 120;

constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

constexpr Vector2 ORIGIN = {
    SCREEN_WIDTH / 2.0f,
    SCREEN_HEIGHT / 2.0f};

AppStatus gAppStatus = RUNNING;
float gPreviousTicks = 0.0f;
float gTimeAccumulator = 0.0f;
Camera2D gCamera = {};

// The scene map owns every scene while this pointer selects the active one
Scene *gCurrentScene = nullptr;
std::map<SceneID, Scene *> gLevels;

// Keep the camera centered on fixed-size boss arenas and clamped on larger maps
Vector2 getBoundedCameraTarget(Scene *scene)
{
    if (scene == nullptr) return ORIGIN;

    const Vector2 origin = scene->getOrigin();
    Map *map = scene->getState().map;
    if (map == nullptr) return origin;

    const float mapWidth = map->getRightBoundary() - map->getLeftBoundary();
    // Single-screen arenas (like Level 1's 1216 px arena in a 1200 px window)
    // stay centered on the scene origin so the walls never slide off-screen
    if (mapWidth <= SCREEN_WIDTH + 64.0f) return origin;

    Entity *player = scene->getState().player;
    if (player == nullptr) return origin;

    float targetX = player->getPosition().x;
    const float minX = map->getLeftBoundary() + ORIGIN.x;
    const float maxX = map->getRightBoundary() - ORIGIN.x;
    if (targetX < minX) targetX = minX;
    if (targetX > maxX) targetX = maxX;

    return { targetX, origin.y };
}

// Shut down the old scene before loading the new one
void switchToScene(Scene *scene)
{
    if (scene == nullptr)
    {
        LOG("Cannot switch to a scene that has not been created.");
        gAppStatus = TERMINATED;
        return;
    }

    if (gCurrentScene != nullptr)
        gCurrentScene->shutdown();

    gCurrentScene = scene;
    gCurrentScene->initialise();
    gCamera.target = getBoundedCameraTarget(gCurrentScene);
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Dreg Rush");
    InitAudioDevice();

    gGameProgress.hurtSound = LoadSound("assets/Misc. Sound/Hurt Sound.mp3");

    // Store each scene under its named ID
    gLevels[MAIN_MENU] = new MenuScene(ORIGIN, "#1b4998");
    // Create the first boss arena using the shared 1200 by 800 origin
    gLevels[LEVEL_1] = new Level1(ORIGIN, "#C0897E");
    gLevels[LOSE_SCREEN] = new LoseScene(ORIGIN, "#A00000");
    gLevels[WIN_SCREEN] = new WinScene(ORIGIN, "#008000");
    switchToScene(gLevels[MAIN_MENU]);

    // Keep the player centered horizontally in the camera
    gCamera.offset = ORIGIN;
    gCamera.rotation = 0.0f;
    gCamera.zoom = 1.0f;
    SetTargetFPS(FPS);

    // Start the frame timer after the first scene loads
    gPreviousTicks = static_cast<float>(GetTime());
}

void processInput()
{
    if (IsKeyPressed(KEY_Q) || WindowShouldClose())
    {
        gAppStatus = TERMINATED;
        return;
    }

    Entity *player = gCurrentScene->getState().player;

    if (player != nullptr)
    {
        // Number keys switch levels for debugging

        SceneID selectedLevel = NO_SCENE;
        if (IsKeyPressed(KEY_ONE))
            // Pressing 1 directly selects the Level 1 boss arena
            selectedLevel = LEVEL_1;
       
        if (selectedLevel != NO_SCENE)
        {
            switchToScene(gLevels[selectedLevel]);
            return;
        }

        if (player->isTopDown())
        {
            // Read horizontal and vertical axes independently so diagonal WASD movement works
            Vector2 movementInput = { 0.0f, 0.0f };
            if (IsKeyDown(KEY_A))      movementInput.x -= 1.0f;
            if (IsKeyDown(KEY_D))      movementInput.x += 1.0f;
            if (IsKeyDown(KEY_W))      movementInput.y -= 1.0f;
            if (IsKeyDown(KEY_S))      movementInput.y += 1.0f;

            // Update movement only when not mid-dodge (setTopDownMovement guards mIsDodging)
            player->setTopDownMovement(movementInput);

            // SPACE triggers the fast-paced phantom dodge; J triggers melee attack
            // (startAttack rejects input automatically while dodging)
            if (IsKeyPressed(KEY_SPACE))
                player->startDodge(movementInput);
            else if (IsKeyPressed(KEY_J))
                player->startAttack();
        }
        else
        {
            player->resetMovement();

            if (IsKeyDown(KEY_A))
                player->moveLeft();
            else if (IsKeyDown(KEY_D))
                player->moveRight();
            else if (IsKeyDown(KEY_S))
                player->moveDown();
            else if (IsKeyDown(KEY_W))
                player->moveUp();

            // Normalize diagonal input to keep its speed consistent
            if (GetLength(player->getMovement()) > 1.0f)
                player->normaliseMovement();
        }
    }
    else if (IsKeyPressed(KEY_ENTER))
    {
        gGameProgress.lives = 5;
        gCurrentScene->requestSceneChange(LEVEL_1);
    }
}

void update()
{
    // Carry leftover time forward so physics uses a fixed update interval
    const float ticks = static_cast<float>(GetTime());
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;
    deltaTime += gTimeAccumulator;

    while (deltaTime >= FIXED_TIMESTEP)
    {
        gCurrentScene->update(FIXED_TIMESTEP);
        deltaTime -= FIXED_TIMESTEP;

        gCamera.target = getBoundedCameraTarget(gCurrentScene);
    }

    gTimeAccumulator = deltaTime;
}

void render()
{
    BeginDrawing();
    BeginMode2D(gCamera);
    gCurrentScene->render();
    EndMode2D();
    EndDrawing();
}

void shutdown()
{
    // Delete each scene through the shared base class
    for (std::pair<const SceneID, Scene *> &entry : gLevels)
        delete entry.second;
    gLevels.clear();
    gCurrentScene = nullptr;

    UnloadSound(gGameProgress.hurtSound);
    gGameProgress.hurtSound = {};
    CloseAudioDevice();
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();

        // Handle scene changes after the current update finishes
        const SceneID nextSceneID = gCurrentScene->getState().nextSceneID;
        if (nextSceneID != NO_SCENE)
        {
            std::map<SceneID, Scene *>::iterator nextScene =
                gLevels.find(nextSceneID);

            if (nextScene == gLevels.end())
            {
                LOG("Cannot switch: the requested scene ID does not exist.");
                gAppStatus = TERMINATED;
            }
            else
            {
                switchToScene(nextScene->second);
            }
        }

        render();
    }

    shutdown();
    return 0;
}
