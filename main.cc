#include <stdio.h>

#include <string>
#include <queue>

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

enum Input {
	LEFT,
	RIGHT,
	ACTION,
	ARCADE_QUIT,

	// Always at the end
	INPUT_COUNT
};

bool input_array[INPUT_COUNT];

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
	}
};

struct RealLifeGame {
	Player *player;

	RealLifeGame(Player *p) {
		this->player = p;
	}

	void update_active() {
	}

	void update() {}

	void render() {}
};

struct ArcadeGame {
	Player *player;

	ArcadeGame(Player *p) {
		this->player = p;
	}

	void update_active() {
		if (input_array[ARCADE_QUIT]) {
			this->player->curr_game = REAL_LIFE;
			printf("Exited arcade game\n");
		}
	}

	void update() {}

	void render() {}
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML works!");
	Player player = Player();
	ArcadeGame ag = ArcadeGame(&player);
	RealLifeGame rlg = RealLifeGame(&player);

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
				else if (event.key.code == sf::Keyboard::Q) {
					input_array[ARCADE_QUIT] = true;
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

