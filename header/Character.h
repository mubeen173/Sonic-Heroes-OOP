#pragma once
#include "Animation.h"
#include "JumpQueue.h"
#include "PositionQueue.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
using namespace sf;
using namespace std;

const int SCREEN_X = 1200;
const int SCREEN_Y = 900;
const int CELL_SIZE = 64;

class Character {
public:
    static const int Idle = 0;
    static const int Running = 1;
    static const int Jumping = 2;
    static const int Pushing = 3;
    static const int Edging = 4;
    static const int Punching = 5;
    static const int Flying = 6;
    static const int StateCount = 7;

    Character(float x, float y, int width, int height, float baseMaxSpeed, float scale = 1.0f)
        : sprite(), posX(x), posY(y), velX(0.0f), velY(0.0f), onGround(false), scale(scale), facingRight(false),
        baseMaxSpeed(baseMaxSpeed), currentMaxSpeed(baseMaxSpeed), followerMoveTime(0.0f), isFollowerBoosted(false),
        justJumped(false), jumpedWhileStill(false), jumpedWhileStillThisFrame(false), jumpDelayTimer(0.0f),
        currentState(Idle), isCollidingLeft(false), isCollidingRight(false)
    {
        sprite.setScale(scale, scale);
        sprite.setPosition(x, y);
        this->width = width * scale;
        this->height = height * scale;
        for (int i = 0; i < StateCount; ++i) {
            leftAnimations[i] = nullptr;
            rightAnimations[i] = nullptr;
        }
        sprite.setPosition(x, y);
    }


    virtual ~Character() {
        for (int i = 0; i < StateCount; ++i) {
            delete leftAnimations[i];
            delete rightAnimations[i];
        }
    }

