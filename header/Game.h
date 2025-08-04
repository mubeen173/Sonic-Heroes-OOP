#pragma once
#include "Character.h"
#include "JumpQueue.h"
#include "PositionQueue.h"
#include "Enemies.h"
#include "Collectable.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include "Menu.h"
#include "PauseMenu.h"
#include "Menu.cpp"  // Including implementation files directly
#include "PauseMenu.cpp"

class Game {
public:
    Game();
    ~Game();
    void run();
    void initializeLevel(int level);
    int getScore() const { return score; }
    void saveGame() {
        if (currentSaveSlot.empty()) {
            cout << "No save slot selected.\n";
            return;
        }
        string filename = "save_" + currentSaveSlot.substr(5) + ".txt"; // e.g., "save_1.txt"
        ofstream out(filename);
        if (!out.is_open()) {
            cout << "Failed to save game to " << filename << "\n";
            return;
        }

        float totalTime = initialTime.asSeconds() + gameTimerClock.getElapsedTime().asSeconds();
        out << currentLevel << "\n";
        out << totalTime << "\n";
        out << sharedHP << "\n";
        out << mainIndex << "\n";
        for (int i = 0; i < 3; ++i) {
            out << characters[i]->getPosX() << " " << characters[i]->getPosY() << "\n";
        }
        int aliveEnemies = 0;
        for (int i = 0; i < enemyCount; ++i) if (enemies[i]->isAlive()) aliveEnemies++;
        out << aliveEnemies << "\n";
        for (int i = 0; i < enemyCount; ++i) {
            if (enemies[i]->isAlive()) {
                out << enemies[i]->getType() << " " << enemies[i]->getPosX() << " " << enemies[i]->getPosY() << "\n";
            }
        }
        out << rows << " " << cols << "\n";
        for (int y = 0; y < rows; ++y) {
            for (int x = 0; x < cols; ++x) {
                out << mapData[y][x];
            }
            out << "\n";
        }
        out << score << "\n";
        out << collectableCount << "\n";
        for (int i = 0; i < collectableCount; ++i) {
            out << collectables[i]->getPosX() << " " << collectables[i]->getPosY() << " " << collectables[i]->getIsCollected() << "\n";
        }
        out << speedBoostTimer << " " << jumpBoostTimer << " " << invincibilityTimer << "\n";
        out << playerName << "\n"; // Save player name
        out.close();
        cout << "Game saved to " << filename << "\n";
    }

    void loadGame(const string& filename) {
        ifstream in(filename);
        if (!in.is_open()) {
            cout << "Failed to open save file.\n";
            return;
        }

        in >> currentLevel;

        float loadedTime;
        in >> loadedTime;
        initialTime = seconds(loadedTime);

        in >> sharedHP;

        in >> mainIndex;

        for (int i = 0; i < 3; ++i) {
            float posX, posY;
            in >> posX >> posY;
            characters[i]->setPosX(posX);
            characters[i]->setPosY(posY);
            characters[i]->setVelX(0.0f);
            characters[i]->setVelY(0.0f);
            characters[i]->setOnGround(true);
        }

        int aliveEnemies;
        in >> aliveEnemies;

        for (int i = 0; i < enemyCount; ++i) {
            delete enemies[i];
            enemies[i] = nullptr;
        }
        enemyCount = 0;

        for (int i = 0; i < aliveEnemies; ++i) {
            char type;
            float posX, posY;
            in >> type >> posX >> posY;
            float scale = 2.0f;
            switch (type) {
            case 'B':
                enemies[enemyCount] = new BatBrain(posX, posY, scale, batBrainIdleLeftTexture, batBrainIdleRightTexture, batBrainMoveLeftTexture, batBrainMoveRightTexture);
                break;
            case 'E':
                enemies[enemyCount] = new BeeBot(posX, posY, scale, beeBotIdleLeftTexture, beeBotIdleRightTexture, beeBotMoveLeftTexture, beeBotMoveRightTexture);
                break;
            case 'M':
                enemies[enemyCount] = new Motobug(posX, posY, scale, motobugIdleLeftTexture, motobugIdleRightTexture, motobugMoveLeftTexture, motobugMoveRightTexture);
                break;
            case 'C':
                enemies[enemyCount] = new CrabMeat(posX, posY, scale, crabMeatIdleLeftTexture, crabMeatIdleRightTexture, crabMeatMoveLeftTexture, crabMeatMoveRightTexture);
                break;
            case 'S':
                enemies[enemyCount] = new EggStinger(posX, posY, scale, eggStingerIdleLeftTexture, eggStingerIdleRightTexture, eggStingerMoveLeftTexture, eggStingerMoveRightTexture);
                break;
            default:
                continue;
            }
            enemyCount++;
        }

        int savedRows, savedCols;
        in >> savedRows >> savedCols;
        string line;

        if (savedRows != rows || savedCols != cols) {
            cout << "Map size mismatch. Loading new map.\n";
            if (mapData) {
                for (int i = 0; i < rows; ++i) {
                    delete[] mapData[i];
                }
                delete[] mapData;
            }
            rows = savedRows;
            cols = savedCols;
            mapData = new char* [rows];
            for (int i = 0; i < rows; ++i) {
                mapData[i] = new char[cols];
            }
        }
        in.ignore(numeric_limits<streamsize>::max(), '\n');
        for (int y = 0; y < rows; ++y) {
            getline(in, line);
            if (line.length() < static_cast<size_t>(cols)) {
                cout << "Invalid map data.\n";
                return;
            }
            for (int x = 0; x < cols; ++x) {
                mapData[y][x] = line[x];
            }
        }
        level = const_cast<const char**>(mapData);
        levelWidth = cols * CELL_SIZE;
        levelHeight = rows * CELL_SIZE;

        in >> score;
        int loadedCollectableCount;
        in >> loadedCollectableCount;
        for (int i = 0; i < collectableCount; ++i) {
            delete collectables[i];
            collectables[i] = nullptr;
        }
        collectableCount = 0;
        for (int i = 0; i < loadedCollectableCount && collectableCount < MAX_COLLECTABLES; ++i) {
            float posX, posY;
            bool isCollected;
            in >> posX >> posY >> isCollected;
            collectables[collectableCount] = new Ring(posX, posY, 32.0f, 32.0f, ringTexture);
            if (isCollected) collectables[collectableCount]->onCollect(*characters[mainIndex]);
            collectableCount++;
        }
        in >> speedBoostTimer >> jumpBoostTimer >> invincibilityTimer;
        in >> playerName; // Load player name
        updateBoosts(0.0f);
        updateDrawOrder();
        in.close();
    }

private:
    Texture wallTexture, backgroundTexture[3], blockTexture, platformTexture, crystalTexture, block3Texture, spikeTexture, pitTexture, block4Texture;
    Texture grassTexture;
    Texture sonicIdleLeftTexture, sonicIdleRightTexture;
    Texture sonicRunLeftTexture, sonicRunRightTexture;
    Texture sonicJumpTexture;
    Texture sonicPushLeftTexture, sonicPushRightTexture;
    Texture sonicEdgeLeftTexture, sonicEdgeRightTexture;
    Texture knucklesIdleLeftTexture, knucklesIdleRightTexture;
    Texture knucklesRunLeftTexture, knucklesRunRightTexture;
    Texture knucklesJumpLeftTexture, knucklesJumpRightTexture;
    Texture knucklesPushLeftTexture, knucklesPushRightTexture;
    Texture knucklesEdgeLeftTexture, knucklesEdgeRightTexture;
    Texture knucklesPunchLeftTexture, knucklesPunchRightTexture;
    Texture tailsIdleLeftTexture, tailsIdleRightTexture;
    Texture tailsRunLeftTexture, tailsRunRightTexture;
    Texture tailsJumpTexture;
    Texture tailsFlyLeftTexture, tailsFlyRightTexture;
    Texture tailsPushLeftTexture, tailsPushRightTexture;
    Texture tailsEdgeLeftTexture, tailsEdgeRightTexture;
    Texture batBrainIdleLeftTexture, batBrainIdleRightTexture;
    Texture batBrainMoveLeftTexture, batBrainMoveRightTexture;
    Texture beeBotIdleLeftTexture, beeBotIdleRightTexture;
    Texture beeBotMoveLeftTexture, beeBotMoveRightTexture;
    Texture motobugIdleLeftTexture, motobugIdleRightTexture;
    Texture motobugMoveLeftTexture, motobugMoveRightTexture;
    Texture crabMeatIdleLeftTexture, crabMeatIdleRightTexture;
    Texture crabMeatMoveLeftTexture, crabMeatMoveRightTexture;
    Texture eggStingerIdleLeftTexture, eggStingerIdleRightTexture;
    Texture eggStingerMoveLeftTexture, eggStingerMoveRightTexture;
    Texture ringTexture, extraLifeTexture, speedBoostTexture, jumpBoostTexture, invincibilityBoostTexture;

