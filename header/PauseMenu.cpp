#include "PauseMenu.h"

PauseMenu::PauseMenu(Font& fontRef) : font(fontRef), selectedIndex(0) {
    menuItems[0] = Text("Resume Game", font, 60);
    menuItems[1] = Text("Save Game", font, 60);
    menuItems[2] = Text("Exit", font, 60); // Changed from "Exit to Main Menu" to "Exit"
    for (int i = 0; i < 3; ++i) {
        menuItems[i].setPosition(420, 400 + i * 80);
    }
    updateSelection();
}

int PauseMenu::run(RenderWindow& window) {
    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                return 1; // Exit to main menu
            if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::Up) moveUp();
                else if (event.key.code == Keyboard::Down) moveDown();
                else if (event.key.code == Keyboard::Enter) {
                    if (selectedIndex == 0) return 0;      // Resume game
                    else if (selectedIndex == 1) return 2; // Save game
                    else return 1;                         // Exit to main menu
                }
            }
        }

        window.clear();
        draw(window);
        window.display();
    }
    return 1; // Exit to main menu if window is closed
}

void PauseMenu::moveUp() {
    selectedIndex = (selectedIndex - 1 + 3) % 3;
    updateSelection();
}

void PauseMenu::moveDown() {
    selectedIndex = (selectedIndex + 1) % 3;
    updateSelection();
}

void PauseMenu::updateSelection() {
    for (int i = 0; i < 3; ++i) {
        menuItems[i].setFillColor(i == selectedIndex ? Color::Yellow : Color::White);
    }
}

void PauseMenu::draw(RenderWindow& window) {
    for (int i = 0; i < 3; ++i) {
        Text shadow = menuItems[i];
        shadow.move(3.f, 3.f);
        shadow.setFillColor(Color(50, 50, 50));
        window.draw(shadow);
        window.draw(menuItems[i]);
    }
}