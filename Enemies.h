#pragma once
#include <SFML/Graphics.hpp>
#include "Animation.h"
#include "Projectile.h"
#include <string>
#include <iostream>
#include <cmath>

using namespace sf;
using namespace std;

class Enemy {
public:
    static const int Idle = 0;
    static const int Moving = 1;
    static const int StateCount = 2;

    Enemy(float x, float y, int width, int height, float speed, int maxHP, float scale = 1.0f)
        : sprite(), posX(x), posY(y), velX(0.0f), velY(0.0f), onGround(false), scale(scale),
        speed(speed), maxHP(maxHP), currentHP(maxHP), currentState(Idle), isActive(true),
        facingRight(true), initialX(x), sectionLeft(x - 150.0f), sectionRight(x + 150.0f)
    {
        sprite.setScale(scale, scale);
        sprite.setPosition(x, y);
        this->width = width * scale;
        this->height = height * scale;
        for (int i = 0; i < StateCount; ++i) {
            leftAnimations[i] = nullptr;
            rightAnimations[i] = nullptr;
        }

    }

    virtual ~Enemy() {
        for (int i = 0; i < StateCount; ++i) {
            delete leftAnimations[i];
            delete rightAnimations[i];
        }
    }

    float getPosX() const { return posX; }
    float getPosY() const { return posY; }
    int getCurrentHP() const { return currentHP; }
    bool isAlive() const { return isActive && currentHP > 0; }

    int getEnemyWidth() const { return width; }
    int getEnemyHeight() const { return height; }
    bool getFacingRight() const { return facingRight; }

    virtual void update(float gravity, float terminalVelocity, const char** level, int rows, int cols,
        float deltaTime, float playerX, float playerY, bool playerInBallForm)
    {
        if (!isActive) return;

        posX += velX * deltaTime;
        applyHorizontalCollision(level, rows, cols);
        posY += velY * deltaTime;
        applyVerticalCollision(level, rows, cols, gravity, terminalVelocity, deltaTime);

        updateAnimation(deltaTime);
        sprite.setPosition(posX, posY);

    }

    virtual void draw(RenderWindow& window, const RenderStates& states = RenderStates::Default) {
        if (isActive) {
            window.draw(sprite, states);
           
        }
    }

    bool takeDamage(int damage, bool fromBallForm) {
        if (!isActive) return false;
        if (fromBallForm) {
            currentHP -= damage;
            
            if (currentHP <= 0) {
                isActive = false;
                return true; // Enemy defeated
            }
            return true; // Damage applied
        }
        return false; // No damage applied (not in ball form)
    }

    virtual char getType() const = 0;
    int getMaxHP() const { return maxHP; }
protected:
    Sprite sprite;
    float posX, posY;
    float velX, velY;
    bool onGround;
    float scale;
    int width, height;
    float speed;
    int maxHP, currentHP;
    bool facingRight;
    Animation* leftAnimations[StateCount];
    Animation* rightAnimations[StateCount];
    int currentState;
    bool isActive;
    float initialX;
    float sectionLeft, sectionRight;

    // HP display
    Font font;
   

   