    Sprite backgroundSprite, blockSprite, platformSprite, crystalSprite, block3Sprite, block4Sprite, spikeSprite, pitSprite;
    Sprite grassSprite;
    Font font;
    Text timerText;
    Text gameTimerText;
    Text scoreText;
    Text hpText;

    Clock gameTimerClock;
    float timerX, timerY;
    Music backgroundMusic;
    int sharedHP;
    float invincibilityTimer;
    float speedBoostTimer;
    float jumpBoostTimer;
    int currentLevel;
    const char** level;
    char** mapData;
    int rows, cols;
    float startX, startY;
    float levelWidth, levelHeight;
    Time initialTime;

    Character* characters[3];
    int mainIndex;
    float offScreenTimers[3];
    int drawOrder[3];
    float maxSpeed;

    JumpQueue jumpQueues[3];
    PositionQueue positionQueue;
    const int delayFrames;

    static const int MAX_ENEMIES = 50;
    Enemy* enemies[MAX_ENEMIES];
    int enemyCount;

    PauseMenu pauseMenu;
    bool isPaused;

    string currentSaveSlot;
    static const int MAX_COLLECTABLES = 100;
    Collectable* collectables[MAX_COLLECTABLES];
    int collectableCount;
    int score;
    string playerName; // Added to store player name

    void updateDrawOrder();
    void loadMap(const string& filename);
    void loadEnemies(const string& filename);
    void updateEnemies(float deltaTime, float gravity, float terminalVelocity);
    void drawEnemies(RenderWindow& window, const RenderStates& states);
    void checkCollisions();
    void drawLevel(RenderWindow& window, Sprite& wallSprite, const RenderStates& states);
    void checkCharacterRespawn(float cameraX, float cameraY);
    void checkHazardCollisions(int& sharedHP, float& invincibilityTimer);
    void respawnCharacter(int charIndex, bool isMain);
    void handlePause(RenderWindow& window);
    void loadCollectables(const string& filename);
    void updateCollectables();
    void drawCollectables(RenderWindow& window, const RenderStates& states);
    void applyBoost(Character* character, int type, float duration);
    void updateBoosts(float deltaTime);
};

