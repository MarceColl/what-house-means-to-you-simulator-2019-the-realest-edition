#include <stdio.h>
#include <string.h>

#include <string>
#include <queue>
#include <unordered_map>
#include <cmath>

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#define W_WIDTH 800
#define W_HEIGHT 600

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

		this->arcade_pos = sf::Vector2f(W_WIDTH/2, 100.0);
	}
};

struct SoundPlayer {
	std::unordered_map<std::string, sf::Sound> sound;
	std::unordered_map<std::string, sf::SoundBuffer> buffers;

	float immersion;

 	SoundPlayer() {
		this->add_track("intro_track_arcade", "./Tracks/Intro Arcade.ogg");
		this->immersion = 0.0f;
	}

	void add_track(std::string name, std::string file_path) {
		sf::Sound sound;

		if (this->buffers.count(file_path) == 0) {
			sf::SoundBuffer buffer;
			buffer.loadFromFile(file_path);
			this->buffers[file_path] = buffer;
		}

		sound.setBuffer(this->buffers[file_path]);
		this->sound[name] = sound;
	}

	void play_from_arcade(std::string name) {
		this->sound[name].play();
	}

	void play_from_real_life(std::string name) {
		this->sound[name].setVolume(100.0 - this->immersion);
		this->sound[name].play();
	}
};

struct RealLifeGame {
	struct Actionable{
		int id;
		std::string action_message;
		sf::Vector2f position;
		void (*action)(RealLifeGame*, Actionable);
		int reach;

		bool isReachable(float x);
	};

	bool debug;
	Player *player;
	std::vector<Actionable> actionables;
	std::vector<Actionable> reachable_actionables;
	int selected_reachable;
	sf::RenderWindow &window;
	sf::RectangleShape player_sprite;
	sf::Font action_font;
	sf::Text reachables_text;
	sf::Text action_text;
	sf::Text debug_text;

	float mov_speed;
	float action_reach;

	RealLifeGame(Player *p, sf::RenderWindow &window);

	void removeActionable(int id);

	void update_debug();

	void update_active(sf::Time time);

	void update(sf::Time time);

	void render();
};

////////////////////
/// ACTION CALLBACKS
////////////////////
void pay_respects(RealLifeGame *rlg, RealLifeGame::Actionable a);
void pay_bills(RealLifeGame *rlg, RealLifeGame::Actionable a);

bool RealLifeGame::Actionable::isReachable(float x) {
	return abs(x - this->position.x) < this->reach;
}

RealLifeGame::RealLifeGame(Player *p, sf::RenderWindow &window): window(window), actionables(2) {
	this->player = p;
	this->player_sprite = sf::RectangleShape(sf::Vector2f(20.0, 20.0));
	this->player_sprite.setFillColor(sf::Color(255,0,0));
	this->mov_speed = 0.1;
	this->action_reach = 50;
	this->debug = true;

	this->selected_reachable = 0;
	this->actionables[0].id = 0;
	this->actionables[0].reach = this->action_reach;
	this->actionables[0].action = pay_respects;
	this->actionables[0].action_message = "Press 'F' to pay respects";
	this->actionables[0].position.x = 300;
	this->actionables[0].position.y = 400;
	this->actionables[1].id = 1;
	this->actionables[1].reach = this->action_reach;
	this->actionables[1].action = pay_bills;
	this->actionables[1].action_message = "Press 'F' to pay bills";
	this->actionables[1].position.x = 325;
	this->actionables[1].position.y = 400;

	if (!this->action_font.loadFromFile("pixelart.ttf")) {
		printf("The font was not found!\n");
	}

	this->debug_text.setFont(this->action_font);
	this->debug_text.setPosition(0, window.getSize().y-50);
	this->action_text.setFont(this->action_font);
	this->action_text.setFillColor(sf::Color::White);
	this->reachables_text.setFont(this->action_font);
	this->reachables_text.setFillColor(sf::Color::White);
}

void RealLifeGame::removeActionable(int id){
	for (int i = 0; i < this->actionables.size(); i++) {
		if (this->actionables[i].id == id) {
			this->actionables.erase(this->actionables.begin()+i);
			return;
		}
	}
	printf("Warning! you tried to remove the actionable with id ( %d ), but it doesn't exist. This is a no-op but might be an error");
}

void RealLifeGame::update_debug(){
	char buff[100];
	snprintf(buff, sizeof(buff), "%f , %f", this->player->real_life_pos.x, this->player->real_life_pos.y);
	std::string buffAsStdStr = buff;
	this->debug_text.setString(buffAsStdStr);
}

void RealLifeGame::update(sf::Time time) {}

