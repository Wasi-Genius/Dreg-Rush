#include "Entity.h"

namespace
{

// Helper to load one Texture2D per Direction into a map after unloading old ones
void loadDirectionalTextureMap(
    std::map<Direction, Texture2D> &destinationMap,
    const std::map<Direction, const char *> &texturePaths
)
{
    for (std::pair<const Direction, Texture2D> &entry : destinationMap)
        UnloadTexture(entry.second);
    destinationMap.clear();

    for (std::map<Direction, const char *>::const_iterator it = texturePaths.begin();
         it != texturePaths.end();
         ++it)
    {
        destinationMap[it->first] = LoadTexture(it->second);
    }
}

// Convert a movement vector into the closest 4-way sprite Direction
Direction directionFromVector(Vector2 vector, Direction fallback)
{
    if (GetLength(vector) <= 0.0f) return fallback;

    if (fabs(vector.x) >= fabs(vector.y))
        return vector.x < 0.0f ? LEFT : RIGHT;

    return vector.y < 0.0f ? UP : DOWN;
}
}

Entity::Entity() :
    mPosition{ 0.0f, 0.0f }, mMovement{ 0.0f, 0.0f },
    mVelocity{ 0.0f, 0.0f }, mAcceleration{ 0.0f, 0.0f },
    mScale{ DEFAULT_SIZE, DEFAULT_SIZE },
    mColliderDimensions{ DEFAULT_SIZE, DEFAULT_SIZE },
    mTexture{}, mTextureType{ SINGLE }, mSpriteSheetDimensions{},
    mAnimationAtlas{}, mAnimationIndices{}, mDirection{ RIGHT },
    mFrameSpeed{ 0 }, mSpeed{ DEFAULT_SPEED }, mAngle{ 0.0f },
    mEntityType{ NONE }, mAIType{ WANDERER }, mAIState{ IDLE } {}

Entity::Entity(
    Vector2     position,
    Vector2     scale,
    const char *textureFilepath,
    EntityType  entityType
) :
    mPosition{ position }, mMovement{ 0.0f, 0.0f },
    mVelocity{ 0.0f, 0.0f }, mAcceleration{ 0.0f, 0.0f },
    mScale{ scale }, mColliderDimensions{ scale },
    mTexture{ LoadTexture(textureFilepath) }, mTextureType{ SINGLE },
    mSpriteSheetDimensions{}, mAnimationAtlas{}, mAnimationIndices{},
    mDirection{ RIGHT }, mFrameSpeed{ 0 }, mSpeed{ DEFAULT_SPEED },
    mAngle{ 0.0f }, mEntityType{ entityType }, mAIType{ WANDERER },
    mAIState{ IDLE } {}

Entity::Entity(
    Vector2                               position,
    Vector2                               scale,
    const char                           *textureFilepath,
    TextureType                           textureType,
    Vector2                               spriteSheetDimensions,
    std::map<Direction, std::vector<int>> animationAtlas,
    EntityType                            entityType
) :
    mPosition{ position }, mMovement{ 0.0f, 0.0f },
    mVelocity{ 0.0f, 0.0f }, mAcceleration{ 0.0f, 0.0f },
    mScale{ scale }, mColliderDimensions{ scale },
    mTexture{ LoadTexture(textureFilepath) }, mTextureType{ textureType },
    mSpriteSheetDimensions{ spriteSheetDimensions },
    mAnimationAtlas{ animationAtlas },
    mAnimationIndices{ animationAtlas.at(RIGHT) }, mDirection{ RIGHT },
    mFrameSpeed{ DEFAULT_FRAME_SPEED }, mSpeed{ DEFAULT_SPEED },
    mAngle{ 0.0f }, mEntityType{ entityType }, mAIType{ WANDERER },
    mAIState{ IDLE } {}