    void applyVerticalCollision(const char** level, int rows, int cols, float gravity,
        float terminalVelocity, float deltaTime)
    {
        int leftCol = static_cast<int>((posX + 8 * scale) / CELL_SIZE);
        int rightCol = static_cast<int>((posX + width - 8 * scale) / CELL_SIZE);
        int botRow = static_cast<int>((posY + height) / CELL_SIZE);
        int topRow = static_cast<int>(posY / CELL_SIZE);

        if (!onGround) {
            velY = (velY + gravity * deltaTime < terminalVelocity) ? velY + gravity * deltaTime : terminalVelocity;
        }

        if (velY > 0 && botRow < rows) {
            bool anyFloor = false;
            for (int x = leftCol; x <= rightCol; ++x) {
                if (x >= 0 && x < cols && botRow < rows) {
                    char c = level[botRow][x];
                    if (c == 'f' || c == 'p' || c == 'b') {
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
            else {
                onGround = false;
            }
        }

        if (velY < 0 && topRow >= 0) {
            bool hitCeiling = false;
            for (int x = leftCol; x <= rightCol; ++x) {
                if (x >= 0 && x < cols && topRow < rows) {
                    char c = level[topRow][x];
                    if (c == 'r' || c == 'b' || c == 'p') {
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

        if (posY > rows * CELL_SIZE) {
            isActive = false;
        }
    }

    void applyHorizontalCollision(const char** level, int rows, int cols)
    {
        int leftCol = static_cast<int>((posX + 8 * scale) / CELL_SIZE);
        int rightCol = static_cast<int>((posX + width - 8 * scale) / CELL_SIZE);
        int topRow = static_cast<int>((posY + 5 * scale) / CELL_SIZE);
        int botRow = static_cast<int>((posY + height - 5 * scale) / CELL_SIZE);

        if (velX < 0 && leftCol >= 0) {
            for (int y = topRow; y <= botRow; ++y) {
                if (y >= 0 && y < rows && leftCol < cols) {
                    char c = level[y][leftCol];
                    if (c == 'w' || c == 'b' || c == 'p') {
                        posX = (leftCol + 1) * CELL_SIZE - 8 * scale;
                        velX = 0;
                        break;
                    }
                }
            }
        }

        if (velX > 0 && rightCol < cols) {
            for (int y = topRow; y <= botRow; ++y) {
                if (y >= 0 && y < rows && rightCol >= 0) {
                    char c = level[y][rightCol];
                    if (c == 'w' || c == 'b' || c == 'p') {
                        posX = rightCol * CELL_SIZE - width + 8 * scale;
                        velX = 0;
                        break;
                    }
                }
            }
        }
    }

    void updateAnimation(float deltaTime) {
        Animation* currentAnim = facingRight ? rightAnimations[currentState] : leftAnimations[currentState];
        if (currentAnim) {
            currentAnim->update(deltaTime);
            sprite.setTexture(*currentAnim->getTexture());
            sprite.setTextureRect(currentAnim->getCurrentFrame());
        }
    }
};

class BatBrain : public Enemy {
public:
    char getType() const override { return 'B'; }
    BatBrain(float x, float y, float scale, Texture& idleLeft, Texture& idleRight,
        Texture& moveLeft, Texture& moveRight)
        : Enemy(x, y, 32, 32, 90.0f, 3, 2.0f)  // HP is 3
    {
        const int FRAME_WIDTH = 32;
        const int FRAME_HEIGHT = 32;
        const float FRAME_DURATION = 0.1f;

        sectionLeft = x - 120.0f;
        sectionRight = x + 120.0f;

        leftAnimations[Idle] = new Animation(&idleLeft, FRAME_WIDTH, FRAME_HEIGHT, 1, FRAME_DURATION);
        rightAnimations[Idle] = new Animation(&idleRight, FRAME_WIDTH, FRAME_HEIGHT, 1, FRAME_DURATION);

        int moveLeftFrames = moveLeft.getSize().x / FRAME_WIDTH;
        leftAnimations[Moving] = new Animation(&moveLeft, FRAME_WIDTH, FRAME_HEIGHT, moveLeftFrames, FRAME_DURATION);
        int moveRightFrames = moveRight.getSize().x / FRAME_WIDTH;
        rightAnimations[Moving] = new Animation(&moveRight, FRAME_WIDTH, FRAME_HEIGHT, moveRightFrames, FRAME_DURATION);

        sprite.setTexture(*rightAnimations[Idle]->getTexture());
        sprite.setTextureRect(rightAnimations[Idle]->getCurrentFrame());

     
    }

    void update(float gravity, float terminalVelocity, const char** level, int rows, int cols,
        float deltaTime, float playerX, float playerY, bool playerInBallForm) override
    {
        if (!isActive) {
            Enemy::update(gravity, terminalVelocity, level, rows, cols, deltaTime, playerX, playerY, playerInBallForm);
            return;
        }

        float dx = playerX - posX;
        float dy = playerY - posY;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance < 300.0f) {
            currentState = Moving;
            velX = (dx > 0 ? speed * 0.6f : -speed * 0.6f);
            velY = (dy > 0 ? speed * 0.6f : -speed * 0.6f);
            facingRight = dx > 0;
        }
        else {
            currentState = Idle;
            velX = 0.0f;
            velY = 0.0f;
            if (abs(posX - initialX) > 5.0f) {
                velX = (posX > initialX) ? -speed * 0.5f : speed * 0.5f;
                facingRight = (posX < initialX);
                currentState = Moving;
            }
        }

        if (posX < sectionLeft) {
            posX = sectionLeft;
            velX = speed * 0.5f;
            facingRight = true;
        }
        else if (posX > sectionRight) {
            posX = sectionRight;
            velX = -speed * 0.5f;
            facingRight = false;
        }

        Enemy::update(gravity, terminalVelocity, level, rows, cols, deltaTime, playerX, playerY, playerInBallForm);
    }
};

class BeeBot : public Enemy {
public:
    char getType() const override { return 'E'; }
    BeeBot(float x, float y, float scale, Texture& idleLeft, Texture& idleRight,
        Texture& moveLeft, Texture& moveRight)
        : Enemy(x, y, 32, 32, 120.0f, 5, 1.5f),
        shootTimer(0.0f), shootCooldown(5.0f)
    {
        const int FRAME_WIDTH = 32;
        const int FRAME_HEIGHT = 32;
        const float FRAME_DURATION = 0.1f;

        sectionLeft = x - 180.0f;
        sectionRight = x + 180.0f;

        leftAnimations[Idle] = new Animation(&idleLeft, FRAME_WIDTH, FRAME_HEIGHT, 1, FRAME_DURATION);
        rightAnimations[Idle] = new Animation(&idleRight, FRAME_WIDTH, FRAME_HEIGHT, 1, FRAME_DURATION);

        int moveLeftFrames = moveLeft.getSize().x / FRAME_WIDTH;
        leftAnimations[Moving] = new Animation(&moveLeft, FRAME_WIDTH, FRAME_HEIGHT, moveLeftFrames, FRAME_DURATION);
        int moveRightFrames = moveRight.getSize().x / FRAME_WIDTH;
        rightAnimations[Moving] = new Animation(&moveRight, FRAME_WIDTH, FRAME_HEIGHT, moveRightFrames, FRAME_DURATION);

        sprite.setTexture(*rightAnimations[Idle]->getTexture());
        sprite.setTextureRect(rightAnimations[Idle]->getCurrentFrame());

        
    }

    void update(float gravity, float terminalVelocity, const char** level, int rows, int cols,
        float deltaTime, float playerX, float playerY, bool playerInBallForm) override
    {
        if (!isActive) {
            Enemy::update(gravity, terminalVelocity, level, rows, cols, deltaTime, playerX, playerY, playerInBallForm);
            return;
        }

        shootTimer += deltaTime;

        float dx = playerX - posX;
        float distance = abs(dx);

        if (distance < 300.0f) {
            currentState = Moving;
            velX = (dx > 0 ? speed * 0.6f : -speed * 0.6f);
            velY = sin(posX * 0.05f) * 5.0f;
            facingRight = dx > 0;
        }
        else {
            currentState = Moving;
            velX = facingRight ? speed * 0.5f : -speed * 0.5f;
            velY = sin(posX * 0.05f) * 5.0f;
            if (abs(posX - initialX) > 5.0f) {
                velX = (posX > initialX) ? -speed * 0.5f : speed * 0.5f;
                facingRight = (posX < initialX);
            }
        }

        if (posX < sectionLeft) {
            posX = sectionLeft;
            velX = speed * 0.5f;
            facingRight = true;
        }
        else if (posX > sectionRight) {
            posX = sectionRight;
            velX = -speed * 0.5f;
            facingRight = false;
        }

        Enemy::update(gravity, terminalVelocity, level, rows, cols, deltaTime, playerX, playerY, playerInBallForm);
    }

    Projectile* shootProjectile(float playerX, float playerY, Texture& projectileTexture)
    {
        cout << "BeeBot shooting projectile!" << endl;
        float startX = posX + (facingRight ? width : -16);
        float startY = posY + height / 2;
        return new Projectile(startX, startY, playerX, playerY, 100.0f, projectileTexture);
    }

private:
    float shootTimer;
    float shootCooldown;
};
class Motobug : public Enemy {
public:
    char getType() const override { return 'M'; }
    Motobug(float x, float y, float scale, Texture& idleLeft, Texture& idleRight,
        Texture& moveLeft, Texture& moveRight)
        : Enemy(x, y, 32, 32, 30.0f, 4, scale),
        startX(x), patrolState(0), shootTimer(0.0f), shootCooldown(5.0f), justSawPlayer(false)
    {
        const int FRAME_WIDTH = 32;
        const int FRAME_HEIGHT = 32;
        const float FRAME_DURATION = 0.1f;

        leftAnimations[Idle] = new Animation(&idleLeft, FRAME_WIDTH, FRAME_HEIGHT, 1, FRAME_DURATION);
        rightAnimations[Idle] = new Animation(&idleRight, FRAME_WIDTH, FRAME_HEIGHT, 1, FRAME_DURATION);

        int moveLeftFrames = moveLeft.getSize().x / FRAME_WIDTH;
        leftAnimations[Moving] = new Animation(&moveLeft, FRAME_WIDTH, FRAME_HEIGHT, moveLeftFrames, FRAME_DURATION);
        int moveRightFrames = moveRight.getSize().x / FRAME_WIDTH;
        rightAnimations[Moving] = new Animation(&moveRight, FRAME_WIDTH, FRAME_HEIGHT, moveRightFrames, FRAME_DURATION);

        sprite.setTexture(*rightAnimations[Idle]->getTexture());
        sprite.setTextureRect(rightAnimations[Idle]->getCurrentFrame());
    }

    void update(float gravity, float terminalVelocity, const char** level, int rows, int cols,
        float deltaTime, float playerX, float playerY, bool playerInBallForm) override
    {
        if (!isActive) {
            Enemy::update(gravity, terminalVelocity, level, rows, cols, deltaTime, playerX, playerY, playerInBallForm);
            return;
        }

        shootTimer += deltaTime;

        float dx = playerX - posX;
        float distance = abs(dx);

        bool wasSeeingPlayer = justSawPlayer;
        justSawPlayer = (distance < 300.0f);

        if (justSawPlayer && !wasSeeingPlayer) {
            shootTimer = shootCooldown; // Trigger immediate shot when player enters range
        }

        if (distance < 300.0f) {
            currentState = Moving;
            velX = (dx > 0 ? speed * 0.6f : -speed * 0.6f);
            facingRight = dx > 0;
        }
        else {
            currentState = Moving;
            switch (patrolState) {
            case 0:
                velX = speed * 0.5f;
                facingRight = true;
                if (posX >= startX + 3 * CELL_SIZE) {
                    posX = startX + 3 * CELL_SIZE;
                    velX = -speed * 0.5f;
                    facingRight = false;
                    patrolState = 1;
                }
                break;
            case 1:
                velX = -speed * 0.5f;
                facingRight = false;
                if (posX <= startX - 3 * CELL_SIZE) {
                    posX = startX - 3 * CELL_SIZE;
                    velX = speed * 0.5f;
                    facingRight = true;
                    patrolState = 2;
                }
                break;
            case 2:
                velX = speed * 0.5f;
                facingRight = true;
                if (posX >= startX + 3 * CELL_SIZE) {
                    posX = startX + 3 * CELL_SIZE;
                    velX = -speed * 0.5f;
                    facingRight = false;
                    patrolState = 3;
                }
                break;
            case 3:
                velX = -speed * 0.5f;
                facingRight = false;
                if (posX <= startX - 3 * CELL_SIZE) {
                    posX = startX - 3 * CELL_SIZE;
                    velX = speed * 0.5f;
                    facingRight = true;
                    patrolState = 0;
                }
                break;
            }
        }

        if (posX < sectionLeft) {
            posX = sectionLeft;
            velX = speed * 0.5f;
            facingRight = true;
            patrolState = 0;
        }
        else if (posX > sectionRight) {
            posX = sectionRight;
            velX = -speed * 0.5f;
            facingRight = false;
            patrolState = 1;
        }

        Enemy::update(gravity, terminalVelocity, level, rows, cols, deltaTime, playerX, playerY, playerInBallForm);
    }

    Projectile* shootProjectile(float playerX, float playerY, Texture& projectileTexture)
    {
        cout << "Motobug shooting projectile!" << endl;
        float startX = posX + (facingRight ? width : -16);
        float startY = posY + height / 2;
        return new Projectile(startX, startY, playerX, playerY, 100.0f, projectileTexture);
    }

    bool shouldShoot() const {
        return shootTimer >= shootCooldown;
    }

    void resetShootTimer() {
        shootTimer = 0.0f;
    }

private:
    float startX;
    int patrolState;
    float shootTimer;
    float shootCooldown;
    bool justSawPlayer;
};

class CrabMeat : public Enemy {
public:
    char getType() const override { return 'C'; }
    CrabMeat(float x, float y, float scale, Texture& idleLeft, Texture& idleRight,
        Texture& moveLeft, Texture& moveRight)
        : Enemy(x, y, 32, 32, 30.0f, 4, 3.0f),
        startX(x), patrolState(0), shootTimer(0.0f), shootCooldown(5.0f), justSawPlayer(false)
    {
        const int FRAME_WIDTH = 32;
        const int FRAME_HEIGHT = 32;
        const float FRAME_DURATION = 0.1f;

        leftAnimations[Idle] = new Animation(&idleLeft, FRAME_WIDTH, FRAME_HEIGHT, 1, FRAME_DURATION);
        rightAnimations[Idle] = new Animation(&idleRight, FRAME_WIDTH, FRAME_HEIGHT, 1, FRAME_DURATION);

        int moveLeftFrames = moveLeft.getSize().x / FRAME_WIDTH;
        leftAnimations[Moving] = new Animation(&moveLeft, FRAME_WIDTH, FRAME_HEIGHT, moveLeftFrames, FRAME_DURATION);
        int moveRightFrames = moveRight.getSize().x / FRAME_WIDTH;
        rightAnimations[Moving] = new Animation(&moveRight, FRAME_WIDTH, FRAME_HEIGHT, moveRightFrames, FRAME_DURATION);

        sprite.setTexture(*rightAnimations[Idle]->getTexture());
        sprite.setTextureRect(rightAnimations[Idle]->getCurrentFrame());
    }

    void update(float gravity, float terminalVelocity, const char** level, int rows, int cols,
        float deltaTime, float playerX, float playerY, bool playerInBallForm) override
    {
        if (!isActive) {
            Enemy::update(gravity, terminalVelocity, level, rows, cols, deltaTime, playerX, playerY, playerInBallForm);
            return;
        }

        shootTimer += deltaTime;

        float dx = playerX - posX;
        float distance = abs(dx);

        bool wasSeeingPlayer = justSawPlayer;
        justSawPlayer = (distance < 300.0f);

        if (justSawPlayer && !wasSeeingPlayer) {
            shootTimer = shootCooldown; // Trigger immediate shot when player enters range
        }

        currentState = Moving;
        switch (patrolState) {
        case 0:
            velX = speed * 0.5f;
            facingRight = true;
            if (posX >= startX + 3 * CELL_SIZE) {
                posX = startX + 3 * CELL_SIZE;
                velX = -speed * 0.5f;
                facingRight = false;
                patrolState = 1;
            }
            break;
        case 1:
            velX = -speed * 0.5f;
            facingRight = false;
            if (posX <= startX - 3 * CELL_SIZE) {
                posX = startX - 3 * CELL_SIZE;
                velX = speed * 0.5f;
                facingRight = true;
                patrolState = 2;
            }
            break;
        case 2:
            velX = speed * 0.5f;
            facingRight = true;
            if (posX >= startX + 3 * CELL_SIZE) {
                posX = startX + 3 * CELL_SIZE;
                velX = -speed * 0.5f;
                facingRight = false;
                patrolState = 3;
            }
            break;
        case 3:
            velX = -speed * 0.5f;
            facingRight = false;
            if (posX <= startX - 3 * CELL_SIZE) {
                posX = startX - 3 * CELL_SIZE;
                velX = speed * 0.5f;
                facingRight = true;
                patrolState = 0;
            }
            break;
        }

        if (posX < sectionLeft) {
            posX = sectionLeft;
            velX = speed * 0.5f;
            facingRight = true;
            patrolState = 0;
        }
        else if (posX > sectionRight) {
            posX = sectionRight;
            velX = -speed * 0.5f;
            facingRight = false;
            patrolState = 1;
        }

        Enemy::update(gravity, terminalVelocity, level, rows, cols, deltaTime, playerX, playerY, playerInBallForm);
    }

    Projectile* shootProjectile(float playerX, float playerY, Texture& projectileTexture)
    {
        cout << "CrabMeat shooting projectile!" << endl;
        float startX = posX + (facingRight ? width : -16);
        float startY = posY + height / 2;
        return new Projectile(startX, startY, playerX, playerY, 100.0f, projectileTexture);
    }

    bool shouldShoot() const {
        return shootTimer >= shootCooldown;
    }

    void resetShootTimer() {
        shootTimer = 0.0f;
    }

private:
    float startX;
    int patrolState;
    float shootTimer;
    float shootCooldown;
    bool justSawPlayer;
};

class EggStinger : public Enemy {
public:
    char getType() const override { return 'S'; }
    EggStinger(float x, float y, float scale, Texture& idleLeft, Texture& idleRight,
        Texture& moveLeft, Texture& moveRight)
        : Enemy(x, y, 32, 32, 18.0f, 15, scale)
    {
        const int FRAME_WIDTH = 32;
        const int FRAME_HEIGHT = 32;
        const float FRAME_DURATION = 0.1f;

        sectionLeft = x - 300.0f;
        sectionRight = x + 300.0f;

        leftAnimations[Idle] = new Animation(&idleLeft, FRAME_WIDTH, FRAME_HEIGHT, 1, FRAME_DURATION);
        rightAnimations[Idle] = new Animation(&idleRight, FRAME_WIDTH, FRAME_HEIGHT, 1, FRAME_DURATION);

        int moveLeftFrames = moveLeft.getSize().x / FRAME_WIDTH;
        leftAnimations[Moving] = new Animation(&moveLeft, FRAME_WIDTH, FRAME_HEIGHT, moveLeftFrames, FRAME_DURATION);
        int moveRightFrames = moveRight.getSize().x / FRAME_WIDTH;
        rightAnimations[Moving] = new Animation(&moveRight, FRAME_WIDTH, FRAME_HEIGHT, moveRightFrames, FRAME_DURATION);

        sprite.setTexture(*rightAnimations[Idle]->getTexture());
        sprite.setTextureRect(rightAnimations[Idle]->getCurrentFrame());

       
    }

    void update(float gravity, float terminalVelocity, const char** level, int rows, int cols,
        float deltaTime, float playerX, float playerY, bool playerInBallForm) override
    {
        if (!isActive) {
            Enemy::update(gravity, terminalVelocity, level, rows, cols, deltaTime, playerX, playerY, playerInBallForm);
            return;
        }

        float dx = playerX - posX;
        float distance = abs(dx);

        if (distance < 300.0f) {
            currentState = Moving;
            velX = (dx > 0 ? speed * 0.6f : -speed * 0.6f);
            velY = -5.0f;
            facingRight = dx > 0;
        }
        else {
            currentState = Moving;
            velX = facingRight ? speed * 0.5f : -speed * 0.5f;
            velY = -5.0f;
            if (abs(posX - initialX) > 5.0f) {
                velX = (posX > initialX) ? -speed * 0.5f : speed * 0.5f;
                facingRight = (posX < initialX);
            }
        }

        if (posX < sectionLeft) {
            posX = sectionLeft;
            velX = speed * 0.5f;
            facingRight = true;
        }
        else if (posX > sectionRight) {
            posX = sectionRight;
            velX = -speed * 0.5f;
            facingRight = false;
        }

        Enemy::update(gravity, terminalVelocity, level, rows, cols, deltaTime, playerX, playerY, playerInBallForm);
    }
};