    float getPosX() const { return posX; }
    float getPosY() const { return posY; }
    float getBaseMaxSpeed() const { return baseMaxSpeed; }
    int getCurrentState() const { return currentState; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    bool isOnGround() const { return onGround; }
    float getVelX() const { return velX; }
    float getVelY() const { return velY; }

    bool isFacingRight() const { return facingRight; }

    void setOnGround(bool value) { onGround = value; }
    void setPosX(float value) { posX = value; }
    void setPosY(float value) { posY = value; }
    void setVelX(float value) { velX = value; }
    void setVelY(float value) { velY = value; }

    virtual void update(float gravity, float terminalVelocity, float jumpStrength,
        const char** level, int rows, int cols, float deltaTime)
    {
        justJumped = false;
        jumpedWhileStill = false;
        isCollidingLeft = false;
        isCollidingRight = false;

        bool isMovingLeft = Keyboard::isKeyPressed(Keyboard::Left);
        bool isMovingRight = Keyboard::isKeyPressed(Keyboard::Right);
        int currentDirection = (isMovingRight ? 1 : (isMovingLeft ? -1 : 0));

        if (!onGround) {
            currentState = Jumping;
        }
        else if (currentDirection != 0) {
            currentState = Running;
        }
        else {
            currentState = Idle;
        }

        if (velX > 0) facingRight = true;
        else if (velX < 0) facingRight = false;

        if (Keyboard::isKeyPressed(Keyboard::Up) && onGround) {
            velY = jumpStrength;
            onGround = false;
            justJumped = true;
            if (velX == 0) {
                jumpedWhileStill = true;
                jumpedWhileStillThisFrame = true;
            }
        }

        if (currentDirection == 0) velX = 0.0f;
        else velX = currentDirection * currentMaxSpeed;

        posX += velX;
        applyHorizontalCollision(level, rows, cols);

        if (isCollidingLeft && isMovingLeft && onGround) {
            currentState = Pushing;
            facingRight = false;
        }
        else if (isCollidingRight && isMovingRight && onGround) {
            currentState = Pushing;
            facingRight = true;
        }

        if (currentState == Idle && onGround) {
            int botRow = static_cast<int>((posY + height) / CELL_SIZE);
            float centerX = posX + width / 2.0f;
            int centerCol = static_cast<int>(centerX / CELL_SIZE);

            if (centerCol >= 0 && centerCol < cols && botRow >= 0 && botRow < rows) {
                char currentBlock = level[botRow][centerCol];
                if (currentBlock == 'f' || currentBlock == 'p') {
                    bool hasLeftNeighbor = (centerCol > 0) &&
                        (level[botRow][centerCol - 1] == 'f' || level[botRow][centerCol - 1] == 'p');
                    bool hasRightNeighbor = (centerCol < cols - 1) &&
                        (level[botRow][centerCol + 1] == 'f' || level[botRow][centerCol + 1] == 'p');

                    float blockLeftX = centerCol * CELL_SIZE;
                    float blockRightX = (centerCol + 1) * CELL_SIZE;
                    float playerLeftX = posX;
                    float playerRightX = posX + width;

                    if (!hasLeftNeighbor && (playerLeftX - blockLeftX < 10) && (playerLeftX < blockRightX)) {
                        currentState = Edging;
                        facingRight = false;
                    }
                    else if (!hasRightNeighbor && (blockRightX - playerRightX < 10) && (playerRightX > blockLeftX)) {
                        currentState = Edging;
                        facingRight = true;
                    }
                }
            }
        }

        float groundCheckOffset = 32.0f * scale;
        float checkLeftX = posX + groundCheckOffset;
        float checkRightX = posX + width - groundCheckOffset;

        onGround = false;
        float bottomY = posY + height;
        int row = static_cast<int>(bottomY / CELL_SIZE);
        if (row < rows) {
            int colLeft = static_cast<int>(checkLeftX / CELL_SIZE);
            if (colLeft >= 0 && colLeft < cols && (level[row][colLeft] == 'f' || level[row][colLeft] == 'p' || level[row][colLeft] == 'b' || level[row][colLeft] == 'N')) {
                onGround = true;
            }
            int colRight = static_cast<int>(checkRightX / CELL_SIZE);
            if (!onGround && colRight >= 0 && colRight < cols && (level[row][colRight] == 'f' || level[row][colRight] == 'p' || level[row][colRight] == 'b'||level[row][colRight] == 'N')) {
                onGround = true;
            }
        }

        if (!onGround) velY = (velY + gravity < terminalVelocity ? velY + gravity : terminalVelocity);
        else if (!justJumped) velY = 0.0f;

        posY += velY;
        applyVerticalCollision(level, rows, cols);

        Animation* currentAnim = (facingRight && currentState != Jumping && currentState != Edging) ?
            rightAnimations[currentState] : leftAnimations[currentState];
        if (currentState == Edging) {
            currentAnim = facingRight ? rightAnimations[Edging] : leftAnimations[Edging];
        }
        if (currentAnim) {
            currentAnim->update(deltaTime);
            sprite.setTexture(*currentAnim->getTexture());
            sprite.setTextureRect(currentAnim->getCurrentFrame());
        }

        sprite.setPosition(posX, posY);
    }

    virtual void updateFollower(float gravity, float terminalVelocity, float jumpStrength,
        const char** level, int rows, int cols, float deltaTime,
        float targetX, float targetY, JumpQueue& jumpQueue)
    {
        jumpDelayTimer -= deltaTime;
        justJumped = false;
        isCollidingLeft = false;
        isCollidingRight = false;

        float dx = targetX - posX;
        float targetVelocityX = 0.0f;

        if (abs(dx) > 15.0f) {
            followerMoveTime += deltaTime;
            if (followerMoveTime >= 2.0f) isFollowerBoosted = true;
            float speed = baseMaxSpeed * (isFollowerBoosted ? 1.2f : 1.0f);
            targetVelocityX = (dx > 0) ? speed : -speed;
        }
        else {
            followerMoveTime = 0.0f;
            isFollowerBoosted = false;
        }

        if (abs(posX - lastPosX) < 1.0f && onGround) {
            stuckTimer += deltaTime;
        }
        else {
            stuckTimer = 0.0f;
            lastPosX = posX;
        }

        if (stuckTimer >= 3.0f) {
            float cameraX = targetX - SCREEN_X / 2.0f;
            posX = cameraX - 200.0f;
            posY = targetY;
            velX = 0.0f;
            velY = 0.0f;
            onGround = true;
            stuckTimer = 0.0f;
            lastPosX = posX;
        }

        velX += (targetVelocityX - velX) * 0.1f;

        if (!onGround) currentState = Jumping;
        else if (abs(velX) > 0.1f) currentState = Running;
        else currentState = Idle;

        if (targetVelocityX > 0) facingRight = true;
        else if (targetVelocityX < 0) facingRight = false;

        int leftCol = int((posX + 8 * scale) / CELL_SIZE);
        int rightCol = int((posX + width - 8 * scale) / CELL_SIZE);
        int topRow = int((posY + 5 * scale) / CELL_SIZE);
        int botRow = int((posY + height - 5 * scale) / CELL_SIZE);

        if (velX > 0 && dx > 0) {
            bool shouldJump = false;
            for (int offset = 1; offset <= 2; ++offset) {
                int aheadCol = rightCol + offset;
                if (aheadCol < cols) {
                    for (int y = topRow; y <= botRow; ++y) {
                        if (level[y][aheadCol] == 'w' || level[y][aheadCol] == 'b'|| level[y][aheadCol] == 'N') {
                            if (onGround && targetX > (aheadCol * CELL_SIZE)) {
                                shouldJump = true;
                                break;
                            }
                        }
                    }
                    if (botRow + 1 < rows && level[botRow + 1][aheadCol] == '.' && onGround) {
                        if (targetX > (aheadCol * CELL_SIZE)) {
                            shouldJump = true;
                            break;
                        }
                    }
                }
            }
            if (shouldJump) {
                velY = jumpStrength * 1.1f; // Increased jump strength for follower
                onGround = false;
                justJumped = true;
                jumpDelayTimer = 0.2f;
            }
        }
        else if (velX < 0 && dx < 0) {
            bool shouldJump = false;
            for (int offset = 1; offset <= 2; ++offset) {
                int aheadCol = leftCol - offset;
                if (aheadCol >= 0) {
                    for (int y = topRow; y <= botRow; ++y) {
                        if (level[y][aheadCol] == 'w' || level[y][aheadCol] == 'b'||level[y][aheadCol] == 'b') {
                            if (onGround && targetX < (aheadCol * CELL_SIZE)) {
                                shouldJump = true;
                                break;
                            }
                        }
                    }
                    if (botRow + 1 < rows && level[botRow + 1][aheadCol] == '.' && onGround) {
                        if (targetX < (aheadCol* CELL_SIZE)) {
                            shouldJump = true;
                            break;
                        }
                    }
                }
            }
            if (shouldJump) {
                velY = jumpStrength * 1.1f; // Increased jump strength for follower
                onGround = false;
                justJumped = true;
                jumpDelayTimer = 0.2f;
            }
        }

        if (!jumpQueue.isEmpty()) {
            float jumpX = jumpQueue.peek();
            float tolerance = 2.0f;
            bool movingRight = velX > 0;
            bool movingLeft = velX < 0;
            bool atJumpPosition = false;

            if (movingRight && posX >= jumpX - tolerance) atJumpPosition = true;
            else if (movingLeft && posX <= jumpX + tolerance) atJumpPosition = true;
            else if (abs(posX - jumpX) < tolerance) atJumpPosition = true;

            if (atJumpPosition && onGround) {
                velY = jumpStrength * 1.1f; // Increased jump strength for follower
                onGround = false;
                justJumped = true;
                jumpQueue.dequeue();
            }
        }

        posX += velX;
        applyHorizontalCollision(level, rows, cols);

        if (isCollidingLeft && targetVelocityX < 0 && onGround) {
            currentState = Pushing;
            facingRight = false;
        }
        else if (isCollidingRight && targetVelocityX > 0 && onGround) {
            currentState = Pushing;
            facingRight = true;
        }

        if (currentState == Idle && onGround) {
            int botRow = static_cast<int>((posY + height) / CELL_SIZE);
            float centerX = posX + width / 2.0f;
            int centerCol = static_cast<int>(centerX / CELL_SIZE);

            if (centerCol >= 0 && centerCol < cols && botRow >= 0 && botRow < rows) {
                char currentBlock = level[botRow][centerCol];
                if (currentBlock == 'f' || currentBlock == 'p') {
                    bool hasLeftNeighbor = (centerCol > 0) &&
                        (level[botRow][centerCol - 1] == 'f' || level[botRow][centerCol - 1] == 'p');
                    bool hasRightNeighbor = (centerCol < cols - 1) &&
                        (level[botRow][centerCol + 1] == 'f' || level[botRow][centerCol + 1] == 'p');

                    float blockLeftX = centerCol * CELL_SIZE;
                    float blockRightX = (centerCol + 1) * CELL_SIZE;
                    float playerLeftX = posX;
                    float playerRightX = posX + width;

                    if (!hasLeftNeighbor || !hasRightNeighbor) {
                        currentState = Edging;
                    }
                }
            }
        }

        float groundCheckOffset = 32.0f * scale;
        float checkLeftX = posX + groundCheckOffset;
        float checkRightX = posX + width - groundCheckOffset;

        onGround = false;
        float bottomY = posY + height;
        int row = static_cast<int>(bottomY / CELL_SIZE);
        if (row < rows) {
            int colLeft = static_cast<int>(checkLeftX / CELL_SIZE);
            if (colLeft >= 0 && colLeft < cols && (level[row][colLeft] == 'f' || level[row][colLeft] == 'p')) {
                onGround = true;
            }
            int colRight = static_cast<int>(checkRightX / CELL_SIZE);
            if (!onGround && colRight >= 0 && colRight < cols && (level[row][colRight] == 'f' || level[row][colRight] == 'p')) {
                onGround = true;
            }
        }

        if (!onGround) velY = (velY + gravity < terminalVelocity ? velY + gravity : terminalVelocity);
        else if (!justJumped) velY = 0.0f;

        posY += velY;
        applyVerticalCollision(level, rows, cols);

        Animation* currentAnim = (facingRight && currentState != Jumping && currentState != Edging) ?
            rightAnimations[currentState] : leftAnimations[currentState];
        if (currentState == Edging) {
            currentAnim = facingRight ? rightAnimations[Edging] : leftAnimations[Edging];
        }
        if (currentAnim) {
            currentAnim->update(deltaTime);
            sprite.setTexture(*currentAnim->getTexture());
            sprite.setTextureRect(currentAnim->getCurrentFrame());
        }

        sprite.setPosition(posX, posY);
    }

    virtual void draw(RenderWindow& window, const RenderStates& states = RenderStates::Default) {
        window.draw(sprite, states);
    }

    bool justJumped;
    bool jumpedWhileStill;
    bool jumpedWhileStillThisFrame;
    float jumpDelayTimer;
    float currentMaxSpeed;


protected:
    
   
    Sprite sprite;
    float posX, posY;
    float velX, velY;
    bool onGround;
    float lastGroundX, lastGroundY;
    float scale;
    float width, height;
    bool facingRight;
    float baseMaxSpeed;
    float followerMoveTime;
    bool isFollowerBoosted;
    Animation* leftAnimations[StateCount];
    Animation* rightAnimations[StateCount];
    int currentState;
    bool isCollidingLeft;
    bool isCollidingRight;
    float stuckTimer;
    float lastPosX;



    virtual bool isBlockSolid(char c) const {
        return c == 'w' || c == 'b' || c == 'l' || c == 'g' || c == 'N'; // Added 'g' as solid
    }

    // ...existing code...
virtual void applyHorizontalCollision(const char** level, int rows, int cols) {
    if (level == nullptr) return;
    int leftCol = int((posX + 8 * scale) / CELL_SIZE);
    int rightCol = int((posX + width - 8 * scale) / CELL_SIZE);
    int topRow = int((posY + 5 * scale) / CELL_SIZE);
    int botRow = int((posY + height - 5 * scale) / CELL_SIZE);

    // Left collision
    if (velX < 0 && leftCol >= 0) {
        for (int y = topRow; y <= botRow; ++y) {
            if (y >= 0 && y < rows && leftCol >= 0 && leftCol < cols) {
                char c = level[y][leftCol];
                if (isBlockSolid(c)) {
                    posX = (leftCol + 1) * CELL_SIZE - 8 * scale;
                    velX = 0;
                    isCollidingLeft = true;
                    break;
                }
            }
        }
    }

    // Right collision
    if (velX > 0 && rightCol < cols) {
        for (int y = topRow; y <= botRow; ++y) {
            if (y >= 0 && y < rows && rightCol >= 0 && rightCol < cols) {
                char c = level[y][rightCol];
                if (isBlockSolid(c)) {
                    posX = rightCol * CELL_SIZE - width + 8 * scale;
                    velX = 0;
                    isCollidingRight = true;
                    break;
                }
            }
        }
    }
}


    void applyVerticalCollision(const char** level, int rows, int cols) {
        if (level == nullptr) return;
        int leftCol = int((posX + 8 * scale) / CELL_SIZE);
        int rightCol = int((posX + width - 8 * scale) / CELL_SIZE);
        int botRow = static_cast<int>((posY + height) / CELL_SIZE);

        if (velY > 0 && botRow < rows && !justJumped) {
            bool anyFloor = false;
            for (int x = leftCol; x <= rightCol; ++x) {
                if (x >= 0 && x < cols && botRow < rows) {
                    char c = level[botRow][x];
                    if (c == 'f' || c == 'p' || c == 'b' || c == 'l' || c == 'g') {
                        anyFloor = true;
                        break;
                    }
                }
            }
            if (anyFloor) {
                posY = (botRow * CELL_SIZE) - height;
                velY = 0;
                onGround = true;
            }
            else onGround = false;
        }
        else if (velY < 0) {
            int topRow = static_cast<int>(posY / CELL_SIZE);
            if (topRow >= 0 && topRow < rows && leftCol >= 0 && rightCol < cols) {
                bool hitCeiling = false;
                for (int x = leftCol; x <= rightCol; ++x) {
                    if (x >= 0 && x < cols) {
                        char c = level[topRow][x];
                        if (c == 'r' || c == 'b' || c == 'l' || c == 'g') {
                            hitCeiling = true;
                            break;
                        }
                    }
                }
                if (hitCeiling) {
                    posY = (topRow + 1) * CELL_SIZE;
                    velY = 0;
                }
            }
        }
    }
    void setEdgingAnimation() {
        currentState = Edging;
        if (facingRight) {
            rightAnimations[Edging]->reset();
        }
        else {
            leftAnimations[Edging]->reset();
        }
    }

    bool isOnEdge(const char** level, int rows, int cols)  {
        int botRow = static_cast<int>((posY + height) / CELL_SIZE);
        float centerX = posX + width / 2.0f;
        int centerCol = static_cast<int>(centerX / CELL_SIZE);

        if (centerCol >= 0 && centerCol < cols && botRow >= 0 && botRow < rows) {
            char currentBlock = level[botRow][centerCol];
            if (currentBlock == 'f' || currentBlock == 'b' || currentBlock == 'p') {
                bool hasLeftNeighbor = (centerCol > 0) &&
                    (level[botRow][centerCol - 1] == 'f' || level[botRow][centerCol - 1] == 'b' || level[botRow][centerCol - 1] == 'p');
                bool hasRightNeighbor = (centerCol < cols - 1) &&
                    (level[botRow][centerCol + 1] == 'f' || level[botRow][centerCol + 1] == 'b' || level[botRow][centerCol + 1] == 'p');
                if (!hasLeftNeighbor) {
                    facingRight = false;
                    return true;
                }
                if (!hasRightNeighbor) {
                    facingRight = true;
                    return true;
                }
            }
        }
        return false;
    }
};



class Knuckles : public Character {
public:
    struct Coord {
        int x, y;
    };
    static const int MAX_BLOCKS_TO_BREAK = 10; // Maximum number of blocks that can be queued to break
    Coord blocksToBreak[MAX_BLOCKS_TO_BREAK];
    int numBlocksToBreak;
    Knuckles(float x, float y, int width, int height, float baseMaxSpeed, float scale,
         Texture& idleLeft,  Texture& idleRight,  Texture& runLeft,  Texture& runRight,
         Texture& jumpLeft,  Texture& jumpRight,  Texture& pushLeft,  Texture& pushRight,
         Texture& edgeLeft,  Texture& edgeRight,  Texture& punchLeft,  Texture& punchRight)
        : Character(x, y, width, height, baseMaxSpeed, scale),
        moveTime(0.0f), previousDirection(0), isBoosted(false), stuckTimer(0.0f), lastPosX(x), punchTimer(0.0f),
        numBlocksToBreak(0)
    {
        const int FRAME_WIDTH = 40;
        const int FRAME_HEIGHT = 40;
        const float FRAME_DURATION = 0.05f; // Default for most animations
        const float IDLE_FRAME_DURATION = 0.2f; // Slower for idle animation (4 frames, 0.8s cycle)
        const float PUNCH_FRAME_DURATION = 0.1f; // For punch animation

        // Idle animation (assuming 4 frames in sprite sheet)
        int idleLeftFrames = idleLeft.getSize().x / FRAME_WIDTH;
        leftAnimations[Idle] = new Animation(&idleLeft, FRAME_WIDTH, FRAME_HEIGHT, idleLeftFrames, IDLE_FRAME_DURATION);
        int idleRightFrames = idleRight.getSize().x / FRAME_WIDTH;
        rightAnimations[Idle] = new Animation(&idleRight, FRAME_WIDTH, FRAME_HEIGHT, idleRightFrames, IDLE_FRAME_DURATION);

        // Other animations
        int runLeftFrames = runLeft.getSize().x / FRAME_WIDTH;
        leftAnimations[Running] = new Animation(&runLeft, FRAME_WIDTH, FRAME_HEIGHT, runLeftFrames, FRAME_DURATION);
        int runRightFrames = runRight.getSize().x / FRAME_WIDTH;
        rightAnimations[Running] = new Animation(&runRight, FRAME_WIDTH, FRAME_HEIGHT, runRightFrames, FRAME_DURATION);

        int jumpLeftFrames = jumpLeft.getSize().x / FRAME_WIDTH;
        leftAnimations[Jumping] = new Animation(&jumpLeft, FRAME_WIDTH, FRAME_HEIGHT, jumpLeftFrames, FRAME_DURATION);
        int jumpRightFrames = jumpRight.getSize().x / FRAME_WIDTH;
        rightAnimations[Jumping] = new Animation(&jumpRight, FRAME_WIDTH, FRAME_HEIGHT, jumpRightFrames, FRAME_DURATION);

        int pushLeftFrames = pushLeft.getSize().x / FRAME_WIDTH;
        leftAnimations[Pushing] = new Animation(&pushLeft, FRAME_WIDTH, FRAME_HEIGHT, pushLeftFrames, FRAME_DURATION);
        int pushRightFrames = pushRight.getSize().x / FRAME_WIDTH;
        rightAnimations[Pushing] = new Animation(&pushRight, FRAME_WIDTH, FRAME_HEIGHT, pushRightFrames, FRAME_DURATION);

        int edgeLeftFrames = edgeLeft.getSize().x / FRAME_WIDTH;
        leftAnimations[Edging] = new Animation(&edgeLeft, FRAME_WIDTH, FRAME_HEIGHT, edgeLeftFrames, FRAME_DURATION);
        int edgeRightFrames = edgeRight.getSize().x / FRAME_WIDTH;
        rightAnimations[Edging] = new Animation(&edgeRight, FRAME_WIDTH, FRAME_HEIGHT, edgeRightFrames, FRAME_DURATION);

        int punchLeftFrames = punchLeft.getSize().x / FRAME_WIDTH;
        leftAnimations[Punching] = new Animation(&punchLeft, FRAME_WIDTH, FRAME_HEIGHT, punchLeftFrames, PUNCH_FRAME_DURATION);
        int punchRightFrames = punchRight.getSize().x / FRAME_WIDTH;
        rightAnimations[Punching] = new Animation(&punchRight, FRAME_WIDTH, FRAME_HEIGHT, punchRightFrames, PUNCH_FRAME_DURATION);

        sprite.setTexture(*rightAnimations[Idle]->getTexture());
        sprite.setTextureRect(rightAnimations[Idle]->getCurrentFrame());

        for (int i = 0; i < MAX_BLOCKS_TO_BREAK; ++i) {
            blocksToBreak[i].x = 0;
            blocksToBreak[i].y = 0;
        }
    }