// Implementation section
Game::Game() : jumpQueues{ JumpQueue(), JumpQueue(), JumpQueue() }, positionQueue(100), delayFrames(30), enemyCount(0), pauseMenu(font), isPaused(false), sharedHP(3), invincibilityTimer(0.0f), speedBoostTimer(0.0f), jumpBoostTimer(0.0f), currentLevel(1), initialTime(Time::Zero), currentSaveSlot(""), collectableCount(0), score(0), playerName("Player") {
    for (int i = 0; i < MAX_COLLECTABLES; ++i) collectables[i] = nullptr;
    if (!wallTexture.loadFromFile("Data/brick1.png") ||
        !backgroundTexture[0].loadFromFile("Data/background_level1.png") ||
        !backgroundTexture[1].loadFromFile("Data/background_level2.png") ||
        !backgroundTexture[2].loadFromFile("Data/background_level3.png") ||
        !blockTexture.loadFromFile("Data/block.png") ||
        !platformTexture.loadFromFile("Data/platform.png") ||
        !crystalTexture.loadFromFile("Data/crystal.png") ||
        !block3Texture.loadFromFile("Data/block3.png") ||
        !spikeTexture.loadFromFile("Data/spik.png") ||
        !pitTexture.loadFromFile("Data/pit.png") ||
        !grassTexture.loadFromFile("Data/grass.png") ||
        !block4Texture.loadFromFile("Data/block4.png") ||
        !ringTexture.loadFromFile("Data/ring.png") ||
        !extraLifeTexture.loadFromFile("Data/extralife.png") ||
        !speedBoostTexture.loadFromFile("Data/speedboost.png") ||
        !jumpBoostTexture.loadFromFile("Data/jumpboost.png") ||
        !invincibilityBoostTexture.loadFromFile("Data/invincibilityboost.png") ||
        !font.loadFromFile("Data/arial.ttf") ||
        !backgroundMusic.openFromFile("Data/labrynth.ogg")) {
        cout << "Failed to load assets.\n";
        return;
    }

    if (!sonicIdleLeftTexture.loadFromFile("Data/0left_still.png") ||
        !sonicIdleRightTexture.loadFromFile("Data/0right_still.png") ||
        !sonicRunLeftTexture.loadFromFile("Data/sonic_runl.png") ||
        !sonicRunRightTexture.loadFromFile("Data/sonic_runr.png") ||
        !sonicJumpTexture.loadFromFile("Data/sonic_jump.png") ||
        !sonicPushLeftTexture.loadFromFile("Data/sonic_pushl.png") ||
        !sonicPushRightTexture.loadFromFile("Data/sonic_pushr.png") ||
        !sonicEdgeLeftTexture.loadFromFile("Data/sonic_edgel.png") ||
        !sonicEdgeRightTexture.loadFromFile("Data/sonic_edger.png") ||
        !knucklesIdleLeftTexture.loadFromFile("Data/knuckles_idle_left.png") ||
        !knucklesIdleRightTexture.loadFromFile("Data/knuckles_idle_right.png") ||
        !knucklesRunLeftTexture.loadFromFile("Data/knuckles_run_left.png") ||
        !knucklesRunRightTexture.loadFromFile("Data/knuckles_run_right.png") ||
        !knucklesJumpLeftTexture.loadFromFile("Data/knuckles_jump_left.png") ||
        !knucklesJumpRightTexture.loadFromFile("Data/knuckles_jump_right.png") ||
        !knucklesPushLeftTexture.loadFromFile("Data/knuckles_push_left.png") ||
        !knucklesPushRightTexture.loadFromFile("Data/knuckles_push_right.png") ||
        !knucklesEdgeLeftTexture.loadFromFile("Data/knuckles_edge_left.png") ||
        !knucklesEdgeRightTexture.loadFromFile("Data/knuckles_edge_right.png") ||
        !knucklesPunchLeftTexture.loadFromFile("Data/knuckles_punch_left.png") ||
        !knucklesPunchRightTexture.loadFromFile("Data/knuckles_punch_right.png") ||
        !tailsIdleLeftTexture.loadFromFile("Data/tails_idle_left.png") ||
        !tailsFlyLeftTexture.loadFromFile("Data/tails_fly_left.png") ||
        !tailsFlyRightTexture.loadFromFile("Data/tails_fly_right.png") ||
        !tailsIdleRightTexture.loadFromFile("Data/tails_idle_right.png") ||
        !tailsRunLeftTexture.loadFromFile("Data/tails_run_left.png") ||
        !tailsRunRightTexture.loadFromFile("Data/tails_run_right.png") ||
        !tailsJumpTexture.loadFromFile("Data/tails_jump.png") ||
        !tailsPushLeftTexture.loadFromFile("Data/tails_push_left.png") ||
        !tailsPushRightTexture.loadFromFile("Data/tails_push_right.png") ||
        !tailsEdgeLeftTexture.loadFromFile("Data/tails_edge_left.png") ||
        !tailsEdgeRightTexture.loadFromFile("Data/tails_edge_right.png")) {
        cout << "Failed to load character textures.\n";
        return;
    }

    if (!batBrainIdleLeftTexture.loadFromFile("Data/batbrain_idle_left.png") ||
        !batBrainIdleRightTexture.loadFromFile("Data/batbrain_idle_right.png") ||
        !batBrainMoveLeftTexture.loadFromFile("Data/batbrain_move_left.png") ||
        !batBrainMoveRightTexture.loadFromFile("Data/batbrain_move_right.png") ||
        !beeBotIdleLeftTexture.loadFromFile("Data/beebot_idle_left.png") ||
        !beeBotIdleRightTexture.loadFromFile("Data/beebot_idle_right.png") ||
        !beeBotMoveLeftTexture.loadFromFile("Data/beebot_move_left.png") ||
        !beeBotMoveRightTexture.loadFromFile("Data/beebot_move_right.png") ||
        !motobugIdleLeftTexture.loadFromFile("Data/motobug_idle_left.png") ||
        !motobugIdleRightTexture.loadFromFile("Data/motobug_idle_right.png") ||
        !motobugMoveLeftTexture.loadFromFile("Data/motobug_move_left.png") ||
        !motobugMoveRightTexture.loadFromFile("Data/motobug_move_right.png") ||
        !crabMeatIdleLeftTexture.loadFromFile("Data/crabmeat_idle_left.png") ||
        !crabMeatIdleRightTexture.loadFromFile("Data/crabmeat_idle_right.png") ||
        !crabMeatMoveLeftTexture.loadFromFile("Data/crabmeat_move_left.png") ||
        !crabMeatMoveRightTexture.loadFromFile("Data/crabmeat_move_right.png") ||
        !eggStingerIdleLeftTexture.loadFromFile("Data/eggstinger_idle_left.png") ||
        !eggStingerIdleRightTexture.loadFromFile("Data/eggstinger_idle_right.png") ||
        !eggStingerMoveLeftTexture.loadFromFile("Data/eggstinger_move_left.png") ||
        !eggStingerMoveRightTexture.loadFromFile("Data/eggstinger_move_right.png")) {
        cout << "Failed to load enemy textures.\n";
        return;
    }

    for (int i = 0; i < MAX_ENEMIES; ++i) {
        enemies[i] = nullptr;
    }

    backgroundSprite.setTexture(backgroundTexture[0]);
    backgroundSprite.setScale(1.4f, 1.4f);
    blockSprite.setTexture(blockTexture);
    platformSprite.setTexture(platformTexture);
    crystalSprite.setTexture(crystalTexture);
    block3Sprite.setTexture(block3Texture);
    block4Sprite.setTexture(block4Texture);
    spikeSprite.setTexture(spikeTexture);
    pitSprite.setTexture(pitTexture);
    grassSprite.setTexture(grassTexture);
    backgroundMusic.setLoop(true);
    backgroundMusic.setVolume(30);
    backgroundMusic.play();

    timerText.setFont(font);
    timerText.setCharacterSize(20);
    timerText.setFillColor(Color::White);
    timerText.setPosition(10, 10);

    gameTimerText.setFont(font);
    gameTimerText.setCharacterSize(20);
    gameTimerText.setFillColor(Color::White);
    timerX = 10;
    timerY = 40;
    gameTimerText.setPosition(timerX, timerY);

    scoreText.setFont(font);
    scoreText.setCharacterSize(30);
    scoreText.setFillColor(Color::Yellow);
    scoreText.setPosition(10, 70);

    hpText.setFont(font);
    hpText.setCharacterSize(20);
    hpText.setFillColor(Color::White);
    hpText.setPosition(10, 100);

    loadMap("Data/map.txt");
    if (!level || rows <= 0 || cols <= 0) {
        cout << "Failed to load valid level data.\n";
        return;
    }
    loadEnemies("Data/enemies.txt");
    loadCollectables("Data/collectables.txt");

    levelWidth = cols * CELL_SIZE;
    levelHeight = rows * CELL_SIZE;

    float sonicScale = 2.8f;
    float knucklesScale = 2.8f;
    float tailsScale = 2.8f;

    startX = CELL_SIZE * 1.0f;
    startY = CELL_SIZE * 11.1f;
    maxSpeed = 14.0f;

    characters[0] = new Sonic(startX, startY, 40, 40, maxSpeed, sonicScale,
        sonicIdleLeftTexture, sonicIdleRightTexture,
        sonicRunLeftTexture, sonicRunRightTexture, sonicJumpTexture,
        sonicPushLeftTexture, sonicPushRightTexture,
        sonicEdgeLeftTexture, sonicEdgeRightTexture);

    characters[1] = new Knuckles(startX, startY, 40, 40, maxSpeed, knucklesScale,
        knucklesIdleLeftTexture, knucklesIdleRightTexture,
        knucklesRunLeftTexture, knucklesRunRightTexture,
        knucklesJumpLeftTexture, knucklesJumpRightTexture,
        knucklesPushLeftTexture, knucklesPushRightTexture,
        knucklesEdgeLeftTexture, knucklesEdgeRightTexture,
        knucklesPunchLeftTexture, knucklesPunchRightTexture);

    characters[2] = new Tails(startX, startY, 40, 40, maxSpeed, tailsScale,
        tailsIdleLeftTexture, tailsIdleRightTexture,
        tailsRunLeftTexture, tailsRunRightTexture, tailsJumpTexture,
        tailsPushLeftTexture, tailsPushRightTexture,
        tailsEdgeLeftTexture, tailsEdgeRightTexture,
        tailsFlyLeftTexture, tailsFlyRightTexture);

    mainIndex = 0;

    for (int i = 0; i < 3; ++i) {
        characters[i]->currentMaxSpeed = characters[i]->getBaseMaxSpeed();
    }
    characters[mainIndex]->currentMaxSpeed = characters[mainIndex]->getBaseMaxSpeed() * 1.2f;

    updateDrawOrder();
}

