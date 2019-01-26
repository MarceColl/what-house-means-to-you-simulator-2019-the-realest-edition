#include <stdio.h>
#include <string.h>

#include <string>
#include <queue>

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "RealLifeGame.h"

enum Input {
	LEFT,
	RIGHT,
	ACTION,
	TOGGLE,
	ARCADE_QUIT,

	// Always at the end
	INPUT_COUNT
};

bool input_array[INPUT_COUNT];
bool once_array[INPUT_COUNT];

typedef enum Game {
	REAL_LIFE,
	ARCADE
} Game;

typedef enum EventType {
	NOTIFICATION
} EventType;

typedef enum NotificationType {
	STANDARD
} NotificationType;

struct NotificationData {
	std::string text;
	NotificationType type;
};
	
struct Event {
	EventType type;

	union {
		NotificationData notification;
	};
};

struct Player {
	// 0.0 -> 1.0
	float pee_need;

	// 0.0 -> 1.0
	float hunger_need;

	// 0.0 -> 1.0
	float hygiene_need;

	int curr_game_points;
	int high_score;

	sf::Vector2f real_life_pos;
	sf::Vector2f arcade_pos;

	Game curr_game;

	std::queue<Event*> event_queue;

	Player() {
		this->pee_need = 0.0;
		this->hunger_need = 0.0;
		this->hygiene_need = 0.0;

		this->curr_game_points = 0;
		this->high_score = 0;

		this->curr_game = ARCADE;

		this->arcade_pos = sf::Vector2f(200.0, 400.0);
		this->real_life_pos = sf::Vector2f(200.0, 400.0);
	}
};



struct ArcadeGame {
	struct Platform {
		float start;
		int height;
		float duration;
	};

	Player *player;
	sf::RenderWindow &window;

	float current_beat;
	float bpm;
	bool paused;

	sf::RectangleShape player_sprite;
	sf::RectangleShape platform_sprite;

	std::vector<Platform> platforms;

	int curr_platform;

	ArcadeGame(Player *p, sf::RenderWindow &window): window(window) {
		this->player = p;
		this->current_beat = 0.0;
		this->bpm = 10.0;
		this->paused = false;
		this->player_sprite = sf::RectangleShape(sf::Vector2f(20.0, 20.0));
		this->platform_sprite = sf::RectangleShape(sf::Vector2f(20.0, 20.0));
		this->curr_platform = 0;

		for (int i = 0; i < 50; ++i) {
			Platform plat = {
				.start = i * 100.0,
				.height = std::rand()%100 * 50,
				.duration = 9.0
			};
			platforms.push_back(plat);
		}
	}

	void update_active() {
		if (input_array[ACTION]) {
			// JUMP
		}
		else if (input_array[ARCADE_QUIT]) {
			this->player->curr_game = REAL_LIFE;
			printf("Exited arcade game\n");
		}
	}

	void update() {}

	void render() {
		for (int i = 0; i < this->platforms.size(); ++i) {
			Platform *p = &this->platforms[i];
			this->platform_sprite.setPosition(sf::Vector2f(this->platforms[i].start, p->height));
			this->platform_sprite.setFillColor(sf::Color(255, 0, 255));
			this->platform_sprite.setScale(p->duration, 1);
			window.draw(this->platform_sprite);
		}

		this->player_sprite.setPosition(player->arcade_pos);
		window.draw(this->player_sprite);
	}
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML works!");
	Player player = Player();
	ArcadeGame ag = ArcadeGame(&player, window);
	RealLifeGame rlg = RealLifeGame(&player, window);

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
					once_array[LEFT] = true;
				}
				else if (event.key.code == sf::Keyboard::Right) {
					input_array[RIGHT] = true;
					once_array[RIGHT] = true;
				}
				else if (event.key.code == sf::Keyboard::X) {
					input_array[ACTION] = true;
					once_array[ACTION] = true;
				}
				else if (event.key.code == sf::Keyboard::Q) {
					input_array[ARCADE_QUIT] = true;
					once_array[ARCADE_QUIT] = true;
				}
				else if (event.key.code == sf::Keyboard::Z) {
					input_array[TOGGLE] = true;
					once_array[TOGGLE] = true;

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
				else if (event.key.code == sf::Keyboard::Q) {
					input_array[ARCADE_QUIT] = false;
				}
				else if (event.key.code == sf::Keyboard::Z) {
					input_array[TOGGLE] = false;
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

		for (int i = 0; i < INPUT_COUNT; i++) {
			once_array[i] = false;
		}

        window.display();
    }

    return 0;
}