    void update(float gravity, float terminalVelocity, float jumpStrength,
        const char** level, int rows, int cols, float deltaTime) override
    {
        justJumped = false;
        jumpedWhileStill = false;
        isCollidingLeft = false;
        isCollidingRight = false;

        int previousState = currentState;

        if (currentState == Punching) {
            punchTimer -= deltaTime;
            if (punchTimer <= 0) {
                if (!onGround) currentState = Jumping;
                else if (abs(velX) > 0.1f) currentState = Running;
                else currentState = Idle;
            }
        }

        bool isMovingLeft =  Keyboard::isKeyPressed( Keyboard::Left);
        bool isMovingRight =  Keyboard::isKeyPressed( Keyboard::Right);
        int currentDirection = (isMovingRight ? 1 : (isMovingLeft ? -1 : 0));

        if (currentDirection != 0) {
            if (currentDirection != previousDirection) {
                moveTime = 0.0f;
                isBoosted = false;
                previousDirection = currentDirection;
            }
            else {
                moveTime += deltaTime;
                if (moveTime >= 2.0f) isBoosted = true;
            }
        }
        else {
            moveTime = 0.0f;
            isBoosted = false;
            previousDirection = 0;
        }

        float boostMultiplier = isBoosted ? 1.5f : 1.0f;
        float targetVelocityX = currentDirection * currentMaxSpeed * boostMultiplier;

        velX += (targetVelocityX - velX) * 0.1f;

        if (!onGround) currentState = Jumping;
        else if (currentDirection != 0 && currentState != Punching) currentState = Running;
        else if (currentState != Punching) currentState = Idle;

        if (velX > 0) facingRight = true;
        else if (velX < 0) facingRight = false;

        if ( Keyboard::isKeyPressed( Keyboard::Up) && onGround) {
            velY = jumpStrength;
            onGround = false;
            justJumped = true;
            if (abs(velX) < 0.1f) {
                jumpedWhileStill = true;
                jumpedWhileStillThisFrame = true;
            }
            leftAnimations[Jumping]->reset();
            rightAnimations[Jumping]->reset();
            if (currentState == Punching) {
                currentState = Jumping;
                punchTimer = 0.0f;
            }
        }

        posX += velX;
        applyHorizontalCollision(level, rows, cols);

        if (velX != 0 && currentState != Punching) {
            int checkCol = velX > 0 ? static_cast<int>((posX + width) / CELL_SIZE) : static_cast<int>(posX / CELL_SIZE);
            int topRow = static_cast<int>(posY / CELL_SIZE);
            int botRow = static_cast<int>((posY + height) / CELL_SIZE);
            for (int y = topRow; y <= botRow; ++y) {
                if (y >= 0 && y < rows && checkCol >= 0 && checkCol < cols && level[y][checkCol] == 'l') {
                    if (numBlocksToBreak < MAX_BLOCKS_TO_BREAK) {
                        blocksToBreak[numBlocksToBreak].x = checkCol;
                        blocksToBreak[numBlocksToBreak].y = y;
                        numBlocksToBreak++;
                        currentState = Punching;
                        punchTimer = 0.3f;
                        leftAnimations[Punching]->reset();
                        rightAnimations[Punching]->reset();
                        velX = 0.0f; // Set velocity to 0 on collision with 'l'
                    }
                    break;
                }
            }
        }

        if (currentState != Punching) {
            if (isCollidingLeft && isMovingLeft && onGround) {
                currentState = Pushing;
                facingRight = false;
                leftAnimations[Pushing]->reset();
            }
            else if (isCollidingRight && isMovingRight && onGround) {
                currentState = Pushing;
                facingRight = true;
                rightAnimations[Pushing]->reset();
            }
            else if (currentState == Idle && onGround && isOnEdge(level, rows, cols)) {
                setEdgingAnimation();
            }
        }

        if (currentState == Idle && onGround && isOnEdge(level, rows, cols)) {
            setEdgingAnimation();
        }

        float groundCheckOffset = 32.0f * scale;
        float checkLeftX = posX + groundCheckOffset;
        float checkRightX = posX + width - groundCheckOffset;

        onGround = false;
        float bottomY = posY + height;
        int row = static_cast<int>(bottomY / CELL_SIZE);
        if (row < rows) {
            int colLeft = static_cast<int>(checkLeftX / CELL_SIZE);
            if (colLeft >= 0 && colLeft < cols && (level[row][colLeft] == 'f' || level[row][colLeft] == 'p' || level[row][colLeft] == 'b')) {
                onGround = true;
            }
            int colRight = static_cast<int>(checkRightX / CELL_SIZE);
            if (!onGround && colRight >= 0 && colRight < cols && (level[row][colRight] == 'f' || level[row][colRight] == 'p' || level[row][colRight] == 'b')) {
                onGround = true;
            }
        }

        if (!onGround) velY = (velY + gravity < terminalVelocity ? velY + gravity : terminalVelocity);
        else if (!justJumped) velY = 0.0f;

        posY += velY;
        applyVerticalCollision(level, rows, cols);

        Animation* currentAnim = nullptr;
        if (currentState == Jumping) {
            currentAnim = facingRight ? rightAnimations[Jumping] : leftAnimations[Jumping];
        }
        else if (currentState == Edging) {
            currentAnim = facingRight ? rightAnimations[Edging] : leftAnimations[Edging];
        }
        else if (currentState == Punching) {
            currentAnim = facingRight ? rightAnimations[Punching] : leftAnimations[Punching];
        }
        else {
            currentAnim = facingRight ? rightAnimations[currentState] : leftAnimations[currentState];
        }
        if (currentAnim) {
            currentAnim->update(deltaTime);
            sprite.setTexture(*currentAnim->getTexture());
            sprite.setTextureRect(currentAnim->getCurrentFrame());
        }
        sprite.setPosition(posX, posY);
    }