Game::~Game() {
    for (int i = 0; i < 3; ++i) delete characters[i];
    for (int i = 0; i < rows; ++i) delete[] mapData[i];
    delete[] mapData;
    for (int i = 0; i < enemyCount; ++i) delete enemies[i];
    for (int i = 0; i < collectableCount; ++i) delete collectables[i];
}

void Game::checkCharacterRespawn(float cameraX, float cameraY) {
    for (int i = 0; i < 3; ++i) {
        float charX = characters[i]->getPosX();
        float charY = characters[i]->getPosY();
        bool isMain = (i == mainIndex);

        if (charY > levelHeight + 100.0f) {
            charX = isMain ? startX : characters[mainIndex]->getPosX() - 200.0f;
            charY = isMain ? startY : characters[mainIndex]->getPosY();
            characters[i]->setPosX(charX);
            characters[i]->setPosY(charY);
            characters[i]->setVelX(0.0f);
            characters[i]->setVelY(0.0f);
            characters[i]->setOnGround(true);
            offScreenTimers[i] = 0.0f;
            cout << "Character " << i << " respawned due to falling below map.\n";
            continue;
        }

        if (!isMain) {
            float screenLeft = cameraX;
            float screenRight = cameraX + SCREEN_X;
            float screenTop = cameraY;
            float screenBottom = cameraY + SCREEN_Y;

            bool isOnScreen = (charX >= screenLeft && charX <= screenRight &&
                charY >= screenTop && charY <= screenBottom);

            if (!isOnScreen) {
                offScreenTimers[i] += 1.0f / 60.0f;
                if (offScreenTimers[i] >= 4.0f) {
                    characters[i]->setPosX(characters[mainIndex]->getPosX() - 1200.0f);
                    characters[i]->setPosY(characters[mainIndex]->getPosY());
                    characters[i]->setVelX(0.0f);
                    characters[i]->setVelY(0.0f);
                    characters[i]->setOnGround(true);
                    offScreenTimers[i] = 0.0f;
                    cout << "Follower " << i << " respawned due to being stuck off-screen.\n";
                }
            }
            else {
                offScreenTimers[i] = 0.0f;
            }
        }
    }
}