Entity::~Entity()
{
    UnloadTexture(mTexture);
    UnloadTexture(mIdleTexture);
    UnloadTexture(mTurnAroundTexture);

    for (std::pair<const Direction, Texture2D> &entry : mDirectionalRunTextures)
        UnloadTexture(entry.second);
    mDirectionalRunTextures.clear();

    for (std::pair<const Direction, Texture2D> &entry : mDirectionalIdleTextures)
        UnloadTexture(entry.second);
    mDirectionalIdleTextures.clear();

    for (std::pair<const Direction, Texture2D> &entry : mDirectionalAttackTextures)
        UnloadTexture(entry.second);
    mDirectionalAttackTextures.clear();
}

void Entity::setTurnAroundAnimation(
    const char *textureFilepath,
    Vector2 spriteSheetDimensions,
    std::vector<int> animationFrames
)
{
    UnloadTexture(mTurnAroundTexture);
    mTurnAroundTexture = LoadTexture(textureFilepath);
    mTurnAroundSpriteSheetDimensions = spriteSheetDimensions;
    mTurnAroundAnimationFrames = animationFrames;
    mFlipWithDirection = true;
}

void Entity::setIdleAnimation(
    const char *textureFilepath,
    Vector2 spriteSheetDimensions,
    std::vector<int> animationFrames,
    int frameSpeed
)
{
    UnloadTexture(mIdleTexture);
    mIdleTexture = LoadTexture(textureFilepath);
    mIdleSpriteSheetDimensions = spriteSheetDimensions;
    mIdleAnimationFrames = animationFrames;
    mIdleFrameSpeed = frameSpeed > 0 ? frameSpeed : DEFAULT_FRAME_SPEED;
    mIdleAnimationFrame = 0;
    mIdleAnimationTime = 0.0f;
    mIsIdleAnimating = false;
}

void Entity::setDirectionalRunAnimations(
    const std::map<Direction, const char *> &texturePaths,
    Vector2 spriteSheetDimensions,
    const std::vector<int> &animationFrames,
    int frameSpeed
)
{
    loadDirectionalTextureMap(mDirectionalRunTextures, texturePaths);
    mUseDirectionalSheets = true;
    mTextureType = ATLAS;
    mSpriteSheetDimensions = spriteSheetDimensions;
    mAnimationAtlas[UP] = animationFrames;
    mAnimationAtlas[DOWN] = animationFrames;
    mAnimationAtlas[LEFT] = animationFrames;
    mAnimationAtlas[RIGHT] = animationFrames;
    mAnimationIndices = animationFrames;
    mFrameSpeed = frameSpeed > 0 ? frameSpeed : DEFAULT_FRAME_SPEED;
    mCurrentFrameIndex = 0;
    mAnimationTime = 0.0f;
}

void Entity::setDirectionalIdleAnimations(
    const std::map<Direction, const char *> &texturePaths,
    Vector2 spriteSheetDimensions,
    const std::vector<int> &animationFrames,
    int frameSpeed
)
{
    loadDirectionalTextureMap(mDirectionalIdleTextures, texturePaths);
    mIdleSpriteSheetDimensions = spriteSheetDimensions;
    mIdleAnimationFrames = animationFrames;
    mIdleFrameSpeed = frameSpeed > 0 ? frameSpeed : 8;
    mIdleAnimationFrame = 0;
    mIdleAnimationTime = 0.0f;
}

void Entity::setDirectionalAttackAnimations(
    const std::map<Direction, const char *> &texturePaths,
    Vector2 spriteSheetDimensions,
    const std::vector<int> &animationFrames,
    int frameSpeed
)
{
    loadDirectionalTextureMap(mDirectionalAttackTextures, texturePaths);
    mAttackSpriteSheetDimensions = spriteSheetDimensions;
    mAttackAnimationFrames = animationFrames;
    mAttackFrameSpeed = frameSpeed > 0 ? frameSpeed : 28;
    mAttackFrameIndex = 0;
    mAttackAnimationTime = 0.0f;
}

