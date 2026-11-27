#ifndef ENTITY_H
#define ENTITY_H

#include "Map.h"

enum Direction    { LEFT, UP, RIGHT, DOWN              }; 
enum EntityStatus { ACTIVE, INACTIVE                   };
enum EntityType   { PLAYER, BLOCK, PLATFORM, NPC, NONE };
enum AIType       { WANDERER, FOLLOWER, FLYER          };
enum AIState      { WALKING, IDLE, FOLLOWING           };

class Entity
{
private:
    // Position and movement values
    Vector2 mPosition;
    Vector2 mMovement;
    Vector2 mVelocity;
    Vector2 mAcceleration;

    Vector2 mScale;
    Vector2 mColliderDimensions;
    Vector2 mColliderOffset{ 0.0f, 0.0f };

    // Sprite textures and animation data
    Texture2D   mTexture{};
    Texture2D   mIdleTexture{};
    Texture2D   mTurnAroundTexture{};
    TextureType mTextureType;
    Vector2     mSpriteSheetDimensions;
    Vector2     mIdleSpriteSheetDimensions{};
    Vector2     mTurnAroundSpriteSheetDimensions{};

    // Directional 4-way sprite sheets used by top-down characters 
    // (e.g. idle_up, idle_down, idle_left, idle_right) 
    // Each Direction maps to its own Texture2D
    std::map<Direction, Texture2D> mDirectionalRunTextures;
    std::map<Direction, Texture2D> mDirectionalIdleTextures;
    std::map<Direction, Texture2D> mDirectionalAttackTextures;
    bool    mUseDirectionalSheets = false;
    Vector2 mAttackSpriteSheetDimensions{};
    std::vector<int> mAttackAnimationFrames;

    // Fast melee swing (~0.28s for 8 frames)
    int     mAttackFrameSpeed = 28; 

    std::map<Direction, std::vector<int>> mAnimationAtlas;
    std::vector<int>                      mAnimationIndices;
    std::vector<int>                      mIdleAnimationFrames;
    std::vector<int>                      mTurnAroundAnimationFrames;
    Direction                             mDirection;
    int                                   mFrameSpeed;

    int   mCurrentFrameIndex = 0;
    int   mIdleAnimationFrame = 0;
    int   mTurnAroundFrameIndex = 0;
    int   mAttackFrameIndex = 0;
    float mAnimationTime     = 0.0f;
    float mIdleAnimationTime = 0.0f;
    float mTurnAroundAnimationTime = 0.0f;
    float mAttackAnimationTime = 0.0f;

    bool  mIsTopDown       = false; 
    bool  mIsIdleAnimating = false;
    bool  mIsTurningAround = false;
    bool  mIsAttacking     = false;
    bool  mFlipWithDirection = false;
    float mJumpingPower = 0.0f;

    // Fast-paced dodge-dash:
    // While mIsDodging is true:
    //   1. The player moves rapidly in the locked mDodgeDirection
    //   2. Player movement input cannot stop or redirect the dash mid-flight
    //   3. Player attacks are locked out until the dash finishes
    //   4. isInvulnerable() returns true so incoming damage is negated
    //   5. Level1 passes dodge state to dodge.fs so the GPU renders the effect

    bool    mIsDodging          = false;
    Vector2 mDodgeOrigin        { 0.0f, 0.0f };
    Vector2 mDodgeDirection     { 0.0f, 0.0f };
    float   mDodgeTimer         = 0.0f;
    float   mDodgeDuration      = 0.18f;  // Quick burst (~0.18s across ~3.4 tiles)
    float   mDodgePeakSpeed     = 1650.0f;
    float   mDodgeCooldownTimer = 0.0f;
    float   mDodgeCooldown      = 0.18f;  // Brief recovery before next dodge

    // Audio flags consumed by the owning Scene so audio stays scene-owned
    bool mAttackSoundPending = false;
    bool mDodgeSoundPending  = false;

    int   mSpeed;
    int   mIdleFrameSpeed = 8;
    float mAngle;

    // Collision state for the current update
    bool mIsCollidingTop    = false;
    bool mIsCollidingBottom = false;
    bool mIsCollidingRight  = false;
    bool mIsCollidingLeft   = false;

    // Entity and AI types
    EntityStatus mEntityStatus = ACTIVE;
    EntityType   mEntityType;