void Game::respawnCharacter(int charIndex, bool isMain) {
    float charX = characters[charIndex]->getPosX();
    float charY = characters[charIndex]->getPosY();
    int col = static_cast<int>(charX / CELL_SIZE);
    int row = static_cast<int>(charY / CELL_SIZE);

    float newX = charX;
    float newY = charY;
    bool found = false;

    for (int y = row - 1; y >= 0 && !found; --y) {
        for (int x = col - 2; x <= col + 2; ++x) {
            if (x >= 0 && x < cols && y >= 0 && y < rows) {
                if (mapData[y][x] == 'b' || mapData[y][x] == 'p') {
                    newX = x * CELL_SIZE + CELL_SIZE / 2.0f;
                    newY = y * CELL_SIZE - characters[charIndex]->getHeight() - 64.0f;
                    found = true;
                    break;
                }
            }
        }
    }

    if (!found) {
        newX = isMain ? startX : characters[mainIndex]->getPosX() - 200.0f;
        newY = isMain ? startY : characters[mainIndex]->getPosY();
    }

    characters[charIndex]->setPosX(newX);
    characters[charIndex]->setPosY(newY);
    characters[charIndex]->setVelX(0.0f);
    characters[charIndex]->setVelY(0.0f);
    characters[charIndex]->setOnGround(true);
}

void Game::checkHazardCollisions(int& sharedHP, float& invincibilityTimer) {
    if (invincibilityTimer > 0.0f) {
        invincibilityTimer -= 1.0f / 60.0f;
        return;
    }

    for (int i = 0; i < 3; ++i) {
        float charX = characters[i]->getPosX();
        float charY = characters[i]->getPosY();
        float charWidth = characters[i]->getWidth();
        float charHeight = characters[i]->getHeight();
        bool isMain = (i == mainIndex);

        int leftCol = static_cast<int>(charX / CELL_SIZE);
        int rightCol = static_cast<int>((charX + charWidth) / CELL_SIZE);
        int topRow = static_cast<int>(charY / CELL_SIZE);
        int botRow = static_cast<int>((charY + charHeight) / CELL_SIZE);

        for (int y = topRow; y <= botRow; ++y) {
            for (int x = leftCol; x <= rightCol; ++x) {
                if (x >= 0 && x < cols && y >= 0 && y < rows) {
                    char tile = level[y][x];
                    if (tile == 'u') {
                        if (isMain) {
                            cout << "Fall on spike\n";
                            this->sharedHP--;
                            this->invincibilityTimer = 1.0f;
                            cout << "Player HP: " << sharedHP << "\n";
                            if (this->sharedHP <= 0) {
                                cout << "Game Over!\n";
                            }
                        }
                        else {
                            cout << "Follower collides on spike\n";
                        }
                        respawnCharacter(i, isMain);
                        return;
                    }
                    else if (tile == 'x') {
                        if (isMain) {
                            cout << "Game Over\n";
                            this->sharedHP = 0;
                        }
                        else {
                            cout << "Follower falls in pit\n";
                        }
                        respawnCharacter(i, isMain);
                        return;
                    }
                }
            }
        }
    }
}

