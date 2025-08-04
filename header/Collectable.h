#pragma once
#include <SFML/Graphics.hpp>
#include "Character.h"
#include <string>
#include <iostream>

class Collectable {
public:
    Collectable(float x, float y, float width, float height, const  Texture& texture, int scoreValue)
        : posX(x), posY(y), width(width), height(height), isCollected(false), scoreValue(scoreValue) {
        sprite.setTexture(texture);
        sprite.setPosition(x, y);
        sprite.setScale(width / texture.getSize().x, height / texture.getSize().y);
    }

    virtual ~Collectable() = default;

    bool collisionCheck(const Character& character) {
        if (isCollected) return false;

        float charX = character.getPosX();
        float charY = character.getPosY();
        float charWidth = character.getWidth();
        float charHeight = character.getHeight();

        bool overlapX = (charX < posX + width) && (charX + charWidth > posX);
        bool overlapY = (charY < posY + height) && (charY + charHeight > posY);

        if (overlapX && overlapY) {
            isCollected = true;
            onCollect(character);
            return true;
        }
        return false;
    }

    virtual void onCollect(const Character& character) = 0;

    void draw( RenderWindow& window, const  RenderStates& states =  RenderStates::Default) const {
        if (!isCollected) {
            window.draw(sprite, states);
        }
    }

    bool getIsCollected() const { return isCollected; }
    float getPosX() const { return posX; }
    float getPosY() const { return posY; }
    int getScoreValue() const { return scoreValue; }

protected:
    float posX, posY;
    float width, height;
    bool isCollected;
    int scoreValue;
     Sprite sprite;
};

class Ring : public Collectable {
public:
    Ring(float x, float y, float width, float height, const  Texture& texture)
        : Collectable(x, y, width, height, texture, 5) {}

    void onCollect(const Character& character) override {
        std::cout << "Ring collected! Score +5\n";
    }
};

class ExtraLife : public Collectable {
public:
    ExtraLife(float x, float y, float width, float height, const  Texture& texture)
        : Collectable(x, y, width, height, texture, 10) {}

    void onCollect(const Character& character) override {
        std::cout << "Extra Life collected! Score +10, HP +1\n";
    }
};

class SpecialBoost : public Collectable {
public:
    static const int SPEED = 0;
    static const int JUMP = 1;
    static const int INVINCIBILITY = 2;

    SpecialBoost(float x, float y, float width, float height, const  Texture& texture, int type)
        : Collectable(x, y, width, height, texture, 20), boostType(type), duration(10.0f) {}

    void onCollect(const Character& character) override {
        std::cout << "Boost collected! Score +20\n";
    }

    int getBoostType() const { return boostType; }
    float getDuration() const { return duration; }

private:
    int boostType;
    float duration;
};