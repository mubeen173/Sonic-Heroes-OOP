#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>

class PauseMenu {
public:
    PauseMenu(Font& font);
    int run(RenderWindow& window);

private:
    Font& font;
    Text menuItems[3];
    int selectedIndex;

    void moveUp();
    void moveDown();
    void updateSelection();
    void draw(RenderWindow& window);
};