void Game::run() {
    RenderWindow window(VideoMode(SCREEN_X, SCREEN_Y), "Sonic Platformer", Style::Close);
    window.setFramerateLimit(60);
    Menu menu(font, backgroundMusic);
    int action = menu.run(window);

    if (action == Menu::EXIT) {
        window.close();
        return;
    }

    if (action == Menu::START_GAME) {
        currentSaveSlot = "Slot 1";
        int selectedLevel = menu.getSelectedLevel();
        initializeLevel(selectedLevel);
        char playerNameTemp[32];
        menu.getPlayerName(window, playerNameTemp); // Get player name again if needed
        playerName = string(playerNameTemp);
    }
    else if (action == Menu::LOAD_GAME) {
        currentSaveSlot = menu.getSelectedSaveSlot();
        string filename = "save_" + currentSaveSlot.substr(5) + ".txt";
        loadGame(filename);
    }
    else {
        return;
    }

    Sprite wallSprite(wallTexture);
    const float gravity = 3.0f;
    const float terminalVel = 19.0f;
    const float jumpStrength = -26.0f;
    Clock clock;

    while (window.isOpen()) {
        Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == Event::Closed) {
                menu.updateScoreboard(playerName.c_str(), score);
                window.close();
                return;
            }
            if (ev.key.code == Keyboard::Escape) {
                isPaused = !isPaused;
                if (isPaused) {
                    int pauseAction = pauseMenu.run(window);
                    if (pauseAction == 1) {
                        menu.updateScoreboard(playerName.c_str(), score);
                        window.close();
                        return;
                    }
                    else if (pauseAction == 2) {
                        saveGame();
                        isPaused = false;
                    }
                    else {
                        isPaused = false;
                    }
                }
            }
            else if (!isPaused && ev.type == Event::KeyPressed && ev.key.code == Keyboard::A) {
                for (int idx = 0; idx < 3; ++idx) {
                    characters[idx]->currentMaxSpeed = characters[idx]->getBaseMaxSpeed();
                }
                mainIndex = (mainIndex + 1) % 3;
                characters[mainIndex]->currentMaxSpeed = characters[mainIndex]->getBaseMaxSpeed() * 1.2f;
                updateDrawOrder();
            }
        }

        float deltaTime = clock.restart().asSeconds();

        positionQueue.enqueue(characters[mainIndex]->getPosX(), characters[mainIndex]->getPosY());
        while (positionQueue.size > delayFrames) positionQueue.dequeue();

        characters[mainIndex]->update(gravity, terminalVel, jumpStrength, level, rows, cols, deltaTime);

        if (characters[mainIndex]->justJumped) {
            float xPos = characters[mainIndex]->getPosX();
            for (int i = 0; i < 3; ++i) {
                if (i != mainIndex) jumpQueues[i].enqueue(xPos);
            }
        }
        for (int i = 0; i < 3; ++i) {
            if (Knuckles* knuckles = dynamic_cast<Knuckles*>(characters[i])) {
                for (int j = 0; j < knuckles->numBlocksToBreak; ++j) {
                    int x = knuckles->blocksToBreak[j].x;
                    int y = knuckles->blocksToBreak[j].y;
                    if (x >= 0 && x < cols && y >= 0 && y < rows) {
                        mapData[y][x] = ' ';
                    }
                }
                knuckles->numBlocksToBreak = 0;
            }
        }

        for (int i = 0; i < 3; ++i) {
            if (i != mainIndex) {
                PositionQueue::Position targetPos = positionQueue.isEmpty() ?
                    PositionQueue::Position{ characters[mainIndex]->getPosX(), characters[mainIndex]->getPosY() } :
                    positionQueue.peek();
                characters[i]->updateFollower(gravity, terminalVel, jumpStrength, level, rows, cols, deltaTime,
                    targetPos.x, targetPos.y, jumpQueues[i]);
            }
        }

        for (int i = 0; i < 3; ++i) characters[i]->jumpedWhileStillThisFrame = false;

        updateEnemies(deltaTime, gravity, terminalVel);
        checkCollisions();
        checkHazardCollisions(sharedHP, invincibilityTimer);

        updateCollectables();
        updateBoosts(deltaTime);

        if (sharedHP <= 0) {
            menu.updateScoreboard(playerName.c_str(), score);
            action = menu.run(window);
            if (action == Menu::EXIT) {
                window.close();
                return;
            }
            else if (action == Menu::START_GAME) {
                currentSaveSlot = "Slot 1";
                int selectedLevel = menu.getSelectedLevel();
                initializeLevel(selectedLevel);
                char playerNameTemp[32];
                menu.getPlayerName(window, playerNameTemp);
                playerName = string(playerNameTemp);
            }
            else if (action == Menu::LOAD_GAME) {
                currentSaveSlot = menu.getSelectedSaveSlot();
                string filename = "save_" + currentSaveSlot.substr(5) + ".txt";
                loadGame(filename);
            }
            continue;
        }

        for (int i = 0; i < 3; ++i) {
            if (Tails* tailsPtr = dynamic_cast<Tails*>(characters[i])) {
                timerText.setString(tailsPtr->getTimerText());
                break;
            }
        }

        int elapsedSeconds = static_cast<int>((initialTime + gameTimerClock.getElapsedTime()).asSeconds());
        gameTimerText.setString("Time: " + to_string(elapsedSeconds) + "s");
        scoreText.setString("Score: " + to_string(score));
        hpText.setString("HP: " + to_string(sharedHP));

        float centerX = characters[mainIndex]->getPosX();
        float centerY = characters[mainIndex]->getPosY();
        float idealCameraX = centerX - SCREEN_X / 2.0f;
        float idealCameraY = centerY - SCREEN_Y / 2.0f;

        float cameraX, cameraY;
        if (levelWidth < SCREEN_X) cameraX = -(SCREEN_X - levelWidth) / 2.0f;
        else cameraX = max(0.0f, min(idealCameraX, levelWidth - SCREEN_X));
        if (levelHeight < SCREEN_Y) cameraY = -(SCREEN_Y - levelHeight) / 2.0f;
        else cameraY = max(0.0f, min(idealCameraY, levelHeight - SCREEN_Y));

        checkCharacterRespawn(cameraX, cameraY);

        RenderStates states;
        states.transform.translate(-cameraX, -cameraY);

        window.clear();
        window.draw(backgroundSprite);
        drawLevel(window, wallSprite, states);
        for (int i = 0; i < 3; ++i) characters[drawOrder[i]]->draw(window, states);
        drawEnemies(window, states);
        drawCollectables(window, states);
        window.draw(timerText);
        window.draw(gameTimerText);
        window.draw(scoreText);
        window.draw(hpText);
        window.display();
    }
}

void Game::initializeLevel(int level) {
    currentLevel = level;
    string mapFile = "Data/map_" + to_string(level) + ".txt";
    string enemiesFile = "Data/enemies_" + to_string(level) + ".txt";
    string collectablesFile = "Data/collectables_" + to_string(level) + ".txt";
    if (mapData) {
        for (int i = 0; i < rows; ++i) delete[] mapData[i];
        delete[] mapData;
    }
    for (int i = 0; i < enemyCount; ++i) delete enemies[i];
    enemyCount = 0;
    for (int i = 0; i < collectableCount; ++i) delete collectables[i];
    for (int i = 0; i < MAX_COLLECTABLES; ++i) collectables[i] = nullptr;
    collectableCount = 0;

    loadMap(mapFile);
    if (!level || rows <= 0 || cols <= 0) {
        cout << "Failed to load valid level data for level " << level << ".\n";
        loadMap("Data/map.txt");
        loadEnemies("Data/enemies.txt");
        loadCollectables("Data/collectables.txt");
        backgroundSprite.setTexture(backgroundTexture[0]);
        return;
    }
    loadEnemies(enemiesFile);
    loadCollectables(collectablesFile);

    if (level >= 1 && level <= 3) {
        backgroundSprite.setTexture(backgroundTexture[level - 1]);
        backgroundSprite.setScale(1.4f, 1.4f);
    }
    else {
        backgroundSprite.setTexture(backgroundTexture[0]);
        backgroundSprite.setScale(1.4f, 1.4f);
    }

    levelWidth = cols * CELL_SIZE;
    levelHeight = rows * CELL_SIZE;

    startX = CELL_SIZE * 1.0f;
    startY = CELL_SIZE * 11.1f;
    for (int i = 0; i < 3; ++i) {
        characters[i]->setPosX(startX);
        characters[i]->setPosY(startY);
        characters[i]->setVelX(0.0f);
        characters[i]->setVelY(0.0f);
        characters[i]->setOnGround(true);
    }
    score = 0;
    speedBoostTimer = 0.0f;
    jumpBoostTimer = 0.0f;
    invincibilityTimer = 0.0f;
}

void Game::updateDrawOrder() {
    int idx = 0;
    for (int i = 0; i < 3; ++i) {
        if (i != mainIndex) drawOrder[idx++] = i;
    }
    drawOrder[2] = mainIndex;
}