    AIType  mAIType;
    AIState mAIState;

    bool isColliding(Entity *other) const;
    void checkCollisionY(Entity *collidableEntities, int collisionCheckCount);
    void checkCollisionY(Map *map);

    void checkCollisionX(Entity *collidableEntities, int collisionCheckCount);
    void checkCollisionX(Map *map);

    void resetColliderFlags()
    {
        mIsCollidingTop    = false;
        mIsCollidingBottom = false;
        mIsCollidingRight  = false;
        mIsCollidingLeft   = false;
    }

    void animate(float deltaTime);
    void animateIdle(float deltaTime);
    void animateTurnAround(float deltaTime);
    void animateAttack(float deltaTime);
    void updateDodge(float deltaTime);
    void startTurnAround();
    void AIActivate(Entity *target);
    void AIWander();
    void AIFollow(Entity *target);
    void AIFly(Entity *target);

public:
    static constexpr int   DEFAULT_SIZE          = 250;
    static constexpr int   DEFAULT_SPEED         = 200;
    static constexpr int   DEFAULT_FRAME_SPEED   = 14;
    static constexpr float Y_COLLISION_THRESHOLD = 0.5f;

    Entity();
    Entity(
        Vector2     position,
        Vector2     scale,
        const char *textureFilepath,
        EntityType  entityType
    );
    Entity(
        Vector2                               position,
        Vector2                               scale,
        const char                           *textureFilepath,
        TextureType                           textureType,
        Vector2                               spriteSheetDimensions,
        std::map<Direction, std::vector<int>> animationAtlas,
        EntityType                            entityType
    );
    ~Entity();

    void update(
        float   deltaTime,
        Entity *player,
        Map    *map,
        Entity *collidableEntities,
        int     collisionCheckCount
    );
    void render();
    void normaliseMovement() { Normalise(&mMovement); }

    // Top-down combat & dodge triggers (return true if the action started)
    bool startAttack();
    bool startDodge(Vector2 inputDirection);
    void setTopDown(bool isTopDown) { mIsTopDown = isTopDown; }

    // Consume sound requests from the owning update loop
    bool consumeAttackSoundPending();
    bool consumeDodgeSoundPending();

    void activate()   { mEntityStatus = ACTIVE;    }
    void deactivate() { mEntityStatus = INACTIVE;  }
    void displayCollider();

    bool isActive()       const { return mEntityStatus == ACTIVE; }
    bool isDodging()      const { return mIsDodging;              }
    bool isAttacking()    const { return mIsAttacking;            }

    // Damage negation during a dodge: boss/projectile hits check this
    bool isInvulnerable() const { return mIsDodging;              }
    bool isTopDown()      const { return mIsTopDown;              }
    bool collidesWith(Entity *other) const { return isColliding(other); }

    // Dodge state getters uploaded as GLSL uniforms by the active Scene
    Vector2 getDodgeOrigin()    const { return mDodgeOrigin;    }
    Vector2 getDodgeDirection() const { return mDodgeDirection; }
    float   getDodgeProgress()  const
    {
        return mDodgeDuration > 0.0f ? (mDodgeTimer / mDodgeDuration) : 0.0f;
    }

    // Active melee strike window & hitbox for boss damage checks
    bool      isAttackHitActive() const;
    Rectangle getAttackHitbox()   const;

    void moveUp()    { mMovement.y = -1; mDirection = UP;    }
    void moveDown()  { mMovement.y =  1; mDirection = DOWN;  }
    void moveLeft()  { mMovement.x = -1; setDirection(LEFT);  }
    void moveRight() { mMovement.x =  1; setDirection(RIGHT); }

    // Update horizontal/vertical input independently while facing the axis
    void setTopDownMovement(Vector2 movement);

    void resetMovement() { mMovement = { 0.0f, 0.0f }; }

