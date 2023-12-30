#include <SFML/Graphics.hpp>

#include <chrono>

#include "grid.h"

sf::RectangleShape rectangles[grid::GRID_SIZE][grid::GRID_SIZE];

int main() {
    sf::RenderWindow window(sf::VideoMode(1000, 1000), "Game");

    const double ratio = 1.0 / 10;

    creatures_from_id[0] = &creatures[0];
    fish_nets[0] = fish_borders[0];
    drones[0][0].pos = Vector(3000, 3500);

    grid::initialize_probability();
    for (int i = 0; i < grid::GRID_SIZE; i++) {
        for (int j = 0; j < grid::GRID_SIZE; j++) {
            double x = i * UNITS_PER_BLOCK * ratio;
            double y = j * UNITS_PER_BLOCK * ratio;

            rectangles[i][j].setSize(sf::Vector2f(UNITS_PER_BLOCK * ratio, UNITS_PER_BLOCK * ratio));
            rectangles[i][j].setPosition(x, y);
        }
    }

    while (window.isOpen()) {
        for (sf::Event event; window.pollEvent(event); ) {
            switch (event.type) {
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::KeyPressed:
                    if (event.key.code == sf::Keyboard::Space) {
                        auto start = std::chrono::high_resolution_clock::now();
                        grid::spread(0);
                        auto stop = std::chrono::high_resolution_clock::now();
                        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

                        std::cerr << "TIME: " << duration.count() * 0.001 << "ms" << '\n';
                    }
                    else
                    if (event.key.code == sf::Keyboard::R) {
                        grid::clear_drone_view(0);
                    }
                default:
                    break;
            }
        }

        window.clear();

        for (int i = 0; i < grid::GRID_SIZE; i++) {
            for (int j = 0; j < grid::GRID_SIZE; j++) {
                    rectangles[i][j].setFillColor(sf::Color(255 * pow(p[0][0][i][j], 5), 0, 0));
                window.draw(rectangles[i][j]);
            }
        }

        window.display();
    }
}