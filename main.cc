#include <stdio.h>

#include <string>
#include <queue>
#include <cmath>

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#define W_WIDTH 800
#define W_HEIGHT 600

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

		this->arcade_pos = sf::Vector2f(W_WIDTH/2, 100.0);
	}
};

struct RealLifeGame {
	Player *player;
	sf::RenderWindow &window;

	RealLifeGame(Player *p, sf::RenderWindow &window): window(window) {
		this->player = p;
	}

	void update_active(sf::Time dt) {
	}

	void update(sf::Time dt) {}

	void render() {}
};

struct ArcadeGame {
	struct Platform {
		int start;
		int height;
		float duration;
		bool played;
	};

	Player *player;
	sf::RenderWindow &window;

	float current_beat;
	float bpm;
	double time;
	bool paused;
	bool on_platform;
	bool dj_avail;

	int score;

	float ppb;

	bool reached_platforms;

	sf::Vector2f speed;

	sf::RectangleShape player_sprite;
	sf::RectangleShape platform_sprite;

	sf::VertexArray lines;

	std::vector<Platform> platforms;

	int curr_platform;

	ArcadeGame(Player *p, sf::RenderWindow &window): window(window), lines(sf::Lines, 2) {
		this->player = p;
		this->bpm = 1200.0;
		this->current_beat = -1.3 * (this->bpm/60);
		this->paused = false;
		this->on_platform = false;
		this->ppb = 25;
		this->player_sprite = sf::RectangleShape(sf::Vector2f(20.0, 20.0));
		this->player_sprite.setOrigin(sf::Vector2f(10.0, 20.0));
		this->platform_sprite = sf::RectangleShape(sf::Vector2f(1.0, 10.0));
		this->curr_platform = 0;
		this->time = 0;
		this->dj_avail = true;
		this->score = 0;

		this->reached_platforms = false;

		this->lines[0] = sf::Vector2f(W_WIDTH/2.0, 0);
		this->lines[1] = sf::Vector2f(W_WIDTH/2.0, W_HEIGHT);

		int offset_beat = 0;
		for (int i = 0; i < 50; ++i) {
			int beats = std::rand()%4 + 1;
			int height = 200 + std::rand()%4 * 50;

			for (int j = 0; j < beats; ++j) {
				Platform plat = {
					.start = offset_beat,
					.height = height,
					.duration = 1,
					.played = false
				};

				this->platforms.push_back(plat);

				offset_beat += 1;
			}
		}
	}

	void update_active(sf::Time dt) {
		if (input_array[ACTION]) {
			if (this->on_platform || this->dj_avail) { 
				this->speed.y = -800;
				this->on_platform = false;
				this->dj_avail = false;
			}
		}
		else if (input_array[ARCADE_QUIT]) {
			this->player->curr_game = REAL_LIFE;
			printf("Exited arcade game\n");
		}
		
		this->time += dt.asSeconds();

		if (this->time > 1.0 && !this->reached_platforms) {
			this->reached_platforms = true;
		}

		this->current_beat += (this->bpm/60)*dt.asSeconds();

		if (this->reached_platforms) {
			if (this->current_beat >= this->platforms[this->curr_platform].start + this->platforms[this->curr_platform].duration) {
				this->curr_platform += 1; 

				if (this->curr_platform >= this->platforms.size()) {
					this->curr_platform = 0;
					this->current_beat = 0.0;
					this->bpm += 200;
				}

				this->on_platform = false;
			}


			sf::Vector2f *pos = &this->player->arcade_pos;

			if (this->speed.y > 0 &&
				pos->y > this->platforms[this->curr_platform].height &&
				pos->y < this->platforms[this->curr_platform].height + 20) {
				this->on_platform = true;
				this->dj_avail = true;
				pos->y = this->platforms[this->curr_platform].height;
				this->platforms[this->curr_platform].played = true;
				this->score += this->bpm;
				printf("%d\n", this->score);
			}

			if (this->on_platform) {
				this->speed.y = 0;
				this->player_sprite.setRotation(0);
				this->player_sprite.setOrigin(sf::Vector2f(10.0, 20.0));
			}
			else {
				this->speed.y += 2000.0 * dt.asSeconds();
				this->player_sprite.rotate(180*dt.asSeconds());
				this->player_sprite.setOrigin(sf::Vector2f(10.0, 10.0));
			}

			pos->y += this->speed.y * dt.asSeconds();
		}

		// Let it wiggle
		this->player_sprite.setScale(
				1.0 + sin(this->time*10)*0.2, 
				1.0 + cos(this->time*10)*0.2);
	}

	void update(sf::Time dt) {}

	void render(sf::RenderTarget &target) {
		int size = this->platforms.size();
		for (int i = this->curr_platform; i < size * 2; ++i) {
			Platform *p = &this->platforms[i%size];

			float x = p->start * this->ppb + W_WIDTH/2  - this->ppb * this->current_beat;

			if (i > size) {
				x += size * this->ppb;
			}

			if (x > W_WIDTH + 100) {
				break;
			}

			this->platform_sprite.setPosition(sf::Vector2f(x, p->height));
			this->platform_sprite.setFillColor(sf::Color(255, 0, 255));
			this->platform_sprite.setScale(p->duration * this->ppb, 1);

			target.draw(this->platform_sprite);
		}

		if (this->reached_platforms) {
			for (int i = 0; i < size * 2; ++i) {
				Platform *p = &this->platforms[(this->curr_platform - i)%size];

				float x = p->start * this->ppb + W_WIDTH/2  - this->ppb * this->current_beat;

				if (i > this->curr_platform) {
					x -= size * this->ppb;
				}

				if (x < -100) {
					break;
				}

				this->platform_sprite.setPosition(sf::Vector2f(x, p->height));
				if (p->played) {
					this->platform_sprite.setFillColor(sf::Color(255, 255, 0));
				}
				else {
					this->platform_sprite.setFillColor(sf::Color(255, 0, 255));
				}
				this->platform_sprite.setScale(p->duration * this->ppb, 1);

				target.draw(this->platform_sprite);
			}
		}

		this->player_sprite.setPosition(player->arcade_pos);
		target.draw(this->player_sprite);

		// window.draw(this->lines);
	}
};

int main() {
    sf::RenderWindow window(sf::VideoMode(W_WIDTH, W_HEIGHT), "SFML works!");
	Player player = Player();
	ArcadeGame ag = ArcadeGame(&player, window);
	RealLifeGame rlg = RealLifeGame(&player, window);

	sf::RenderTexture texture;
	texture.create(W_WIDTH, W_HEIGHT);

	sf::Sprite sc_sprite(texture.getTexture());
	sc_sprite.setOrigin(sf::Vector2f(W_WIDTH/2, W_HEIGHT/2));
	sc_sprite.setScale(sf::Vector2f(1.0, -1.0));
	sc_sprite.setPosition(sf::Vector2f(W_WIDTH/2, W_HEIGHT/2));

	sf::Clock deltaClock;

	sf::Shader bloom;
	bloom.loadFromFile("bloom.glsl", sf::Shader::Fragment);

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

		sf::Time dt = deltaClock.restart();

		if (player.curr_game == REAL_LIFE) {
			ag.update(dt);
			rlg.update_active(dt);
			rlg.render();
		}
		else {
			rlg.update(dt);
			ag.update_active(dt);

			texture.clear();
		
			ag.render(texture);
			window.draw(sc_sprite, &bloom);
		}

        window.display();
    }

    return 0;
}


