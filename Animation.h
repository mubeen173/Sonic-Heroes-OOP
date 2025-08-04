#pragma once
#include <SFML/Graphics.hpp>

class Animation {
public:
    Animation(sf::Texture* texture, int frameWidth, int frameHeight, int frameCount, float frameDuration)
        : texture(texture), frameCount(frameCount), frameDuration(frameDuration),
          currentFrame(0), animationTimer(0.0f)
    {
        frames = new sf::IntRect[frameCount];
        for (int i = 0; i < frameCount; ++i) {
            frames[i] = sf::IntRect(i * frameWidth, 0, frameWidth, frameHeight);
        }
    }

    ~Animation() {
        delete[] frames;
    }

    void update(float deltaTime) {
        if (frameCount <= 1) return;
        animationTimer += deltaTime;
        if (animationTimer >= frameDuration) {
            animationTimer -= frameDuration;
            currentFrame = (currentFrame + 1) % frameCount;
        }
    }

    sf::Texture* getTexture() const { return texture; }
    sf::IntRect getCurrentFrame() const { return frames[currentFrame]; }
    void reset() { currentFrame = 0; animationTimer = 0.0f; }

private:
    sf::Texture* texture;
    sf::IntRect* frames;
    int frameCount;
    float frameDuration;
    int currentFrame;
    float animationTimer;
};