    bool isBlockSolid(char c) const override {
        return c == 'w' || c == 'b' || c == 'g' || c == 'N'; // 'l' is not solid, but 'g' is
    }

    void updateFollower(float gravity, float terminalVelocity, float jumpStrength,
        const char** level, int rows, int cols, float deltaTime,
        float targetX, float targetY, JumpQueue& jumpQueue) override
    {
        jumpDelayTimer -= deltaTime;
        justJumped = false;
        isCollidingLeft = false;
        isCollidingRight = false;

        float dx = targetX - posX;
        float targetVelocityX = 0.0f;

        if (abs(dx) > 15.0f) {
            followerMoveTime += deltaTime;
            if (followerMoveTime >= 2.0f) isFollowerBoosted = true;
            float speed = baseMaxSpeed * (isFollowerBoosted ? 1.2f : 1.0f);
            targetVelocityX = (dx > 0) ? speed : -speed;
            stuckTimer = 0.0f;
        }
        else {
            followerMoveTime = 0.0f;
            isFollowerBoosted = false;
        }

        if (abs(posX - lastPosX) < 1.0f && onGround) {
            stuckTimer += deltaTime;
        }
        else {
            stuckTimer = 0.0f;
            lastPosX = posX;
        }

        velX += (targetVelocityX - velX) * 0.5f;
        if (isOnEdge(level, rows, cols)) {
			if (currentState != Edging) {
				leftAnimations[Edging]->reset();
				rightAnimations[Edging]->reset();
			}
			currentState = Edging;
		}
        if (!onGround) {
            if (currentState != Jumping) {
                leftAnimations[Jumping]->reset();
                rightAnimations[Jumping]->reset();
            }
            currentState = Jumping;
        }
        else if (abs(velX) > 0.1f) {
            if (currentState != Running) {
                leftAnimations[Running]->reset();
                rightAnimations[Running]->reset();
            }
            currentState = Running;
        }
        else {
            if (currentState != Idle) {
                leftAnimations[Idle]->reset();
                rightAnimations[Idle]->reset();
            }
            currentState = Idle;
        }

        if (targetVelocityX > 0) facingRight = true;
        else if (targetVelocityX < 0) facingRight = false;

        int leftCol = int((posX + 8 * scale) / CELL_SIZE);
        int rightCol = int((posX + width - 8 * scale) / CELL_SIZE);
        int topRow = int((posY + 5 * scale) / CELL_SIZE);
        int botRow = static_cast<int>((posY + height) / CELL_SIZE);

        int direction = (velX > 0 && dx > 0) ? 1 : (velX < 0 && dx < 0) ? -1 : 0;

        if (direction != 0 && onGround) {
            bool shouldJump = false;
            int startCol = (direction == 1) ? rightCol : leftCol;

            for (int offset = 1; offset <= 2 && !shouldJump; ++offset) {
                int aheadCol = startCol + offset * direction;
                if (aheadCol < 0 || aheadCol >= cols) break;
                for (int y = topRow; y <= botRow; ++y) {
                    char t = level[y][aheadCol];
                    if (t == 'w' || t == 'b' || t == 'l' || t == 'N') {
                        int obsX = aheadCol * CELL_SIZE;
                        bool beyond = (direction == 1) ? (targetX > obsX) : (targetX < obsX);
                        if (beyond) shouldJump = true;
                        break;
                    }
                }
            }

            int detectedGap = 0;
            if (!shouldJump) {
                detectedGap = detectGap(level, rows, cols, botRow, startCol, direction, targetX,targetY);
                if (detectedGap > 0) {
                     cout << "Gap detected: " << detectedGap <<  endl;
                    shouldJump = true;
                }
            }

            if (shouldJump) {
                float jumpMultiplier = 1.0f + detectedGap * 1.5f;
                velY = jumpStrength * jumpMultiplier; // Increased jump strength for follower
                velX = targetVelocityX * 1.5f;
                onGround = false;
                justJumped = true;
                jumpDelayTimer = 0.0f;
                leftAnimations[Jumping]->reset();
                rightAnimations[Jumping]->reset();
            }
        }

        if (!jumpQueue.isEmpty()) {
            float jumpX = jumpQueue.peek();
            float tolerance = 2.0f;
            bool atPos = false;

            if (velX > 0 && posX >= jumpX - tolerance) atPos = true;
            else if (velX < 0 && posX <= jumpX + tolerance) atPos = true;
            else if (fabs(posX - jumpX) < tolerance) atPos = true;

            if (atPos && onGround) {
                velY = jumpStrength * 1.1f; // Increased jump strength for follower
                onGround = false;
                justJumped = true;
                jumpQueue.dequeue();
                leftAnimations[Jumping]->reset();
                rightAnimations[Jumping]->reset();
            }
        }

        if (stuckTimer >= 3.0f && abs(targetX - posX) > 200.0f) {
            float cameraX = targetX - SCREEN_X / 2.0f;
            posX = cameraX - 300.0f;
            posY = targetY;
            velX = 0.0f;
            velY = 0.0f;
            onGround = true;
            stuckTimer = 0.0f;
            lastPosX = posX;
        }

        posX += velX;
        applyHorizontalCollision(level, rows, cols);

        if (velX != 0 && currentState != Punching) {
            int checkCol = velX > 0 ? rightCol : leftCol;
            for (int y = topRow; y <= botRow; ++y) {
                if (y >= 0 && y < rows && checkCol >= 0 && checkCol < cols && level[y][checkCol] == 'l') {
                    if (numBlocksToBreak < MAX_BLOCKS_TO_BREAK) {
                        blocksToBreak[numBlocksToBreak].x = checkCol;
                        blocksToBreak[numBlocksToBreak].y = y;
                        numBlocksToBreak++;
                        currentState = Punching;
                        punchTimer = 0.3f;
                        leftAnimations[Punching]->reset();
                        rightAnimations[Punching]->reset();
                        velX = 0.0f; // Set velocity to 0 on collision with 'l'
                    }
                    break;
                }
            }
        }

        if (isCollidingLeft && targetVelocityX < 0 && onGround) {
            currentState = Pushing;
            facingRight = false;
            leftAnimations[Pushing]->reset();
        }
        else if (isCollidingRight && targetVelocityX > 0 && onGround) {
            currentState = Pushing;
            facingRight = true;
            rightAnimations[Pushing]->reset();
        }

        if ( isOnEdge(level, rows, cols)) {
            setEdgingAnimation();
        }

        float groundCheckOffset = 32.0f * scale;
        float checkLeftX = posX + groundCheckOffset;
        float checkRightX = posX + width - groundCheckOffset;

        onGround = false;
        float bottomY = posY + height;
        int row = static_cast<int>(bottomY / CELL_SIZE);
        if (row < rows) {
            int colLeft = static_cast<int>(checkLeftX / CELL_SIZE);
            if (colLeft >= 0 && colLeft < cols && (level[row][colLeft] == 'f' || level[row][colLeft] == 'p' || level[row][colLeft] == 'b' || level[row][colLeft] == 'l')) {
                onGround = true;
            }
            int colRight = static_cast<int>(checkRightX / CELL_SIZE);
            if (!onGround && colRight >= 0 && colRight < cols && (level[row][colRight] == 'f' || level[row][colRight] == 'p' || level[row][colRight] == 'b' || level[row][colRight] == 'l')) {
                onGround = true;
            }
        }

        if (!onGround) velY = (velY + gravity < terminalVelocity ? velY + gravity : terminalVelocity);
        else if (!justJumped) velY = 0.0f;

        posY += velY;
        applyVerticalCollision(level, rows, cols);

        Animation* currentAnim = (facingRight && currentState != Jumping && currentState != Edging && currentState != Punching) ?
            rightAnimations[currentState] : leftAnimations[currentState];
        if (currentState == Edging) {
            currentAnim = facingRight ? rightAnimations[Edging] : leftAnimations[Edging];
        }
        else if (currentState == Punching) {
            currentAnim = facingRight ? rightAnimations[Punching] : leftAnimations[Punching];
        }
        if (currentAnim) {
            currentAnim->update(deltaTime);
            sprite.setTexture(*currentAnim->getTexture());
            sprite.setTextureRect(currentAnim->getCurrentFrame());
        }

        sprite.setPosition(posX, posY);
    }

private:
    float moveTime;
    int previousDirection;
    bool isBoosted;
    float stuckTimer;
    float lastPosX;
    float punchTimer;


