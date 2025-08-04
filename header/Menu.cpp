#include "Menu.h"

Menu::Menu(Font& fontRef, Music& music)
    : font(fontRef), music(music), selectedIndex(0), isLevelSubmenu(false),
    isSaveSlotSubmenu(false), isScoreboardView(false), musicOn(true), selectedLevel(1), backgroundX(0.0f),
    fadeDuration(2.0f), entryCount(0) {
    // Load background
    if (!backgroundTexture.loadFromFile("Data/image_fx.jpg"))
        cout << "Error loading background image\n";
    backgroundSprite.setTexture(backgroundTexture);
    backgroundSprite.setScale(1.5f, 1.5f);

    // Load logo
    if (!logoTexture.loadFromFile("Data/logo.png"))
        cout << "Error loading logo image\n";
    float logoScale = 0.6f;
    logoSprite.setTexture(logoTexture);
    logoSprite.setScale(logoScale, logoScale);
    logoSprite.setPosition((1200 - logoTexture.getSize().x * logoScale) / 2, 20);

    // Shadow
    shadowSprite = logoSprite;
    shadowSprite.setColor(Color(0, 0, 0, 120));
    shadowSprite.move(5, 5);

    // Main menu items
    const char* menuNames[6] = { "Start Game", "Load Game", "Scoreboard", "Level 1", "Music: On", "Exit Game" };
    for (int i = 0; i < 6; ++i) {
        mainMenuItems[i] = Text(menuNames[i], font, 60);
        mainMenuItems[i].setPosition(420, 300 + i * 80);
    }

    // Level submenu
    const char* levelNames[4] = { "Level One", "Level Two", "Level Three", "Back" };
    for (int i = 0; i < 4; ++i) {
        levelMenuItems[i] = Text(levelNames[i], font, 60);
        levelMenuItems[i].setPosition(450, 320 + i * 80);
    }

    // Save slot submenu
    const char* saveSlotNames[4] = { "Slot 1", "Slot 2", "Slot 3", "Back" };
    for (int i = 0; i < 4; ++i) {
        saveSlotItems[i] = Text(saveSlotNames[i], font, 60);
        saveSlotItems[i].setPosition(450, 320 + i * 80);
    }

    // Scoreboard texts
    for (int i = 0; i < 10; ++i) {
        scoreboardTexts[i].setFont(font);
        scoreboardTexts[i].setCharacterSize(40);
        scoreboardTexts[i].setFillColor(Color::White);
        scoreboardTexts[i].setPosition(300, 370 + i * 60);
        scoreboardTexts[i].setOutlineThickness(2.f);
        scoreboardTexts[i].setOutlineColor(Color(89, 71, 67));
    }

    // Input prompt
    inputBox.setSize({ 600, 80 });
    inputBox.setFillColor(Color(50, 50, 50, 200));
    inputBox.setPosition(350, 400);
    inputText.setFont(font);
    inputText.setCharacterSize(40);
    inputText.setFillColor(Color::White);
    inputText.setPosition(360, 410);

    // Creators
    Creators = Text("Created by Mubeen and Abubakar", font, 30);
    Creators.setPosition(320, 850);
    Creators.setFillColor(Color::Red);
    Creators.setOutlineColor(Color::White);
    Creators.setOutlineThickness(2);

    updateSelection();
    fadeClock.restart();
}

int Menu::run(RenderWindow& window) {
    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                return EXIT;
            if (event.type == Event::KeyPressed) {
                if (isScoreboardView) {
                    if (event.key.code == Keyboard::Escape)
                        isScoreboardView = false;
                }
                else if (isSaveSlotSubmenu) {
                    if (event.key.code == Keyboard::Up) moveUp();
                    else if (event.key.code == Keyboard::Down) moveDown();
                    else if (event.key.code == Keyboard::Enter) {
                        if (selectedIndex < 3) {
                            selectedSaveSlot = "Slot " + to_string(selectedIndex + 1);
                            cout << "Selected save slot: " << selectedSaveSlot << "\n";
                            return LOAD_GAME;
                        }
                        isSaveSlotSubmenu = false;
                    }
                }
                else if (isLevelSubmenu) {
                    if (event.key.code == Keyboard::Up) moveUp();
                    else if (event.key.code == Keyboard::Down) moveDown();
                    else if (event.key.code == Keyboard::Enter) {
                        if (selectedIndex < 3) {
                            selectedLevel = selectedIndex + 1;
                            mainMenuItems[3].setString("Level " + to_string(selectedLevel));
                        }
                        isLevelSubmenu = false;
                    }
                }
                else {
                    if (event.key.code == Keyboard::Up) moveUp();
                    else if (event.key.code == Keyboard::Down) moveDown();
                    else if (event.key.code == Keyboard::Enter) {
                        int action = select(window);
                        if (action != -1) return action;
                    }
                }
            }
        }

        // Animate background
        backgroundX += 0.5f;
        if (backgroundX >= backgroundTexture.getSize().x) backgroundX = 0;
        backgroundSprite.setPosition(-backgroundX, 0);

        // Fade logo
        float elapsed = fadeClock.getElapsedTime().asSeconds();
        float alpha = min(255.f, (elapsed / fadeDuration) * 255.f);
        Color lc = logoSprite.getColor();
        Color sc = shadowSprite.getColor();
        lc.a = static_cast<Uint8>(alpha);
        sc.a = static_cast<Uint8>(alpha * 0.5f);
        logoSprite.setColor(lc);
        shadowSprite.setColor(sc);

        // Draw
        window.clear();
        window.draw(backgroundSprite);
        window.draw(shadowSprite);
        window.draw(logoSprite);
        if (isScoreboardView) drawScoreboard(window);
        else if (isSaveSlotSubmenu) drawSaveSlots(window);
        else draw(window);
        window.display();
    }
    return EXIT;
}