void RealLifeGame::update_active(sf::Time time) {
	/// ACTIONABLES
	this->reachable_actionables.clear();
	for (auto actionable : this->actionables) {
		if(actionable.isReachable(this->player->real_life_pos.x)) {
			this->reachable_actionables.push_back(actionable);
		}
	}
	// Keep selected in reachables
	if (this->reachable_actionables.size() > 0) {
		this->selected_reachable = this->selected_reachable % this->reachable_actionables.size();
	} 
	else {
		this->selected_reachable = -1;
	}
	// Update reachable text
	if (this->selected_reachable >= 0) {
		Actionable a = this->reachable_actionables[this->selected_reachable];
		float width = this->action_text.getLocalBounds().width;
		this->action_text.setPosition(a.position.x - (width/2.0), a.position.y-50);
		this->action_text.setString(a.action_message);
	} else {
		this->action_text.setString("");
	}

	// Update helper for multiple reachables
	if (this->reachable_actionables.size() > 1) {
		char buff[100];
		snprintf(buff, sizeof(buff), "%d / %d", this->selected_reachable, this->reachable_actionables.size());
		std::string buffAsStdStr = buff;
		this->reachables_text.setString(buffAsStdStr);
	}

	/// PROCESS INPUT
	if (input_array[LEFT]) {
		this->player->real_life_pos.x = this->player->real_life_pos.x - this->mov_speed;
	}
	else if (input_array[RIGHT]) {
		this->player->real_life_pos.x = this->player->real_life_pos.x + this->mov_speed;
	}
	
	if (once_array[ACTION]) {
		if (this->selected_reachable >= 0) {
			Actionable a = this->reachable_actionables[this->selected_reachable];
			a.action(this, a);
		}
	}
	else if (once_array[TOGGLE]) {
		this->selected_reachable = (this->selected_reachable+1) % this->reachable_actionables.size();
	}
	this->player_sprite.setPosition(this->player->real_life_pos);

	if (this->debug) {
		this->update_debug();
	}
}

void RealLifeGame::render(){
	this->window.draw(this->action_text);
	this->window.draw(this->player_sprite);

	if (this->debug) {
		this->window.draw(this->debug_text);
	}
}

void pay_bills(RealLifeGame *rlg, RealLifeGame::Actionable a){
	printf("Bills payed \n");
	rlg->removeActionable(a.id);
}

void pay_respects(RealLifeGame *rlg, RealLifeGame::Actionable a) {
	printf("Respects payed \n");
}

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

	bool menu;

	int score;

	float ppb;

	bool reached_platforms;

	sf::Font action_font;
	sf::Text reachables_text;
	sf::Text action_text;
	sf::Text debug_text;

	sf::Vector2f speed;

	sf::RectangleShape player_sprite;
	sf::RectangleShape platform_sprite;

	sf::VertexArray lines;

	std::vector<Platform> platforms;

	int curr_platform;

	ArcadeGame(Player *p, sf::RenderWindow &window): window(window), lines(sf::Lines, 2) {
		this->player = p;
		this->menu = true;

		if (!this->action_font.loadFromFile("pixelart.ttf")) {
			printf("The font was not found!\n");
		}

		this->debug_text.setFont(this->action_font);
		this->debug_text.setPosition(0, window.getSize().y-50);

		this->action_text.setFont(this->action_font);
		this->action_text.setFillColor(sf::Color::White);
		this->action_text.setPosition(400, window.getSize().y-50);

		this->init();
	}

	void init() {
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
		this->speed.y = 0.0;

		this->player->arcade_pos.y = 100.0;

		this->platforms = std::vector<Platform>();
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

		char buff[100];
		snprintf(buff, sizeof(buff), "%d", this->score);
		std::string buffAsStdStr = buff;
		this->debug_text.setString(buffAsStdStr);

		snprintf(buff, sizeof(buff), "HS %d", this->player->high_score);
		buffAsStdStr = buff;
		this->action_text.setString(buffAsStdStr);

		// Let it wiggle
		this->player_sprite.setScale(
				1.0 + sin(this->time*10)*0.2, 
				1.0 + cos(this->time*10)*0.2);

		if (this->player->arcade_pos.y > W_HEIGHT) {
			if (this->score > this->player->high_score) {
				this->player->high_score = this->score;
			}

			this->init();
		}
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

		target.draw(this->debug_text);
		target.draw(this->action_text);
	}
};

int main() {
    sf::RenderWindow window(sf::VideoMode(W_WIDTH, W_HEIGHT), "SFML works!");
	Player player = Player();
	ArcadeGame ag = ArcadeGame(&player, window);
	RealLifeGame rlg = RealLifeGame(&player, window);

	SoundPlayer sp = SoundPlayer();

	sf::RenderTexture texture;
	texture.create(W_WIDTH, W_HEIGHT);

	sf::Sprite sc_sprite(texture.getTexture());
	sc_sprite.setOrigin(sf::Vector2f(W_WIDTH/2, W_HEIGHT/2));
	sc_sprite.setScale(sf::Vector2f(1.0, -1.0));
	sc_sprite.setPosition(sf::Vector2f(W_WIDTH/2, W_HEIGHT/2));

	sf::Clock deltaClock;

	sf::Shader bloom;
	bloom.loadFromFile("bloom.glsl", sf::Shader::Fragment);

	sp.play_from_arcade("intro_track_arcade");

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

		for (int i = 0; i < INPUT_COUNT; i++) {
			once_array[i] = false;
		}

        window.display();
    }

    return 0;
}