    int detectGap(const char** level, int rows, int cols, int botRow, int startCol, int direction, float targetX, float targetY) {
        if (botRow >= rows) return 0;

        int maxCheck = 6; // Extended range to 6 blocks
        int gapWidth = 0;
        int targetRow = static_cast<int>(targetY / CELL_SIZE);

        for (int offset = 1; offset <= maxCheck; ++offset) {
            int checkCol = startCol + offset * direction;
            if (checkCol < 0 || checkCol >= cols) break;

            // Check ground level
            if (botRow < rows && level[botRow][checkCol] == ' ') {
                gapWidth++;
            }
            else {
                // Found landing spot, check height difference
                int landRow = botRow;
                while (landRow < rows && level[landRow][checkCol] == ' ') landRow++;
                if (landRow < rows && (level[landRow][checkCol] == 'f' || level[landRow][checkCol] == 'b' || level[landRow][checkCol] == 'p' || level[landRow][checkCol] == 'l')) {
                    float landX = checkCol * CELL_SIZE;
                    bool beyond = (direction == 1) ? (targetX > landX) : (targetX < landX);
                    int heightDiff = abs(botRow - landRow) * CELL_SIZE;
                    if (beyond && heightDiff <= CELL_SIZE * 3) { // Max jump height of 3 blocks
                        return gapWidth;
                        cout << "GAP";
                    }
                }
                break;
            }
        }
        return gapWidth > 0 ? gapWidth : 0;
    }
};




class Sonic : public Character {
public:
    Sonic(float x, float y, int width, int height, float baseMaxSpeed, float scale,
        Texture& idleLeft, Texture& idleRight, Texture& runLeft, Texture& runRight,
        Texture& jump, Texture& pushLeft, Texture& pushRight, Texture& edgeLeft, Texture& edgeRight)
        : Character(x, y, width, height, baseMaxSpeed, scale),
        moveTime(0.0f), previousDirection(0), isBoosted(false)
    {
        const int FRAME_WIDTH = 40;
        const int FRAME_HEIGHT = 40;
        const float FRAME_DURATION = 0.05f;

        int idleLeftFrames = idleLeft.getSize().x / FRAME_WIDTH;
        leftAnimations[Idle] = new Animation(&idleLeft, FRAME_WIDTH, FRAME_HEIGHT, idleLeftFrames, FRAME_DURATION);
        int idleRightFrames = idleRight.getSize().x / FRAME_WIDTH;
        rightAnimations[Idle] = new Animation(&idleRight, FRAME_WIDTH, FRAME_HEIGHT, idleRightFrames, FRAME_DURATION);

        int runLeftFrames = runLeft.getSize().x / FRAME_WIDTH;
        leftAnimations[Running] = new Animation(&runLeft, FRAME_WIDTH, FRAME_HEIGHT, runLeftFrames, FRAME_DURATION);
        int runRightFrames = runRight.getSize().x / FRAME_WIDTH;
        rightAnimations[Running] = new Animation(&runRight, FRAME_WIDTH, FRAME_HEIGHT, runRightFrames, FRAME_DURATION);

        int jumpFrames = jump.getSize().x / FRAME_WIDTH;
        leftAnimations[Jumping] = new Animation(&jump, FRAME_WIDTH, FRAME_HEIGHT, jumpFrames, FRAME_DURATION);
        rightAnimations[Jumping] = new Animation(&jump, FRAME_WIDTH, FRAME_HEIGHT, jumpFrames, FRAME_DURATION);

        int pushLeftFrames = pushLeft.getSize().x / FRAME_WIDTH;
        leftAnimations[Pushing] = new Animation(&pushLeft, FRAME_WIDTH, FRAME_HEIGHT, pushLeftFrames, FRAME_DURATION);
        int pushRightFrames = pushRight.getSize().x / FRAME_WIDTH;
        rightAnimations[Pushing] = new Animation(&pushRight, FRAME_WIDTH, FRAME_HEIGHT, pushRightFrames, FRAME_DURATION);

       int edgeLeftFrames = edgeLeft.getSize().x / FRAME_WIDTH;
    leftAnimations[Edging] = new Animation(&edgeLeft, FRAME_WIDTH, FRAME_HEIGHT, edgeLeftFrames, FRAME_DURATION);
    int edgeRightFrames = edgeRight.getSize().x / FRAME_WIDTH;
    rightAnimations[Edging] = new Animation(&edgeRight, FRAME_WIDTH, FRAME_HEIGHT, edgeRightFrames, FRAME_DURATION);

        sprite.setTexture(*rightAnimations[Idle]->getTexture());
        sprite.setTextureRect(rightAnimations[Idle]->getCurrentFrame());
    }