int Menu::getSelectedLevel() const {
    return selectedLevel;
}

string Menu::getSelectedSaveSlot() const {
    return selectedSaveSlot;
}

void Menu::moveUp() {
    int max = isLevelSubmenu || isSaveSlotSubmenu ? 4 : 6;
    selectedIndex = (selectedIndex - 1 + max) % max;
    updateSelection();
}

void Menu::moveDown() {
    int max = isLevelSubmenu || isSaveSlotSubmenu ? 4 : 6;
    selectedIndex = (selectedIndex + 1) % max;
    updateSelection();
}

void Menu::updateSelection() {
    if (isLevelSubmenu) {
        for (int i = 0; i < 4; ++i)
            levelMenuItems[i].setFillColor(i == selectedIndex ? Color::Yellow : Color::White);
    }
    else if (isSaveSlotSubmenu) {
        for (int i = 0; i < 4; ++i)
            saveSlotItems[i].setFillColor(i == selectedIndex ? Color::Yellow : Color::White);
    }
    else {
        for (int i = 0; i < 6; ++i)
            mainMenuItems[i].setFillColor(i == selectedIndex ? Color::Yellow : Color::White);
    }
}

int Menu::select(RenderWindow& window) {
    if (isLevelSubmenu) {
        if (selectedIndex < 3) {
            selectedLevel = selectedIndex + 1;
            mainMenuItems[3].setString("Level " + to_string(selectedLevel));
        }
        isLevelSubmenu = false;
    }
    else if (isSaveSlotSubmenu) {
        if (selectedIndex < 3) {
            selectedSaveSlot = "Slot " + to_string(selectedIndex + 1);
            cout << "Selected save slot: " << selectedSaveSlot << "\n";
        }
        isSaveSlotSubmenu = false;
    }
    else {
        switch (selectedIndex) {
        case 0: {
            char playerName[32] = { '\0' };
            getPlayerName(window, playerName);
            return START_GAME;
        }
        case 1: isSaveSlotSubmenu = true; selectedIndex = 0; break;
        case 2: loadScores(); isScoreboardView = true; break;
        case 3: isLevelSubmenu = true; selectedIndex = 0; break;
        case 4: toggleMusic(); break;
        case 5: return EXIT;
        }
    }
    return -1;
}

void Menu::getPlayerName(RenderWindow& window, char* outName) {
    fill(inputName, inputName + 32, '\0');
    int len = 0;
    bool entering = true;
    while (entering) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) entering = false;
            else if (event.type == Event::TextEntered) {
                if (event.text.unicode == '\b') {
                    if (len > 0) inputName[--len] = '\0';
                }
                else if (len < 31 && event.text.unicode >= 32 && event.text.unicode < 128) {
                    inputName[len++] = static_cast<char>(event.text.unicode);
                    inputName[len] = '\0';
                }
                inputText.setString(inputName);
            }
            else if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::Return) entering = false;
            }
        }
        window.clear();
        window.draw(backgroundSprite);
        window.draw(shadowSprite);
        window.draw(logoSprite);
        window.draw(inputBox);
        window.draw(inputText);
        window.display();
    }
    for (int i = 0; i < 32; ++i) outName[i] = inputName[i];
}

void Menu::toggleMusic() {
    musicOn = !musicOn;
    mainMenuItems[4].setString(musicOn ? "Music: On" : "Music: Off");
    if (musicOn) music.play(); else music.stop();
}

void Menu::draw(RenderWindow& window) {
    if (isLevelSubmenu) {
        for (int i = 0; i < 4; ++i) {
            Text shadow = levelMenuItems[i];
            shadow.move(3.f, 3.f);
            shadow.setFillColor(Color(50, 50, 50));
            window.draw(shadow);
            window.draw(levelMenuItems[i]);
        }
    }
    else if (isSaveSlotSubmenu) {
        drawSaveSlots(window);
    }
    else {
        for (int i = 0; i < 6; ++i) {
            Text shadowText = mainMenuItems[i];
            shadowText.move(3.f, 3.f);
            shadowText.setFillColor(Color(50, 50, 50));
            window.draw(shadowText);
            window.draw(mainMenuItems[i]);
        }
    }
    window.draw(Creators);
}

