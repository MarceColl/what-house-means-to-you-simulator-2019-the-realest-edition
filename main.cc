#include <stdio.h>

#include "input.h"

#include "real_life_game.h"
#include "arcade_game.h"

#include <SFML/Graphics.hpp>

typedef enum Game {
	REAL_LIFE,
	ARCADE
} Game;

struct Player {
	// 0.0 -> 1.0
	float pee_need;

	// 0.0 -> 1.0
	float hunger_need;

	// 0.0 -> 1.0
	float hygiene_need;

	int curr_game_points;
	int high_score;

	Game curr_game;

	Player() {
		
	}
};

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML works!");
	ArcadeGame ag = ArcadeGame();
	RealLifeGame rlg = RealLifeGame();
	Player player = Player();

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                window.close();
			}
			else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Left) {
					input_array[LEFT] = true;
				}
				else if (event.key.code == sf::Keyboard::Right) {
					input_array[RIGHT] = true;
				}
				else if (event.key.code == sf::Keyboard::X) {
					input_array[ACTION] = true;
				}
			}
			else if (event.type == sf::Event::KeyReleased) {
				if (event.key.code == sf::Keyboard::Left) {
					input_array[LEFT] = false;
				}
				else if (event.key.code == sf::Keyboard::Right) {
					input_array[RIGHT] = false;
				}
				else if (event.key.code == sf::Keyboard::X) {
					input_array[ACTION] = false;
				}
			}
        }

        window.clear();

		if (player.curr_game == REAL_LIFE) {
			ag.update();
			rlg.update_active();
			rlg.render();
		}
		else {
			rlg.update();
			ag.update_active();
			ag.render();
		}

        window.display();
    }

    return 0;
}