    void update(float gravity, float terminalVelocity, float jumpStrength,
        const char** level, int rows, int cols, float deltaTime) override
    {
        justJumped = false;
        jumpedWhileStill = false;
        isCollidingLeft = false;
        isCollidingRight = false;

        bool isMovingLeft = Keyboard::isKeyPressed(Keyboard::Left);
        bool isMovingRight = Keyboard::isKeyPressed(Keyboard::Right);
        int currentDirection = (isMovingRight ? 1 : (isMovingLeft ? -1 : 0));

        if (currentDirection != 0) {
            if (currentDirection != previousDirection) {
                moveTime = 0.0f;
                isBoosted = false;
                previousDirection = currentDirection;
            }
            else {
                moveTime += deltaTime;
                if (moveTime >= 3.0f) isBoosted = true;
            }
        }
        else {
            moveTime = 0.0f;
            isBoosted = false;
            previousDirection = 0;
        }

        float boostMultiplier = isBoosted ? 1.3f : 1.0f;
        float targetVelocityX = currentDirection * currentMaxSpeed * boostMultiplier;

        velX += (targetVelocityX - velX) * 0.1f;

        if (!onGround) {
            if (currentState != Jumping) {
                leftAnimations[Jumping]->reset();
                rightAnimations[Jumping]->reset();
            }
            currentState = Jumping;
        }
        else if (currentDirection != 0) {
            if (currentState != Running) {
                leftAnimations[Running]->reset();
                rightAnimations[Running]->reset();
            }
            currentState = Running;
        }
        else if (currentState == Idle && onGround && isOnEdge(level, rows, cols)) {
            setEdgingAnimation();
            
        }
        else {
            if (currentState != Idle) {
                leftAnimations[Idle]->reset();
                rightAnimations[Idle]->reset();
            }
            currentState = Idle;
        }

        if (velX > 0) facingRight = true;
        else if (velX < 0) facingRight = false;

        if (Keyboard::isKeyPressed(Keyboard::Up) && onGround) {
            velY = jumpStrength;
            onGround = false;
            justJumped = true;
            if (abs(velX) < 0.1f) {
                jumpedWhileStill = true;
                jumpedWhileStillThisFrame = true;
            }
            leftAnimations[Jumping]->reset();
            rightAnimations[Jumping]->reset();
        }

        if (currentState == Idle && isOnEdge(level, rows, cols)) {
            setEdgingAnimation();
        }

        posX += velX;
        applyHorizontalCollision(level, rows, cols);

        if (isCollidingLeft && isMovingLeft && onGround) {
            currentState = Pushing;
            facingRight = false;
            leftAnimations[Pushing]->reset();
        }
        else if (isCollidingRight && isMovingRight && onGround) {
            currentState = Pushing;
            facingRight = true;
            rightAnimations[Pushing]->reset();
        }

        float groundCheckOffset = 32.0f * scale;
        float checkLeftX = posX + groundCheckOffset;
        float checkRightX = posX + width - groundCheckOffset;

        onGround = false;
        float bottomY = posY + height;
        int row = static_cast<int>(bottomY / CELL_SIZE);
        if (row < rows) {
            int colLeft = static_cast<int>(checkLeftX / CELL_SIZE);
            if (colLeft >= 0 && colLeft < cols && (level[row][colLeft] == 'f' || level[row][colLeft] == 'p' || level[row][colLeft] == 'b')) {
                onGround = true;
            }
            int colRight = static_cast<int>(checkRightX / CELL_SIZE);
            if (!onGround && colRight >= 0 && colRight < cols && (level[row][colRight] == 'f' || level[row][colRight] == 'p' || level[row][colRight] == 'b')) {
                onGround = true;
            }
        }

        if (!onGround) velY = (velY + gravity < terminalVelocity ? velY + gravity : terminalVelocity);
        else if (!justJumped) velY = 0.0f;

        posY += velY;
        applyVerticalCollision(level, rows, cols);

        Animation* currentAnim = (facingRight && currentState != Jumping && currentState != Edging) ?
    rightAnimations[currentState] : leftAnimations[currentState];
if (currentState == Edging) {
    currentAnim = facingRight ? rightAnimations[Edging] : leftAnimations[Edging];
}
if (currentAnim) {
    currentAnim->update(deltaTime);
    sprite.setTexture(*currentAnim->getTexture());
    sprite.setTextureRect(currentAnim->getCurrentFrame());
}
        sprite.setPosition(posX, posY);
    }