void Menu::drawSaveSlots(RenderWindow& window) {
    for (int i = 0; i < 4; ++i) {
        Text shadow = saveSlotItems[i];
        shadow.move(3.f, 3.f);
        shadow.setFillColor(Color(50, 50, 50));
        window.draw(shadow);
        window.draw(saveSlotItems[i]);
    }
}

void Menu::loadScores() {
    ifstream file(scoreFile);
    entryCount = 0;
    if (!file.is_open()) {
        cout << "Failed to open scoreboard file for reading.\n";
        return;
    }

    string line;
    while (getline(file, line) && entryCount < 1000) {
        // Skip empty lines
        if (line.empty()) continue;

        // Trim leading and trailing whitespace
        size_t start = line.find_first_not_of(" \t");
        size_t end = line.find_last_not_of(" \t");
        if (start == string::npos || end == string::npos) continue;
        line = line.substr(start, end - start + 1);

        // Find the last space to separate name and score
        size_t lastSpace = line.rfind(' ');
        if (lastSpace == string::npos) continue;

        // Extract name and score strings
        string nameStr = line.substr(0, lastSpace);
        string scoreStr = line.substr(lastSpace + 1);

        // Trim name and validate
        start = nameStr.find_first_not_of(" \t");
        end = nameStr.find_last_not_of(" \t");
        if (start == string::npos || end == string::npos) continue;
        nameStr = nameStr.substr(start, end - start + 1);
        if (nameStr.empty() || nameStr.length() > 31) continue;

        // Parse score
        try {
            size_t pos;
            int score = stoi(scoreStr, &pos);
            if (pos != scoreStr.length() || score < 0) continue; // Ensure entire string is a valid number

            // Manually copy name to entry
            for (size_t i = 0; i < nameStr.length() && i < 31; ++i) {
                entries[entryCount].name[i] = nameStr[i];
            }
            entries[entryCount].name[nameStr.length() < 31 ? nameStr.length() : 31] = '\0';
            entries[entryCount].score = score;
            entryCount++;
        }
        catch (...) {
            continue; // Skip invalid score formats
        }
    }
    file.close();

    // Sort entries in descending order
    for (int i = 0; i < entryCount - 1; i++) {
        for (int j = 0; j < entryCount - i - 1; j++) {
            if (entries[j].score < entries[j + 1].score) {
                Entry temp = entries[j];
                entries[j] = entries[j + 1];
                entries[j + 1] = temp;
            }
        }
    }

    // Update display for top 10 scores
    for (int i = 0; i < entryCount && i < 10; i++) {
        char displayText[64];
        char scoreStr[16];
        int tempScore = entries[i].score;
        int scoreDigits = 0;
        if (tempScore == 0) {
            scoreStr[0] = '0';
            scoreStr[1] = '\0';
            scoreDigits = 1;
        }
        else {
            int temp = tempScore;
            while (temp > 0) {
                scoreDigits++;
                temp /= 10;
            }
            for (int j = scoreDigits - 1; j >= 0; j--) {
                scoreStr[j] = '0' + (tempScore % 10);
                tempScore /= 10;
            }
            scoreStr[scoreDigits] = '\0';
        }

        int pos = 0;
        displayText[pos++] = '0' + (i + 1) / 10;
        if ((i + 1) >= 10) displayText[pos++] = '0' + (i + 1) % 10;
        else displayText[pos - 1] = '0' + (i + 1);
        displayText[pos++] = '.';
        displayText[pos++] = ' ';
        for (int j = 0; entries[i].name[j] != '\0'; j++) {
            displayText[pos++] = entries[i].name[j];
        }
        displayText[pos++] = ' ';
        displayText[pos++] = '-';
        displayText[pos++] = ' ';
        for (int j = 0; j < scoreDigits; j++) {
            displayText[pos++] = scoreStr[j];
        }
        displayText[pos] = '\0';
        scoreboardTexts[i].setString(displayText);
    }
}

void Menu::updateScoreboard(const char* name, int score) {
    ofstream file(scoreFile, ios::app);
    if (!file.is_open()) {
        cout << "Failed to open scoreboard file for writing.\n";
        return;
    }
    file << name << " " << score << "\n";
    file.close();
    cout << "Scoreboard updated: " << name << " with score " << score << "\n";
    loadScores();
}

void Menu::drawScoreboard(RenderWindow& window) {
    Text title("--- Top 10 Scores ---", font, 60);
    title.setPosition(250, 300);
    Text titleShadow = title;
    titleShadow.move(3.f, 3.f);
    titleShadow.setFillColor(Color(50, 50, 50));

    window.draw(titleShadow);
    window.draw(title);

    for (int i = 0; i < entryCount && i < 10; ++i) {
        Text shadow = scoreboardTexts[i];
        shadow.move(2.f, 2.f);
        shadow.setFillColor(Color(50, 50, 50));
        window.draw(shadow);
        window.draw(scoreboardTexts[i]);
    }
}