void Game::loadMap(const string& filename) {
    ifstream in(filename);
    if (!in.is_open()) {
        cout << "MAP ERROR: Could not open " << filename << "\n";
        return;
    }

    in >> rows >> cols;
    if (rows <= 0 || cols <= 0) {
        cout << "MAP ERROR: Invalid dimensions (" << rows << "x" << cols << ")\n";
        in.close();
        return;
    }

    mapData = new char* [rows];
    for (int i = 0; i < rows; ++i) {
        mapData[i] = new char[cols];
        for (int j = 0; j < cols; ++j) mapData[i][j] = ' ';
    }

    in.ignore(numeric_limits<streamsize>::max(), '\n');
    string line;
    int y = 0;
    while (y < rows && getline(in, line)) {
        if (line.empty()) continue;
        for (int x = 0; x < cols && x < line.length(); ++x) {
            mapData[y][x] = line[x];
        }
        y++;
    }

    in.close();
    level = const_cast<const char**>(mapData);
}

void Game::loadEnemies(const string& filename) {
    ifstream in(filename);
    if (!in.is_open()) {
        cout << "ENEMY ERROR: Could not open " << filename << "\n";
        return;
    }

    enemyCount = 0;
    char enemyType;
    float x, y;

    while (enemyCount < MAX_ENEMIES && in >> enemyType >> x >> y) {
        in.ignore(numeric_limits<streamsize>::max(), '\n');

        float scale = 2.0f;
        switch (enemyType) {
        case 'B':
            enemies[enemyCount] = new BatBrain(
                x * CELL_SIZE, y * CELL_SIZE, scale,
                batBrainIdleLeftTexture, batBrainIdleRightTexture,
                batBrainMoveLeftTexture, batBrainMoveRightTexture);
            break;
        case 'E':
            enemies[enemyCount] = new BeeBot(
                x * CELL_SIZE, y * CELL_SIZE, scale,
                beeBotIdleLeftTexture, beeBotIdleRightTexture,
                beeBotMoveLeftTexture, beeBotMoveRightTexture);
            break;
        case 'M':
            enemies[enemyCount] = new Motobug(
                x * CELL_SIZE, y * CELL_SIZE, scale,
                motobugIdleLeftTexture, motobugIdleRightTexture,
                motobugMoveLeftTexture, motobugMoveRightTexture);
            break;
        case 'C':
            enemies[enemyCount] = new CrabMeat(
                x * CELL_SIZE, y * CELL_SIZE, scale,
                crabMeatIdleLeftTexture, crabMeatIdleRightTexture,
                crabMeatMoveLeftTexture, crabMeatMoveRightTexture);
            break;
        case 'S':
            enemies[enemyCount] = new EggStinger(
                x * CELL_SIZE, y * CELL_SIZE, scale,
                eggStingerIdleLeftTexture, eggStingerIdleRightTexture,
                eggStingerMoveLeftTexture, eggStingerMoveRightTexture);
            break;
        default:
            continue;
        }
        enemyCount++;
    }

    in.close();
    cout << "Loaded " << enemyCount << " enemies.\n";
}

void Game::updateEnemies(float deltaTime, float gravity, float terminalVelocity) {
    float playerX = characters[mainIndex]->getPosX();
    float playerY = characters[mainIndex]->getPosY();
    bool playerInBallForm = (characters[mainIndex]->getCurrentState() == Character::Jumping);

    for (int i = 0; i < enemyCount; ++i) {
        if (enemies[i] && enemies[i]->isAlive()) {
            enemies[i]->update(gravity, terminalVelocity, level, rows, cols, deltaTime,
                playerX, playerY, playerInBallForm);
        }
    }
}

void Game::drawEnemies(RenderWindow& window, const RenderStates& states) {
    for (int i = 0; i < enemyCount; ++i) {
        if (enemies[i] && enemies[i]->isAlive()) {
            enemies[i]->draw(window, states);
        }
    }
}

void Game::checkCollisions() {
    static float invincibilityTimers[3] = { 0.0f, 0.0f, 0.0f };

    for (int i = 0; i < 3; ++i) {
        if (invincibilityTimers[i] > 0.0f) {
            invincibilityTimers[i] -= 1.0f / 60.0f;
        }
    }

    for (int i = 0; i < 3; ++i) {
        float charX = characters[i]->getPosX();
        float charY = characters[i]->getPosY();
        float charWidth = characters[i]->getWidth();
        float charHeight = characters[i]->getHeight();
        bool isMain = (i == mainIndex);
        bool inBallForm = (characters[i]->getCurrentState() == Character::Jumping);

        for (int j = 0; j < enemyCount; ++j) {
            if (!enemies[j] || !enemies[j]->isAlive()) continue;

            float enemyX = enemies[j]->getPosX();
            float enemyY = enemies[j]->getPosY();
            float enemyWidth = enemies[j]->getEnemyWidth();
            float enemyHeight = enemies[j]->getEnemyHeight();

            bool collision = (charX < enemyX + enemyWidth &&
                charX + charWidth > enemyX &&
                charY < enemyY + enemyHeight &&
                charY + charHeight > enemyY);

            if (collision) {
                if (inBallForm && isMain) {
                    if (enemies[j]->takeDamage(1, true)) {
                        score += 10; // Add 10 points for damaging enemy
                        cout << "Enemy damaged! Score: " << score << "\n";
                        if (!enemies[j]->isAlive()) {
                            cout << "Enemy defeated!\n";
                        }
                    }
                    continue;
                }

                float mainX = characters[mainIndex]->getPosX();
                float mainY = characters[mainIndex]->getPosY();
                bool mainFacingRight = characters[mainIndex]->isFacingRight();
                float respawnX = mainX + (mainFacingRight ? -32.0f : 32.0f);
                float respawnY = mainY;
                characters[i]->setPosX(respawnX);
                characters[i]->setPosY(respawnY);
                characters[i]->setVelX(0.0f);
                characters[i]->setVelY(0.0f);
                characters[i]->setOnGround(true);
                cout << (isMain ? "Main character" : "Follower") << " respawned at (" << respawnX << ", " << respawnY << ") due to enemy collision.\n";

                if (isMain && invincibilityTimers[i] <= 0.0f) {
                    sharedHP--;
                    invincibilityTimers[i] = 2.0f;
                    cout << "Player HP: " << sharedHP << "\n";
                    if (sharedHP <= 0) {
                        cout << "Game Over!\n";
                    }
                }
            }
        }
    }
}