    void updateFollower(float gravity, float terminalVelocity, float jumpStrength,
        const char** level, int rows, int cols, float deltaTime,
        float targetX, float targetY, JumpQueue& jumpQueue) override
    {
        jumpDelayTimer -= deltaTime;
        justJumped = false;
        isCollidingLeft = false;
        isCollidingRight = false;

        float dx = targetX - posX;
        float targetVelocityX = 0.0f;

        if (abs(dx) > 15.0f) {
            followerMoveTime += deltaTime;
            if (followerMoveTime >= 2.0f) isFollowerBoosted = true;
            float speed = baseMaxSpeed * (isFollowerBoosted ? 1.2f : 1.0f);
            targetVelocityX = (dx > 0) ? speed : -speed;
        }
        else {
            followerMoveTime = 0.0f;
            isFollowerBoosted = false;
        }

        velX += (targetVelocityX - velX) * 0.1f;
        if (isOnEdge(level, rows, cols)) {
			setEdgingAnimation();
        }
        if (!onGround) {
            if (currentState != Jumping) {
                leftAnimations[Jumping]->reset();
                rightAnimations[Jumping]->reset();
            }
            currentState = Jumping;
        }
        else if (abs(velX) > 0.1f) {
            if (currentState != Running) {
                leftAnimations[Running]->reset();
                rightAnimations[Running]->reset();
            }
            currentState = Running;
        }
        else {
            if (currentState != Idle) {
                leftAnimations[Idle]->reset();
                rightAnimations[Idle]->reset();
            }
            currentState = Idle;
        }

        if (targetVelocityX > 0) facingRight = true;
        else if (targetVelocityX < 0) facingRight = false;

        int leftCol = int((posX + 8 * scale) / CELL_SIZE);
        int rightCol = int((posX + width - 8 * scale) / CELL_SIZE);
        int topRow = int((posY + 5 * scale) / CELL_SIZE);
        int botRow = int((posY + height - 5 * scale) / CELL_SIZE);

        if (velX > 0 && dx > 0) {
            bool shouldJump = false;
            for (int offset = 1; offset <= 2; ++offset) {
                int aheadCol = rightCol + offset;
                if (aheadCol < cols) {
                    for (int y = topRow; y <= botRow; ++y) {
                        if (level[y][aheadCol] == 'w' || level[y][aheadCol] == 'b') {
                            if (onGround && targetX > (aheadCol * CELL_SIZE)) {
                                shouldJump = true;
                                break;
                            }
                        }
                    }
                    if (botRow + 1 < rows && level[botRow + 1][aheadCol] == '.' && onGround) {
                        if (targetX > (aheadCol * CELL_SIZE)) {
                            shouldJump = true;
                            break;
                        }
                    }
                }
            }
            if (shouldJump) {
                velY = jumpStrength * 1.1f; // Increased jump strength for follower
                onGround = false;
                justJumped = true;
                jumpDelayTimer = 0.2f;
                leftAnimations[Jumping]->reset();
                rightAnimations[Jumping]->reset();
            }
        }
        else if (velX < 0 && dx < 0) {
            bool shouldJump = false;
            for (int offset = 1; offset <= 2; ++offset) {
                int aheadCol = leftCol - offset;
                if (aheadCol >= 0) {
                    for (int y = topRow; y <= botRow; ++y) {
                        if (level[y][aheadCol] == 'w' || level[y][aheadCol] == 'b'||level[y][aheadCol] == 'N') {
                            if (onGround && targetX < (aheadCol * CELL_SIZE)) {
                                shouldJump = true;
                                break;
                            }
                        }
                    }
                    if (botRow + 1 < rows && level[botRow + 1][aheadCol] == '.' && onGround) {
                        if (targetX < (aheadCol* CELL_SIZE)) {
                            shouldJump = true;
                            break;
                        }
                    }
                }
            }
            if (shouldJump) {
                velY = jumpStrength * 1.1f; // Increased jump strength for follower
                onGround = false;
                justJumped = true;
                jumpDelayTimer = 0.2f;
                leftAnimations[Jumping]->reset();
                rightAnimations[Jumping]->reset();
            }
        }

        if (!jumpQueue.isEmpty()) {
            float jumpX = jumpQueue.peek();
            float tolerance = 2.0f;
            bool movingRight = velX > 0;
            bool movingLeft = velX < 0;
            bool atJumpPosition = false;

            if (movingRight && posX >= jumpX - tolerance) atJumpPosition = true;
            else if (movingLeft && posX <= jumpX + tolerance) atJumpPosition = true;
            else if (abs(posX - jumpX) < tolerance) atJumpPosition = true;

            if (atJumpPosition && onGround) {
                velY = jumpStrength * 1.1f; // Increased jump strength for follower
                onGround = false;
                justJumped = true;
                jumpQueue.dequeue();
                leftAnimations[Jumping]->reset();
                rightAnimations[Jumping]->reset();
            }
        }

        posX += velX;
        applyHorizontalCollision(level, rows, cols);

        if (isCollidingLeft && targetVelocityX < 0 && onGround) {
            currentState = Pushing;
            facingRight = false;
            leftAnimations[Pushing]->reset();
        }
        else if (isCollidingRight && targetVelocityX > 0 && onGround) {
            currentState = Pushing;
            facingRight = true;
            rightAnimations[Pushing]->reset();
        }

        if (currentState == Idle && onGround && isOnEdge(level, rows, cols)) {
            setEdgingAnimation();
        }

        float groundCheckOffset = 32.0f * scale;
        float checkLeftX = posX + groundCheckOffset;
        float checkRightX = posX + width - groundCheckOffset;

        onGround = false;
        float bottomY = posY + height;
        int row = static_cast<int>(bottomY / CELL_SIZE);
        if (row < rows) {
            int colLeft = static_cast<int>(checkLeftX / CELL_SIZE);
            if (colLeft >= 0 && colLeft < cols && (level[row][colLeft] == 'f' || level[row][colLeft] == 'p' || level[row][colLeft] == 'b')) {
                onGround = true;
            }
            int colRight = static_cast<int>(checkRightX / CELL_SIZE);
            if (!onGround && colRight >= 0 && colRight < cols && (level[row][colRight] == 'f' || level[row][colRight] == 'p' || level[row][colRight] == 'b')) {
                onGround = true;
            }
        }

        if (!onGround) velY = (velY + gravity < terminalVelocity ? velY + gravity : terminalVelocity);
        else if (!justJumped) velY = 0.0f;

        posY += velY;
        applyVerticalCollision(level, rows, cols);

        Animation* currentAnim = (facingRight && currentState != Jumping && currentState != Edging) ?
            rightAnimations[currentState] : leftAnimations[currentState];
        if (currentState == Edging) {
            currentAnim = facingRight ? rightAnimations[Edging] : leftAnimations[Edging];
        }
        if (currentAnim) {
            currentAnim->update(deltaTime);
            sprite.setTexture(*currentAnim->getTexture());
            sprite.setTextureRect(currentAnim->getCurrentFrame());
        }

        sprite.setPosition(posX, posY);
    }

private:
    float moveTime;
    int previousDirection;
    bool isBoosted;
};


class Tails : public Character {
public:
    Tails(float x, float y, int width, int height, float baseMaxSpeed, float scale,
         Texture& idleLeft,  Texture& idleRight,  Texture& runLeft,  Texture& runRight,
         Texture& jump,  Texture& pushLeft,  Texture& pushRight,  Texture& edgeLeft,  Texture& edgeRight,  Texture& flyLeft,  Texture& flyRight)
        : Character(x, y, width, height, baseMaxSpeed, scale), flyTimer(0.0f),
        isFlying(false), flyTime(0.0f), cooldownTime(0.0f), maxFlyTime(7.0f), maxCooldownTime(20.0f),
        moveTime(0.0f), previousDirection(0), isBoosted(false), stuckTimer(0.0f), lastPosX(x)
    {
        const int FRAME_WIDTH = 40;
        const int FRAME_HEIGHT = 40;
        const float FRAME_DURATION = 0.05f;
        const float IDLE_FRAME_DURATION = 0.2f;
        const float FLY_FRAME_DURATION = 0.08f;

        int idleLeftFrames = idleLeft.getSize().x / FRAME_WIDTH;
        leftAnimations[Idle] = new Animation(&idleLeft, FRAME_WIDTH, FRAME_HEIGHT, idleLeftFrames, IDLE_FRAME_DURATION);
        int idleRightFrames = idleRight.getSize().x / FRAME_WIDTH;
        rightAnimations[Idle] = new Animation(&idleRight, FRAME_WIDTH, FRAME_HEIGHT, idleRightFrames, IDLE_FRAME_DURATION);

        int runLeftFrames = runLeft.getSize().x / FRAME_WIDTH;
        leftAnimations[Running] = new Animation(&runLeft, FRAME_WIDTH, FRAME_HEIGHT, runLeftFrames, FRAME_DURATION);
        int runRightFrames = runRight.getSize().x / FRAME_WIDTH;
        rightAnimations[Running] = new Animation(&runRight, FRAME_WIDTH, FRAME_HEIGHT, runRightFrames, FRAME_DURATION);

        int jumpFrames = jump.getSize().x / FRAME_WIDTH;
        leftAnimations[Jumping] = new Animation(&jump, FRAME_WIDTH, FRAME_HEIGHT, jumpFrames, FRAME_DURATION);
        rightAnimations[Jumping] = new Animation(&jump, FRAME_WIDTH, FRAME_HEIGHT, jumpFrames, FRAME_DURATION);

        int pushLeftFrames = pushLeft.getSize().x / FRAME_WIDTH;
        leftAnimations[Pushing] = new Animation(&pushLeft, FRAME_WIDTH, FRAME_HEIGHT, pushLeftFrames, FRAME_DURATION);
        int pushRightFrames = pushRight.getSize().x / FRAME_WIDTH;
        rightAnimations[Pushing] = new Animation(&pushRight, FRAME_WIDTH, FRAME_HEIGHT, pushRightFrames, FRAME_DURATION);

        int flyLeftFrames = flyLeft.getSize().x / FRAME_WIDTH;
        leftAnimations[Flying] = new Animation(&flyLeft, FRAME_WIDTH, FRAME_HEIGHT, flyLeftFrames, FLY_FRAME_DURATION);
        int flyRightFrames = flyRight.getSize().x / FRAME_WIDTH;
        rightAnimations[Flying] = new Animation(&flyRight, FRAME_WIDTH, FRAME_HEIGHT, flyRightFrames, FLY_FRAME_DURATION);

        int edgeLeftFrames = edgeLeft.getSize().x / FRAME_WIDTH;
        leftAnimations[Edging] = new Animation(&edgeLeft, FRAME_WIDTH, FRAME_HEIGHT, edgeLeftFrames, FRAME_DURATION);
        int edgeRightFrames = edgeRight.getSize().x / FRAME_WIDTH;
        rightAnimations[Edging] = new Animation(&edgeRight, FRAME_WIDTH, FRAME_HEIGHT, edgeRightFrames, FRAME_DURATION);

        sprite.setTexture(*rightAnimations[Idle]->getTexture());
        sprite.setTextureRect(rightAnimations[Idle]->getCurrentFrame());
    }
    void update(float gravity, float terminalVelocity, float jumpStrength,
        const char** level, int rows, int cols, float deltaTime) override
    {
        justJumped = false;
        jumpedWhileStill = false;
        isCollidingLeft = false;
        isCollidingRight = false;

        bool isMovingLeft =  Keyboard::isKeyPressed( Keyboard::Left);
        bool isMovingRight =  Keyboard::isKeyPressed( Keyboard::Right);
        bool isFlyingInput =  Keyboard::isKeyPressed( Keyboard::T);
        int currentDirection = (isMovingRight ? 1 : (isMovingLeft ? -1 : 0));

        if (isFlyingInput && !isFlying && onGround) {
            isFlying = true;
            flyTimer = maxFlyTime;
            leftAnimations[Flying]->reset();
            rightAnimations[Flying]->reset();
        }

        if (isFlying) {
            flyTimer -= deltaTime;
            velY = -10.0f; // Adjust flying speed as needed
            currentState = Flying;
            if (flyTimer <= 0.0f || !isFlyingInput) {
                isFlying = false;
            }
        }
        else if (!onGround) {
            currentState = Jumping;
        }
        else if (currentDirection != 0) {
            currentState = Running;
        }
        else {
            currentState = Idle;
        }

        if ( Keyboard::isKeyPressed( Keyboard::Up) && onGround && !isFlying) {
            velY = jumpStrength;
            onGround = false;
            justJumped = true;
            if (velX == 0) {
                jumpedWhileStill = true;
                jumpedWhileStillThisFrame = true;
            }
            leftAnimations[Jumping]->reset();
            rightAnimations[Jumping]->reset();
        }

        if (velX > 0) facingRight = true;
        else if (velX < 0) facingRight = false;

        if (currentDirection == 0) velX = 0.0f;
        else velX = currentDirection * currentMaxSpeed;

        posX += velX;
        applyHorizontalCollision(level, rows, cols);

        if (isCollidingLeft && isMovingLeft && onGround) {
            currentState = Pushing;
            facingRight = false;
        }
        else if (isCollidingRight && isMovingRight && onGround) {
            currentState = Pushing;
            facingRight = true;
        }

        if (currentState == Idle && onGround && isOnEdge(level, rows, cols)) {
            setEdgingAnimation();
        }

        float groundCheckOffset = 32.0f * scale;
        float checkLeftX = posX + groundCheckOffset;
        float checkRightX = posX + width - groundCheckOffset;

        onGround = false;
        float bottomY = posY + height;
        int row = static_cast<int>(bottomY / CELL_SIZE);
        if (row < rows) {
            int colLeft = static_cast<int>(checkLeftX / CELL_SIZE);
            if (colLeft >= 0 && colLeft < cols && (level[row][colLeft] == 'f' || level[row][colLeft] == 'p' || level[row][colLeft] == 'b')) {
                onGround = true;
            }
            int colRight = static_cast<int>(checkRightX / CELL_SIZE);
            if (!onGround && colRight >= 0 && colRight < cols && (level[row][colRight] == 'f' || level[row][colRight] == 'p' || level[row][colRight] == 'b')) {
                onGround = true;
            }
        }

        if (!isFlying) {
            if (!onGround) velY = (velY + gravity < terminalVelocity ? velY + gravity : terminalVelocity);
            else if (!justJumped) velY = 0.0f;
        }

        posY += velY;
        applyVerticalCollision(level, rows, cols);

        Animation* currentAnim = nullptr;
        if (currentState == Jumping) {
            currentAnim = facingRight ? rightAnimations[Jumping] : leftAnimations[Jumping];
        }
        else if (currentState == Edging) {
            currentAnim = facingRight ? rightAnimations[Edging] : leftAnimations[Edging];
        }
        else if (currentState == Flying) {
            currentAnim = facingRight ? rightAnimations[Flying] : leftAnimations[Flying];
        }
        else {
            currentAnim = facingRight ? rightAnimations[currentState] : leftAnimations[currentState];
        }
        if (currentAnim) {
            currentAnim->update(deltaTime);
            sprite.setTexture(*currentAnim->getTexture());
            sprite.setTextureRect(currentAnim->getCurrentFrame());
        }

        sprite.setPosition(posX, posY);
    }

