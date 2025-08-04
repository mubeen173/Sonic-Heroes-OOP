#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

using namespace sf;

class Projectile {
public:
    Projectile(float startX, float startY, float targetX, float targetY, float speed, Texture& texture)
        : posX(startX), posY(startY), speed(speed), isActive(true)
    {
        // Calculate direction towards target
        float dx = targetX - startX;
        float dy = targetY - startY;
        float distance = sqrt(dx * dx + dy * dy);
        
        // Normalize direction and set velocity
        if (distance > 0) {
            velX = (dx / distance) * speed;
            velY = (dy / distance) * speed;
        } else {
            velX = speed; // Default to moving right if target is at same position
            velY = 0;
        }

        // Set up sprite
        sprite.setTexture(texture);
        sprite.setPosition(posX, posY);
        sprite.setScale(0.5f, 0.5f); // Small projectile size
    }

    void update(float deltaTime, const char** level, int rows, int cols, float playerX, float playerY, int playerWidth, int playerHeight)
    {
        if (!isActive) return;

        // Update position
        posX += velX * deltaTime;
        posY += velY * deltaTime;
        sprite.setPosition(posX, posY);

        // Check collision with level
        if (checkLevelCollision(level, rows, cols)) {
            isActive = false; // Banish on hitting level geometry
            return;
        }

        // Check collision with player
        if (checkPlayerCollision(playerX, playerY, playerWidth, playerHeight)) {
            isActive = false; // Banish on hitting player
        }
    }

    void draw(RenderWindow& window)
    {
        if (isActive) {
            window.draw(sprite);
        }
    }

    bool isAlive() const { return isActive; }

    bool causesDamage() const
    {
        return isActive; // Causes damage when active and hits player
    }

private:
    float posX, posY;
    float velX, velY;
    float speed;
    bool isActive;
    Sprite sprite;
    static const int CELL_SIZE = 32; // Must match game's cell size

    bool checkLevelCollision(const char** level, int rows, int cols)
    {
        int leftCol = static_cast<int>(posX / CELL_SIZE);
        int rightCol = static_cast<int>((posX + 16) / CELL_SIZE); // Assuming 16x16 projectile size
        int topRow = static_cast<int>(posY / CELL_SIZE);
        int botRow = static_cast<int>((posY + 16) / CELL_SIZE);

        // Check all cells the projectile might intersect
        for (int y = topRow; y <= botRow; ++y) {
            for (int x = leftCol; x <= rightCol; ++x) {
                if (x >= 0 && x < cols && y >= 0 && y < rows) {
                    char c = level[y][x];
                    if (c == 'w' || c == 'b' || c == 'p' || c == 'f') { // Collides with walls, blocks, platforms, floors
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool checkPlayerCollision(float playerX, float playerY, int playerWidth, int playerHeight)
    {
        // Simple AABB collision detection
        float projLeft = posX;
        float projRight = posX + 16; // Assuming 16x16 projectile size
        float projTop = posY;
        float projBottom = posY + 16;

        float playerLeft = playerX;
        float playerRight = playerX + playerWidth;
        float playerTop = playerY;
        float playerBottom = playerY + playerHeight;

        return (projLeft < playerRight && projRight > playerLeft &&
                projTop < playerBottom && projBottom > playerTop);
    }
};