void Entity::setTopDownMovement(Vector2 movement)
{
    // Ignore input while mid-dodge so the player cannot stop or steer a dodge
    if (mIsDodging) return;

    mMovement = movement;
    if (GetLength(mMovement) > 1.0f)
        normaliseMovement();

    // Do not change facing mid-slash so the sword arc stays aligned with the start direction
    if (!mIsAttacking && GetLength(mMovement) > 0.0f)
        setDirection(directionFromVector(mMovement, mDirection));
}

bool Entity::startAttack()
{
    // Lock out attacks while dodging or while a slash is already playing
    if (mIsDodging || mIsAttacking || mAttackAnimationFrames.empty())
        return false;

    mIsAttacking = true;
    mAttackFrameIndex = 0;
    mAttackAnimationTime = 0.0f;
    mAttackSoundPending = true;
    return true;
}

bool Entity::startDodge(Vector2 inputDirection)
{
    // Prevent re-triggering during an active dash or while cooling down
    if (mIsDodging || mDodgeCooldownTimer > 0.0f)
        return false;

    // A dodge can interrupt an attack so combat stays responsive, but once the dodge begins it cannot be canceled until completion
    mIsAttacking = false;
    mAttackFrameIndex = 0;
    mAttackAnimationTime = 0.0f;

    // Dash in the held WASD direction
    // If none is held, dash forward along mDirection
    Vector2 dashDir = inputDirection;
    if (GetLength(dashDir) > 0.0f)
    {
        Normalise(&dashDir);
        setDirection(directionFromVector(dashDir, mDirection));
    }
    else
    {
        switch (mDirection)
        {
        case LEFT:  dashDir = { -1.0f,  0.0f }; break;
        case RIGHT: dashDir = {  1.0f,  0.0f }; break;
        case UP:    dashDir = {  0.0f, -1.0f }; break;
        case DOWN:
        default:    dashDir = {  0.0f,  1.0f }; break;
        }
    }

    mIsDodging = true;
    mDodgeOrigin = mPosition;
    mDodgeDirection = dashDir;
    mDodgeTimer = 0.0f;
    mMovement = dashDir;
    mDodgeSoundPending = true;
    return true;
}

bool Entity::consumeAttackSoundPending()
{
    const bool pending = mAttackSoundPending;
    mAttackSoundPending = false;
    return pending;
}

bool Entity::consumeDodgeSoundPending()
{
    const bool pending = mDodgeSoundPending;
    mDodgeSoundPending = false;
    return pending;
}

bool Entity::isAttackHitActive() const
{
    // In attack1_*.png (8 frames), frames 2..4 are the wide sword-arc frames
    return mIsAttacking && mAttackFrameIndex >= 2 && mAttackFrameIndex <= 4;
}

Rectangle Entity::getAttackHitbox() const
{
    // Project a melee slash box in front of the protagonist's collider
    const Vector2 center = getColliderPosition();
    const float reach = 56.0f;
    const float width = 68.0f;

    switch (mDirection)
    {
    case LEFT:
        return { center.x - reach - 12.0f, center.y - width / 2.0f, reach, width };
    case RIGHT:
        return { center.x + 12.0f, center.y - width / 2.0f, reach, width };
    case UP:
        return { center.x - width / 2.0f, center.y - reach - 12.0f, width, reach };
    case DOWN:
    default:
        return { center.x - width / 2.0f, center.y + 12.0f, width, reach };
    }
}

void Entity::startTurnAround()
{
    if (mTurnAroundAnimationFrames.empty()) return;

    mIsTurningAround = true;
    mTurnAroundFrameIndex = 0;
    mTurnAroundAnimationTime = 0.0f;
}