    void updateFollower(float gravity, float terminalVelocity, float jumpStrength,
        const char** level, int rows, int cols, float deltaTime,
        float targetX, float targetY, JumpQueue& jumpQueue) override
    {
        jumpDelayTimer -= deltaTime;
        justJumped = false;
        isCollidingLeft = false;
        isCollidingRight = false;

        if ( Keyboard::isKeyPressed( Keyboard::T) && !isFlying && cooldownTime <= 0.0f) {
            isFlying = true;
            flyTime = maxFlyTime;
            velY = -baseMaxSpeed;
            leftAnimations[Flying]->reset();
            rightAnimations[Flying]->reset();
        }

        if (isFlying) {
            flyTime -= deltaTime;
            velY = -baseMaxSpeed;
            if (flyTime <= 0.0f) {
                isFlying = false;
                cooldownTime = maxCooldownTime;
            }
        }
        else if (cooldownTime > 0.0f) cooldownTime -= deltaTime;

        float dx = targetX - posX;
        float targetVelocityX = 0.0f;

        if (abs(dx) > 15.0f) {
            followerMoveTime += deltaTime;
            if (followerMoveTime >= 2.0f) isFollowerBoosted = true;
            float speed = baseMaxSpeed * (isFollowerBoosted ? 1.2f : 1.0f);
            targetVelocityX = (dx > 0) ? speed : -speed;
            stuckTimer = 0.0f;
        }
        else {
            followerMoveTime = 0.0f;
            isFollowerBoosted = false;
        }

        if (abs(posX - lastPosX) < 1.0f && onGround) {
            stuckTimer += deltaTime;
        }
        else {
            stuckTimer = 0.0f;
            lastPosX = posX;
        }

        velX += (targetVelocityX - velX) * 0.1f;

        if (isFlying || !onGround) {
            if (currentState != Jumping && !isFlying) {
                leftAnimations[Jumping]->reset();
                rightAnimations[Jumping]->reset();
            }
            currentState = isFlying ? Flying : Jumping;
        }
        else if (abs(velX) > 0.1f) {
            if (currentState != Running) {
                leftAnimations[Running]->reset();
                rightAnimations[Running]->reset();
            }
            currentState = Running;
        }
        else {
            if (currentState != Idle) {
                leftAnimations[Idle]->reset();
                rightAnimations[Idle]->reset();
            }
            currentState = Idle;
        }

        if (targetVelocityX > 0) facingRight = true;
        else if (targetVelocityX < 0) facingRight = false;

        if (!isFlying) {
            int leftCol = int((posX + 8 * scale) / CELL_SIZE);
            int rightCol = int((posX + width - 8 * scale) / CELL_SIZE);
            int topRow = int((posY + 5 * scale) / CELL_SIZE);
            int botRow = static_cast<int>((posY + height) / CELL_SIZE);

            int direction = (velX > 0 && dx > 0) ? 1 : (velX < 0 && dx < 0) ? -1 : 0;

            if (direction != 0 && onGround) {
                bool shouldJump = false;
                int startCol = (direction == 1) ? rightCol : leftCol;

                for (int offset = 1; offset <= 2 && !shouldJump; ++offset) {
                    int aheadCol = startCol + offset * direction;
                    if (aheadCol < 0 || aheadCol >= cols) break;
                    for (int y = topRow; y <= botRow; ++y) {
                        char tile = level[y][aheadCol];
                        if (tile == 'w' || tile == 'b'||tile=='N') {
                            int obstacleX = aheadCol * CELL_SIZE;
                            bool beyond = (direction == 1) ? (targetX > obstacleX) : (targetX < obstacleX);
                            if (beyond) {
                                shouldJump = true;
                            }
                            break;
                        }
                    }
                }

                int detectedGap = 0;
                if (!shouldJump && botRow < rows) {
                    for (int gapWidth = 1; gapWidth <= 4 && !shouldJump; ++gapWidth) {
                        bool isGap = true;
                        for (int offset = 1; offset <= gapWidth; ++offset) {
                            int gapCol = startCol + offset * direction;
                            if (gapCol < 0 || gapCol >= cols || level[botRow][gapCol] != ' ') {
                                isGap = false;
                                break;
                            }
                        }
                        if (isGap) {
                            int landingCol = startCol + (gapWidth + 1) * direction;
                            if (landingCol >= 0 && landingCol < cols) {
                                char landTile = level[botRow][landingCol];
                                if (landTile == 'f' || landTile == 'p' || landTile == 'b') {
                                    int landingX = landingCol * CELL_SIZE;
                                    bool beyond = (direction == 1) ? (targetX > landingX) : (targetX < landingX);
                                    if (beyond) {
                                         cout << "Gap detected: " << gapWidth <<  endl;
                                        shouldJump = true;
                                        detectedGap = gapWidth;
                                    }
                                }
                            }
                        }
                    }
                }

                if (shouldJump) {
                    float jumpMultiplier = 1.0f + detectedGap * 1.5f;
                    velY = jumpStrength *jumpMultiplier; // Increased jump strength for follower
                    velX = targetVelocityX * 1.5f;
                    onGround = false;
                    justJumped = true;
                    jumpDelayTimer = (direction == -1) ? 1.9f : 0.2f;
                    leftAnimations[Jumping]->reset();
                    rightAnimations[Jumping]->reset();
                }
            }

            if (!jumpQueue.isEmpty()) {
                float jumpX = jumpQueue.peek();
                float tolerance = 2.0f;
                bool movingRight = velX > 0;
                bool movingLeft = velX < 0;
                bool atJumpPos = false;

                if (movingRight && posX >= jumpX - tolerance) atJumpPos = true;
                else if (movingLeft && posX <= jumpX + tolerance) atJumpPos = true;
                else if (fabs(posX - jumpX) < tolerance) atJumpPos = true;

                if (atJumpPos && onGround) {
                    velY = jumpStrength * 1.1f; // Increased jump strength for follower
                    onGround = false;
                    justJumped = true;
                    jumpQueue.dequeue();
                    leftAnimations[Jumping]->reset();
                    rightAnimations[Jumping]->reset();
                }
            }
        }

        if (stuckTimer >= 3.0f && abs(targetX - posX) > 200.0f) {
            float cameraX = targetX - SCREEN_X / 2.0f;
            posX = cameraX - 300.0f;
            posY = targetY;
            velX = 0.0f;
            velY = 0.0f;
            onGround = true;
            stuckTimer = 0.0f;
            lastPosX = posX;
        }

        posX += velX;
        applyHorizontalCollision(level, rows, cols);

        if (isCollidingLeft && targetVelocityX < 0 && onGround) {
            currentState = Pushing;
            facingRight = false;
            leftAnimations[Pushing]->reset();
        }
        else if (isCollidingRight && targetVelocityX > 0 && onGround) {
            currentState = Pushing;
            facingRight = true;
            rightAnimations[Pushing]->reset();
        }

        if (currentState == Idle && onGround && isOnEdge(level, rows, cols)) {
            setEdgingAnimation();
        }

        float groundCheckOffset = 32.0f * scale;
        float checkLeftX = posX + groundCheckOffset;
        float checkRightX = posX + width - groundCheckOffset;

        onGround = false;
        float bottomY = posY + height;
        int row = static_cast<int>(bottomY / CELL_SIZE);
        if (row < rows) {
            int colLeft = static_cast<int>(checkLeftX / CELL_SIZE);
            if (colLeft >= 0 && colLeft < cols && (level[row][colLeft] == 'f' || level[row][colLeft] == 'p' || level[row][colLeft] == 'b')) {
                onGround = true;
            }
            int colRight = static_cast<int>(checkRightX / CELL_SIZE);
            if (!onGround && colRight >= 0 && colRight < cols && (level[row][colRight] == 'f' || level[row][colRight] == 'p' || level[row][colRight] == 'b')) {
                onGround = true;
            }
        }

        if (!isFlying) {
            if (!onGround) velY = (velY + gravity < terminalVelocity ? velY + gravity : terminalVelocity);
            else if (!justJumped) velY = 0.0f;
        }

        posY += velY;
        applyVerticalCollision(level, rows, cols);

        Animation* currentAnim = nullptr;
        if (currentState == Jumping) {
            currentAnim = facingRight ? rightAnimations[Jumping] : leftAnimations[Jumping];
        }
        else if (currentState == Edging) {
            currentAnim = facingRight ? rightAnimations[Edging] : leftAnimations[Edging];
        }
        else if (currentState == Flying) {
            currentAnim = facingRight ? rightAnimations[Flying] : leftAnimations[Flying];
        }
        else {
            currentAnim = facingRight ? rightAnimations[currentState] : leftAnimations[currentState];
        }
        if (currentAnim) {
            currentAnim->update(deltaTime);
            sprite.setTexture(*currentAnim->getTexture());
            sprite.setTextureRect(currentAnim->getCurrentFrame());
        }

        sprite.setPosition(posX, posY);
    }

     string getTimerText() const {
        if (isFlying) return "Fly Time: " +  to_string(int(flyTime + 0.5f)) + "s";
        else if (cooldownTime > 0.0f) return "Cooldown: " +  to_string(int(cooldownTime + 0.5f)) + "s";
        else return "Press T to Fly";
    }

private:
    float flyTimer;
    bool isFlying;
    float flyTime;
    float cooldownTime;
    const float maxFlyTime;
    const float maxCooldownTime;
    float moveTime;
    int previousDirection;
    bool isBoosted;
    float stuckTimer;
    float lastPosX;
};