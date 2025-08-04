#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <fstream>
#include <iostream>
#include <string>

class Menu {
public:
    static const int START_GAME = 0;
    static const int LOAD_GAME = 1;
    static const int EXIT = 2;

    Menu(sf::Font& fontRef, sf::Music& music);
    int run(sf::RenderWindow& window);
    int getSelectedLevel() const;
    std::string getSelectedSaveSlot() const;
    void updateScoreboard(const char* name, int score);
    void getPlayerName(sf::RenderWindow& window, char* outName);
private:
    sf::Font& font;
    sf::Music& music;
    sf::Texture backgroundTexture, logoTexture;
    sf::Sprite backgroundSprite, logoSprite, shadowSprite;
    sf::Text mainMenuItems[6];
    sf::Text levelMenuItems[4];
    sf::Text saveSlotItems[4]; // Added for save slots
    sf::Text scoreboardTexts[1000];
    sf::Text Creators;
    sf::RectangleShape inputBox;
    sf::Text inputText;
    char inputName[32];
    int selectedIndex;
    bool isLevelSubmenu;
    bool isSaveSlotSubmenu; // Added for save slot selection
    bool isScoreboardView;
    bool musicOn;
    int selectedLevel;
    std::string selectedSaveSlot; // Added to store selected save slot
    float backgroundX;
    sf::Clock fadeClock;
    const float fadeDuration;

    struct Entry {
        char name[32];
        int score;
    };
    const char* scoreFile = "scoreboard.txt";
    Entry entries[10];
    int entryCount;

    void moveUp();
    void moveDown();
    void updateSelection();
    int select(sf::RenderWindow& window);
    
    void toggleMusic();
    void draw(sf::RenderWindow& window);
    void loadScores();
    
    void drawScoreboard(sf::RenderWindow& window);
    void drawSaveSlots(sf::RenderWindow& window); // Added to draw save slots
};