// Resolve vertical collisions with other entities
void Entity::checkCollisionY(
    Entity *collidableEntities, int collisionCheckCount
)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        // Check each entity in the collision list
        Entity *collidableEntity = &collidableEntities[i];

        if (isColliding(collidableEntity))
        {
            const Vector2 position = getColliderPosition();
            const Vector2 otherPosition =
                collidableEntity->getColliderPosition();

            // Find the vertical overlap between the colliders
            float yDistance = fabs(position.y - otherPosition.y);
            float yOverlap  = fabs(
                yDistance - (mColliderDimensions.y / 2.0f) -
                (collidableEntity->mColliderDimensions.y / 2.0f)
            );

            // Move out of the collision and stop vertical motion
            if (mVelocity.y > 0)
            {
                mPosition.y        -= yOverlap;
                mVelocity.y         = 0;
                mIsCollidingBottom  = true;
            }
            else if (mVelocity.y < 0)
            {
                mPosition.y     += yOverlap;
                mVelocity.y      = 0;
                mIsCollidingTop  = true;
            }
        }
    }
}

void Entity::checkCollisionX(
    Entity *collidableEntities, int collisionCheckCount
)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        Entity *collidableEntity = &collidableEntities[i];

        if (isColliding(collidableEntity))
        {
            const Vector2 position = getColliderPosition();
            const Vector2 otherPosition =
                collidableEntity->getColliderPosition();

            // Ignore small vertical overlap while grazing a corner edge
            float yDistance = fabs(position.y - otherPosition.y);
            float yOverlap  = fabs(
                yDistance - (mColliderDimensions.y / 2.0f) -
                (collidableEntity->mColliderDimensions.y / 2.0f)
            );

            // Skip horizontal resolution when barely touching vertically
            if (yOverlap < Y_COLLISION_THRESHOLD) continue;

            float xDistance = fabs(position.x - otherPosition.x);
            float xOverlap  = fabs(
                xDistance - (mColliderDimensions.x / 2.0f) -
                (collidableEntity->mColliderDimensions.x / 2.0f)
            );

            if (mVelocity.x > 0)
            {
                mPosition.x -= xOverlap;
                mVelocity.x  = 0;

                mIsCollidingRight = true;
            }
            else if (mVelocity.x < 0)
            {
                mPosition.x += xOverlap;
                mVelocity.x  = 0;

                mIsCollidingLeft = true;
            }
        }
    }
}