void Game::drawLevel(RenderWindow& window, Sprite& wallSprite, const RenderStates& states) {
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            char c = level[y][x];
            if (c == 'w' || c == 'f' || c == 'r') {
                wallSprite.setPosition(x * CELL_SIZE, y * CELL_SIZE);
                window.draw(wallSprite, states);
            }
            else if (c == 'b') {
                blockSprite.setPosition(x * CELL_SIZE, y * CELL_SIZE);
                window.draw(blockSprite, states);
            }
            else if (c == 'p') {
                platformSprite.setPosition(x * CELL_SIZE, y * CELL_SIZE);
                window.draw(platformSprite, states);
            }
            else if (c == 'c') {
                crystalSprite.setPosition(x * CELL_SIZE, y * CELL_SIZE);
                window.draw(crystalSprite, states);
            }
            else if (c == 'l') {
                block3Sprite.setPosition(x * CELL_SIZE, y * CELL_SIZE);
                window.draw(block3Sprite, states);
            }
            else if (c == 'u') {
                spikeSprite.setPosition(x * CELL_SIZE, y * CELL_SIZE);
                window.draw(spikeSprite, states);
            }
            else if (c == 'x') {
                pitSprite.setPosition(x * CELL_SIZE, y * CELL_SIZE);
                window.draw(pitSprite, states);
            }
            else if (c == 'g') {
                grassSprite.setPosition(x * CELL_SIZE, y * CELL_SIZE);
                window.draw(grassSprite, states);
            }
            else if (c == 'N') {
                block4Sprite.setPosition(x * CELL_SIZE, y * CELL_SIZE);
                window.draw(block4Sprite, states);
            }
        }
    }
}

void Game::loadCollectables(const string& filename) {
    ifstream in(filename);
    if (!in.is_open()) {
        cout << "COLLECTABLE ERROR: Could not open " << filename << "\n";
        return;
    }

    char collectableType;
    float x, y;
    while (in >> collectableType >> x >> y && collectableCount < MAX_COLLECTABLES) {
        in.ignore(numeric_limits<streamsize>::max(), '\n');
        float width = 32.0f;
        float height = 32.0f;
        switch (collectableType) {
        case 'R':
            collectables[collectableCount] = new Ring(x * CELL_SIZE, y * CELL_SIZE, width, height, ringTexture);
            collectableCount++;
            break;
        case 'E':
            collectables[collectableCount] = new ExtraLife(x * CELL_SIZE, y * CELL_SIZE, width, height, extraLifeTexture);
            collectableCount++;
            break;
        case 'S':
            collectables[collectableCount] = new SpecialBoost(x * CELL_SIZE, y * CELL_SIZE, width, height, speedBoostTexture, SpecialBoost::SPEED);
            collectableCount++;
            break;
        case 'J':
            collectables[collectableCount] = new SpecialBoost(x * CELL_SIZE, y * CELL_SIZE, width, height, jumpBoostTexture, SpecialBoost::JUMP);
            collectableCount++;
            break;
        case 'I':
            collectables[collectableCount] = new SpecialBoost(x * CELL_SIZE, y * CELL_SIZE, width, height, invincibilityBoostTexture, SpecialBoost::INVINCIBILITY);
            collectableCount++;
            break;
        default:
            continue;
        }
    }
    in.close();
    cout << "Loaded " << collectableCount << " collectables.\n";
}

void Game::updateCollectables() {
    for (int i = 0; i < collectableCount; ++i) {
        if (collectables[i] && collectables[i]->collisionCheck(*characters[mainIndex])) {
            score += collectables[i]->getScoreValue();
            if (dynamic_cast<Ring*>(collectables[i])) {
                cout << "Collected a ring! Score: " << score << "\n";
            }
            else if (dynamic_cast<ExtraLife*>(collectables[i])) {
                sharedHP++;
                cout << "Collected an extra life! Score: " << score << ", HP: " << sharedHP << "\n";
            }
            else if (SpecialBoost* boost = dynamic_cast<SpecialBoost*>(collectables[i])) {
                applyBoost(characters[mainIndex], boost->getBoostType(), boost->getDuration());
                cout << "Collected a boost! Score: " << score << "\n";
            }
            delete collectables[i];
            for (int j = i; j < collectableCount - 1; ++j) {
                collectables[j] = collectables[j + 1];
            }
            collectables[collectableCount - 1] = nullptr;
            collectableCount--;
            --i;
        }
    }
}

void Game::drawCollectables(RenderWindow& window, const RenderStates& states) {
    for (int i = 0; i < collectableCount; ++i) {
        if (collectables[i]) {
            collectables[i]->draw(window, states);
        }
    }
}

void Game::applyBoost(Character* character, int type, float duration) {
    switch (type) {
    case SpecialBoost::SPEED:
        character->currentMaxSpeed = character->getBaseMaxSpeed() * 1.5f;
        speedBoostTimer = duration;
        cout << "Speed boost applied for " << duration << " seconds.\n";
        break;
    case SpecialBoost::JUMP:
        jumpBoostTimer = duration;
        cout << "Jump boost applied for " << duration << " seconds.\n";
        break;
    case SpecialBoost::INVINCIBILITY:
        invincibilityTimer = duration;
        cout << "Invincibility boost applied for " << duration << " seconds.\n";
        break;
    }
}

void Game::updateBoosts(float deltaTime) {
    if (speedBoostTimer > 0.0f) {
        speedBoostTimer -= deltaTime;
        if (speedBoostTimer <= 0.0f) {
            characters[mainIndex]->currentMaxSpeed = characters[mainIndex]->getBaseMaxSpeed() * 1.2f;
            cout << "Speed boost expired.\n";
        }
    }

    if (jumpBoostTimer > 0.0f) {
        jumpBoostTimer -= deltaTime;
        if (jumpBoostTimer <= 0.0f) {
            cout << "Jump boost expired.\n";
        }
    }

    if (invincibilityTimer > 0.0f) {
        invincibilityTimer -= deltaTime;
        if (invincibilityTimer <= 0.0f) {
            cout << "Invincibility boost expired.\n";
        }
    }
}