    Vector2     getPosition()              const { return mPosition;              }
    Vector2     getMovement()              const { return mMovement;              }
    Vector2     getVelocity()              const { return mVelocity;              }
    Vector2     getAcceleration()          const { return mAcceleration;          }
    Vector2     getScale()                 const { return mScale;                 }
    Vector2     getColliderDimensions()    const { return mColliderDimensions;    }
    Vector2     getColliderPosition()      const
    {
        return {
            mPosition.x + (mDirection == LEFT
                ? -mColliderOffset.x
                : mColliderOffset.x),
            mPosition.y + mColliderOffset.y
        };
    }
    Vector2     getSpriteSheetDimensions() const { return mSpriteSheetDimensions; }
    Texture2D   getTexture()               const { return mTexture;               }
    TextureType getTextureType()           const { return mTextureType;           }
    Direction   getDirection()             const { return mDirection;             }
    int         getFrameSpeed()            const { return mFrameSpeed;            }
    float       getJumpingPower()          const { return mJumpingPower;          }
    int         getSpeed()                 const { return mSpeed;                 }
    float       getAngle()                 const { return mAngle;                 }
    EntityType  getEntityType()            const { return mEntityType;            }
    AIType      getAIType()                const { return mAIType;                }
    AIState     getAIState()               const { return mAIState;               }

    bool isCollidingTop()    const { return mIsCollidingTop;    }
    bool isCollidingBottom() const { return mIsCollidingBottom; }

    std::map<Direction, std::vector<int>> getAnimationAtlas() const { return mAnimationAtlas; }

    void setPosition(Vector2 newPosition)
        { mPosition = newPosition;                 }
    void setMovement(Vector2 newMovement)
        { mMovement = newMovement;                 }
    void setVelocity(Vector2 newVelocity)
        { mVelocity = newVelocity;                 }
    void setAcceleration(Vector2 newAcceleration)
        { mAcceleration = newAcceleration;         }
    void setScale(Vector2 newScale)
        { mScale = newScale;                       }
    void setTexture(const char *textureFilepath)
        { mTexture = LoadTexture(textureFilepath); }
    void setTurnAroundAnimation(
        const char *textureFilepath,
        Vector2 spriteSheetDimensions,
        std::vector<int> animationFrames
    );
    void setIdleAnimation(
        const char *textureFilepath,
        Vector2 spriteSheetDimensions,
        std::vector<int> animationFrames,
        int frameSpeed = 8
    );

    // Load separate UP/DOWN/LEFT/RIGHT sprite sheets
    void setDirectionalRunAnimations(
        const std::map<Direction, const char *> &texturePaths,
        Vector2 spriteSheetDimensions,
        const std::vector<int> &animationFrames,
        int frameSpeed = 14
    );
    void setDirectionalIdleAnimations(
        const std::map<Direction, const char *> &texturePaths,
        Vector2 spriteSheetDimensions,
        const std::vector<int> &animationFrames,
        int frameSpeed = 8
    );
    void setDirectionalAttackAnimations(
        const std::map<Direction, const char *> &texturePaths,
        Vector2 spriteSheetDimensions,
        const std::vector<int> &animationFrames,
        int frameSpeed = 28
    );

    void setColliderDimensions(Vector2 newDimensions)
        { mColliderDimensions = newDimensions;     }
    void setColliderOffset(Vector2 newOffset)
        { mColliderOffset = newOffset;             }
    void setSpriteSheetDimensions(Vector2 newDimensions)
        { mSpriteSheetDimensions = newDimensions;  }
    void setSpeed(int newSpeed)
        { mSpeed = newSpeed;                       }
    void setFrameSpeed(int newSpeed)
        { mFrameSpeed = newSpeed;                  }
    void setJumpingPower(float newJumpingPower)
        { mJumpingPower = newJumpingPower;         }
    void setAngle(float newAngle)
        { mAngle = newAngle;                       }
    void setEntityType(EntityType entityType)
        { mEntityType = entityType;                }
    void setAIState(AIState newState)
        { mAIState = newState;                     }
    void setAIType(AIType newType)
        { mAIType = newType;                       }
    void setDirection(Direction newDirection)
    {
        if (mFlipWithDirection &&
            ((mDirection == LEFT && newDirection == RIGHT) ||
             (mDirection == RIGHT && newDirection == LEFT)))
            startTurnAround();

        mDirection = newDirection;

        if (mTextureType == ATLAS && !mUseDirectionalSheets &&
            mAnimationAtlas.find(mDirection) != mAnimationAtlas.end())
        {
            mAnimationIndices = mAnimationAtlas.at(mDirection);
        }
    }
};

#endif // ENTITY_H