void Entity::checkCollisionY(Map *map)
{
    if (map == nullptr) return;

    // Inset horizontal corner probes slightly so sliding along a side wall does not catch on wall corners
    const float halfWidth = mColliderDimensions.x / 2.0f;
    const float halfHeight = mColliderDimensions.y / 2.0f;
    const float probeInsetX = halfWidth > 3.0f ? halfWidth - 2.0f : halfWidth;

    const Vector2 position = getColliderPosition();
    Vector2 topCentreProbe    = { position.x,               position.y - halfHeight };
    Vector2 topLeftProbe      = { position.x - probeInsetX, position.y - halfHeight };
    Vector2 topRightProbe     = { position.x + probeInsetX, position.y - halfHeight };

    Vector2 bottomCentreProbe = { position.x,               position.y + halfHeight };
    Vector2 bottomLeftProbe   = { position.x - probeInsetX, position.y + halfHeight };
    Vector2 bottomRightProbe  = { position.x + probeInsetX, position.y + halfHeight };

    float xOverlap = 0.0f;
    float yOverlap = 0.0f;

    // Stop upward motion at a solid tile
    if ((map->isSolidTileAt(topCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(topLeftProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(topRightProbe, &xOverlap, &yOverlap)) &&
        mVelocity.y < 0.0f)
    {
        mPosition.y     += yOverlap; // push down
        mVelocity.y      = 0.0f;
        mIsCollidingTop  = true;
    }

    // Stop downward motion at a solid tile
    if ((map->isSolidTileAt(bottomCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(bottomLeftProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(bottomRightProbe, &xOverlap, &yOverlap)) &&
        mVelocity.y > 0.0f)
    {
        mPosition.y        -= yOverlap; 
        mVelocity.y         = 0.0f;
        mIsCollidingBottom  = true;
    }
}

void Entity::checkCollisionX(Map *map)
{
    if (map == nullptr) return;

    // Vertical corner probes for smooth wall sliding
    const float halfWidth = mColliderDimensions.x / 2.0f;
    const float halfHeight = mColliderDimensions.y / 2.0f;
    const float probeInsetY = halfHeight > 3.0f ? halfHeight - 2.0f : halfHeight;

    const Vector2 position = getColliderPosition();
    Vector2 leftCentreProbe  = { position.x - halfWidth, position.y };
    Vector2 leftTopProbe     = { position.x - halfWidth, position.y - probeInsetY };
    Vector2 leftBottomProbe  = { position.x - halfWidth, position.y + probeInsetY };

    Vector2 rightCentreProbe = { position.x + halfWidth, position.y };
    Vector2 rightTopProbe    = { position.x + halfWidth, position.y - probeInsetY };
    Vector2 rightBottomProbe = { position.x + halfWidth, position.y + probeInsetY };

    float xOverlap = 0.0f;
    float yOverlap = 0.0f;

    // Stop rightward motion at a solid tile
    if ((map->isSolidTileAt(rightCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(rightTopProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(rightBottomProbe, &xOverlap, &yOverlap)) &&
        mVelocity.x > 0.0f && yOverlap >= 0.5f)
    {
        mPosition.x       -= xOverlap * 1.01f;
        mVelocity.x        = 0.0f;
        mIsCollidingRight  = true;
    }

    // Stop leftward motion at a solid tile
    if ((map->isSolidTileAt(leftCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(leftTopProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(leftBottomProbe, &xOverlap, &yOverlap)) &&
        mVelocity.x < 0.0f && yOverlap >= 0.5f)
    {
        mPosition.x      += xOverlap * 1.01f;
        mVelocity.x       = 0.0f;
        mIsCollidingLeft  = true;
    }
}

// Check whether two active entities overlap
bool Entity::isColliding(Entity *other) const
{
    if (!other->isActive() || other == this) return false;

    const Vector2 position = getColliderPosition();
    const Vector2 otherPosition = other->getColliderPosition();
    float xDistance = fabs(position.x - otherPosition.x) -
        ((mColliderDimensions.x + other->getColliderDimensions().x) / 2.0f);
    float yDistance = fabs(position.y - otherPosition.y) -
        ((mColliderDimensions.y + other->getColliderDimensions().y) / 2.0f);

    return xDistance < 0.0f && yDistance < 0.0f;
}

// Advance the current movement animation
void Entity::animate(float deltaTime)
{
    if (!mUseDirectionalSheets && mAnimationAtlas.find(mDirection) != mAnimationAtlas.end())
        mAnimationIndices = mAnimationAtlas.at(mDirection);

    if (mAnimationIndices.empty()) return;

    mAnimationTime += deltaTime;
    const float secondsPerFrame = 1.0f / mFrameSpeed;

    while (mAnimationTime >= secondsPerFrame)
    {
        mAnimationTime -= secondsPerFrame;
        mCurrentFrameIndex = (mCurrentFrameIndex + 1) %
            static_cast<int>(mAnimationIndices.size());
    }
}

void Entity::animateIdle(float deltaTime)
{
    if (mIdleAnimationFrames.empty()) return;

    mIdleAnimationTime += deltaTime;
    const float secondsPerFrame = 1.0f / mIdleFrameSpeed;

    while (mIdleAnimationTime >= secondsPerFrame)
    {
        mIdleAnimationTime -= secondsPerFrame;
        mIdleAnimationFrame = (mIdleAnimationFrame + 1) %
            static_cast<int>(mIdleAnimationFrames.size());
    }
}

void Entity::animateTurnAround(float deltaTime)
{
    mTurnAroundAnimationTime += deltaTime;
    const float secondsPerFrame = 1.0f / mFrameSpeed;

    while (mTurnAroundAnimationTime >= secondsPerFrame && mIsTurningAround)
    {
        mTurnAroundAnimationTime -= secondsPerFrame;
        mTurnAroundFrameIndex++;

        if (mTurnAroundFrameIndex >=
            static_cast<int>(mTurnAroundAnimationFrames.size()))
        {
            mIsTurningAround = false;
            mTurnAroundFrameIndex = 0;
            mTurnAroundAnimationTime = 0.0f;
            mCurrentFrameIndex = 0;
            mAnimationTime = 0.0f;
        }
    }
}

void Entity::animateAttack(float deltaTime)
{
    if (mAttackAnimationFrames.empty())
    {
        mIsAttacking = false;
        return;
    }

    mAttackAnimationTime += deltaTime;
    const float secondsPerFrame = 1.0f / static_cast<float>(mAttackFrameSpeed);

    while (mAttackAnimationTime >= secondsPerFrame && mIsAttacking)
    {
        mAttackAnimationTime -= secondsPerFrame;
        mAttackFrameIndex++;

        if (mAttackFrameIndex >= static_cast<int>(mAttackAnimationFrames.size()))
        {
            mIsAttacking = false;
            mAttackFrameIndex = 0;
            mAttackAnimationTime = 0.0f;
        }
    }
}

void Entity::updateDodge(float deltaTime)
{
    // Tick dodge cooldown while not currently dodging
    if (!mIsDodging)
    {
        if (mDodgeCooldownTimer > 0.0f)
        {
            mDodgeCooldownTimer -= deltaTime;
            if (mDodgeCooldownTimer < 0.0f) mDodgeCooldownTimer = 0.0f;
        }
        return;
    }

    mDodgeTimer += deltaTime;

    // End the dodge once its fixed duration finishes
    if (mDodgeTimer >= mDodgeDuration)
    {
        mIsDodging = false;
        mDodgeTimer = 0.0f;
        mDodgeCooldownTimer = mDodgeCooldown;
        mVelocity = { 0.0f, 0.0f };
        mMovement = { 0.0f, 0.0f };
    }
}

void Entity::displayCollider()
{
    // Draw the entity collision box
    const Vector2 position = getColliderPosition();
    Rectangle colliderBox = {
        position.x - mColliderDimensions.x / 2.0f,
        position.y - mColliderDimensions.y / 2.0f,
        mColliderDimensions.x,
        mColliderDimensions.y
    };

    DrawRectangleLines(
        colliderBox.x,
        colliderBox.y,
        colliderBox.width,
        colliderBox.height,
        GREEN
    );
}

void Entity::AIWander()
{
    if (mIsCollidingLeft)
        moveRight();
    else if (mIsCollidingRight)
        moveLeft();
    else if (mMovement.x == 0.0f)
        moveLeft();
}

void Entity::AIFollow(Entity *target)
{
    if (target == nullptr) return;

    switch (mAIState)
    {
    case IDLE:
        if (Vector2Distance(mPosition, target->getPosition()) < 250.0f)
            mAIState = FOLLOWING;
        break;
    case WALKING:
    case FOLLOWING:
        // Face and move toward the target
        if (mPosition.x > target->getPosition().x)
            moveLeft();
        else
            moveRight();
        break;
    default:
        break;
    }
}

void Entity::AIFly(Entity *target)
{
    if (target == nullptr)
    {
        mMovement = {0.0f, 0.0f};
        return;
    }

    const Vector2 targetPosition = target->getPosition();
    const float differenceX = targetPosition.x - mPosition.x;
    const float differenceY = targetPosition.y - mPosition.y;
    const float distance = GetLength({differenceX, differenceY});

    if (distance == 0.0f)
    {
        mMovement = {0.0f, 0.0f};
        return;
    }

    mMovement = {differenceX / distance, differenceY / distance};

    if (differenceX < 0.0f)
        setDirection(LEFT);
    else if (differenceX > 0.0f)
        setDirection(RIGHT);
}

void Entity::AIActivate(Entity *target)
{
    switch (mAIType)
    {
    case WANDERER:
        AIWander();
        break;
    case FOLLOWER:
        AIFollow(target);
        break;
    case FLYER:
        AIFly(target);
        break;
    default:
        break;
    }
}

void Entity::update(
    float   deltaTime,
    Entity *player,
    Map    *map,
    Entity *collidableEntities,
    int     collisionCheckCount
)
{
    if (mEntityStatus == INACTIVE) return;

    if (mEntityType == NPC) AIActivate(player);

    resetColliderFlags();

    // Update dodge timer before resolving motion
    updateDodge(deltaTime);

    // Flyers ignore gravity and solid map tiles
    if (mAIType == FLYER)
    {
        mVelocity = {
            mMovement.x * mSpeed,
            mMovement.y * mSpeed
        };
        mPosition.x += mVelocity.x * deltaTime;
        mPosition.y += mVelocity.y * deltaTime;
    }
    else if (mIsTopDown)
    {
        // Top-down movement controls both X and Y velocity without gravity
        if (mIsDodging)
        {
            // Ease-out burst speed: fast launch that settles crisply across ~3.4 tiles
            const float progress = mDodgeTimer / mDodgeDuration;
            const float currentSpeed = mDodgePeakSpeed * (1.0f - 0.55f * progress);
            mVelocity = {
                mDodgeDirection.x * currentSpeed,
                mDodgeDirection.y * currentSpeed
            };
        }
        else if (mIsAttacking)
        {
            // Commit briefly to the sword slash so attacks feel punchy
            mVelocity = { 0.0f, 0.0f };
        }
        else
        {
            mVelocity = {
                mMovement.x * mSpeed,
                mMovement.y * mSpeed
            };
        }

        // Resolve Y then X independently so the protagonist slides smoothly along walls
        mPosition.y += mVelocity.y * deltaTime;
        checkCollisionY(collidableEntities, collisionCheckCount);
        checkCollisionY(map);

        mPosition.x += mVelocity.x * deltaTime;
        checkCollisionX(collidableEntities, collisionCheckCount);
        checkCollisionX(map);
    }
    else
    {
        mVelocity.x = mMovement.x * mSpeed;

        mVelocity.x += mAcceleration.x * deltaTime;
        mVelocity.y += mAcceleration.y * deltaTime;

        mPosition.y += mVelocity.y * deltaTime;
        checkCollisionY(collidableEntities, collisionCheckCount);
        checkCollisionY(map);

        mPosition.x += mVelocity.x * deltaTime;
        checkCollisionX(collidableEntities, collisionCheckCount);
        checkCollisionX(map);
    }

    // Idle animation activates when standing still
    const bool groundedForIdle = mIsTopDown ? true : mIsCollidingBottom;
    const bool shouldAnimateIdle =
        mTextureType == ATLAS &&
        !mIdleAnimationFrames.empty() &&
        !mIsAttacking &&
        !mIsDodging &&
        GetLength(mMovement) == 0.0f &&
        groundedForIdle;
    if (shouldAnimateIdle != mIsIdleAnimating)
    {
        mIsIdleAnimating = shouldAnimateIdle;
        mIdleAnimationFrame = 0;
        mIdleAnimationTime = 0.0f;

        if (!mIsIdleAnimating)
        {
            mCurrentFrameIndex = 0;
            mAnimationTime = 0.0f;
        }
    }

    if (mIsAttacking)
        animateAttack(deltaTime);
    else if (mIsDodging)
        mCurrentFrameIndex = 2; 
    else if (mIsTurningAround)
        animateTurnAround(deltaTime);
    else if (mIsIdleAnimating)
        animateIdle(deltaTime);
    else if (mTextureType == ATLAS && GetLength(mMovement) != 0 && groundedForIdle)
        animate(deltaTime);
}

void Entity::render()
{
    if (mEntityStatus == INACTIVE) return;

    Rectangle textureArea{};
    Texture2D texture = mTexture;

    if (mUseDirectionalSheets)
    {
        if (mIsAttacking &&
            mDirectionalAttackTextures.find(mDirection) != mDirectionalAttackTextures.end())
        {
            texture = mDirectionalAttackTextures.at(mDirection);
            textureArea = getUVRectangle(
                &texture,
                mAttackAnimationFrames[mAttackFrameIndex],
                static_cast<int>(mAttackSpriteSheetDimensions.x),
                static_cast<int>(mAttackSpriteSheetDimensions.y)
            );
        }
        else if (mIsIdleAnimating &&
                 mDirectionalIdleTextures.find(mDirection) != mDirectionalIdleTextures.end())
        {
            texture = mDirectionalIdleTextures.at(mDirection);
            textureArea = getUVRectangle(
                &texture,
                mIdleAnimationFrames[mIdleAnimationFrame],
                static_cast<int>(mIdleSpriteSheetDimensions.x),
                static_cast<int>(mIdleSpriteSheetDimensions.y)
            );
        }
        else if (mDirectionalRunTextures.find(mDirection) != mDirectionalRunTextures.end())
        {
            texture = mDirectionalRunTextures.at(mDirection);
            textureArea = getUVRectangle(
                &texture,
                mAnimationIndices[mCurrentFrameIndex],
                static_cast<int>(mSpriteSheetDimensions.x),
                static_cast<int>(mSpriteSheetDimensions.y)
            );
        }
    }
    else if (mIsTurningAround)
    {
        texture = mTurnAroundTexture;
        textureArea = getUVRectangle(
            &texture,
            mTurnAroundAnimationFrames[mTurnAroundFrameIndex],
            mTurnAroundSpriteSheetDimensions.x,
            mTurnAroundSpriteSheetDimensions.y
        );
    }
    else switch (mTextureType)
    {
    case SINGLE:
        // Use the full texture for a single image
        textureArea = {
            0.0f, 0.0f,
            static_cast<float>(mTexture.width),
            static_cast<float>(mTexture.height)
        };
        break;
    case ATLAS:
        if (mIsIdleAnimating)
        {
            texture = mIdleTexture;
            textureArea = getUVRectangle(
                &texture,
                mIdleAnimationFrames[mIdleAnimationFrame],
                mIdleSpriteSheetDimensions.x,
                mIdleSpriteSheetDimensions.y
            );
        }
        else
            textureArea = getUVRectangle(
                &texture,
                mAnimationIndices[mCurrentFrameIndex],
                mSpriteSheetDimensions.x,
                mSpriteSheetDimensions.y
            );
        break;
    default:
        break;
    }

    if (mFlipWithDirection && mDirection == LEFT)
        textureArea.width *= -1.0f;

    // Apply subtle speed stretch and forward lean during a dodge.
    Vector2 drawScale = mScale;
    float drawAngle = mAngle;

    if (mIsDodging)
    {
        if (fabs(mDodgeDirection.x) > fabs(mDodgeDirection.y))
        {
            drawScale = { mScale.x * 1.18f, mScale.y * 0.86f };
            drawAngle = mDodgeDirection.x > 0.0f ? 8.0f : -8.0f;
        }
        else if (fabs(mDodgeDirection.y) > fabs(mDodgeDirection.x))
        {
            drawScale = { mScale.x * 0.88f, mScale.y * 1.15f };
        }
        else
        {
            drawScale = { mScale.x * 1.10f, mScale.y * 0.92f };
            drawAngle = mDodgeDirection.x > 0.0f ? 6.0f : -6.0f;
        }
    }

    // Center the sprite on the entity position
    Rectangle destinationArea = {
        mPosition.x,
        mPosition.y,
        drawScale.x,
        drawScale.y
    };

    // Rotate and flip around the sprite center
    Vector2 originOffset = {
        drawScale.x / 2.0f,
        drawScale.y / 2.0f
    };

    DrawTexturePro(
        texture,
        textureArea, destinationArea, originOffset,
        drawAngle, WHITE
    );
}
