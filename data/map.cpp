#include <fstream>
#include <iostream>
#include <vector>
#include <string>

struct EnemyData {
    char type; // B (BatBrain), E (BeeBot), M (Motobug), C (CrabMeat), S (EggStinger)
    float x, y; // Grid coordinates
};

int main() {
    // Define the output file path
    const std::string filename = "enemies.txt";

    // Sample enemy data for a 30x15 grid level
    std::vector<EnemyData> enemies = {
        // BatBrains (flying, chase player within 300 pixels)
        {'B', 8.0f, 5.0f},  // Near start, high up
        {'B', 20.0f, 4.0f}, // Mid-level, high
        // BeeBots (fly in patterns, shoot every 5s)
        {'E', 12.0f, 3.0f}, // Early platform
        {'E', 25.0f, 6.0f}, // Later in level
        // Motobugs (crawl, chase if player within 200 pixels)
        {'M', 10.0f, 10.0f}, // Ground level, early
        {'M', 18.0f, 11.0f}, // Ground, mid-level
        {'M', 28.0f, 10.0f}, // Ground, near end
        // CrabMeats (walk, shoot every 5s)
        {'C', 15.0f, 9.0f},  // Mid-level platform
        {'C', 22.0f, 8.0f},  // Later platform
        // EggStinger (flies, attacks periodically)
        {'S', 30.0f, 2.0f}   // Near level end, high up
    };

    // Open the file for writing
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Error: Could not create " << filename << std::endl;
        return 1;
    }

    // Write each enemy to the file
    for (const auto& enemy : enemies) {
        out << enemy.type << " " << enemy.x << " " << enemy.y << "\n";
    }

    // Close the file
    out.close();

    std::cout << "Successfully created " << filename << " with " << enemies.size() << " enemies.\n";

    return 0;
}