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

		this->arcade_pos = sf::Vector2f(W_WIDTH/4, 100.0);
	}
};

struct SoundPlayer {
	std::unordered_map<std::string, sf::Sound> sound;
	std::unordered_map<std::string, sf::SoundBuffer> buffers;

	int beat;
	double time;
	int bpm;

	float immersion;

	struct FadeInOutData {
		sf::Sound *sound;
		double start_time;
		double duration;
		bool fade_in;
		bool done;
	};

	std::queue<sf::Sound*> play_queue;
	std::queue<sf::Sound*> stop_queue;
	std::vector<FadeInOutData> fade_inout_queue;

 	SoundPlayer() {
		this->beat = 0;
		this->time = 0.0;
		this->bpm = 130;

		this->add_track("intro_track_arcade", "./Tracks/Intro Arcade.ogg");
		this->add_track("FX_Gameover", "./FX/FX_Gameover.ogg");
		this->add_track("Bass 1", "./Stages/1/Bass 1.ogg");
		this->sound["Bass 1"].setLoop(true);
		this->add_track("Kick", "./Stages/1/Kick.ogg");
		this->sound["Kick"].setLoop(true);
		this->add_track("Snare", "./Stages/1/Snare.ogg");
		this->sound["Snare"].setLoop(true);
		this->add_track("Lead 1", "./Stages/1/Lead 1.ogg");
		this->sound["Lead 1"].setLoop(true);
		this->add_track("Lead 2", "./Stages/1/Lead 2.ogg");
		this->sound["Lead 2"].setLoop(true);
		this->add_track("Lead 3", "./Stages/1/Lead 3.ogg");
		this->sound["Lead 3"].setLoop(true);

		this->immersion = 0.0f;
	}

	void update(sf::Time time) {
		this->time += time.asSeconds();

		int cbeat = this->time*this->bpm/60.0;

		if (cbeat > this->beat) {
			this->beat = cbeat;	

			if (cbeat % 8 == 0) {
				while(!this->play_queue.empty()) {
					this->play_queue.front()->play();
					this->play_queue.pop();
				}

				while(!this->stop_queue.empty()) {
					this->stop_queue.front()->stop();
					this->stop_queue.pop();
				}
			}
		}

		for (int i = 0; i < this->fade_inout_queue.size(); ++i) {
			FadeInOutData *n = &this->fade_inout_queue[i];

			float vol = 100*((this->time - n->start_time)/n->duration);
			if (!n->fade_in) {
				vol = 100 - vol;
			}

			if (vol < 0.5 && !n->fade_in) {
				n->sound->stop();
				n->done = true;
			}

			if (vol >= 100 && n->fade_in) {
				n->done = true;
			}

			n->sound->setVolume(vol);
		}

		for (int i = this->fade_inout_queue.size() - 1; i >= 0; --i) {
			if (this->fade_inout_queue[i].done) {
				this->fade_inout_queue.erase(this->fade_inout_queue.begin() + i);
			}
		}
	}

	void fade_out(std::string name, float time) {
		FadeInOutData d = {
			.sound = &this->sound[name],
			.start_time = this->time,
			.duration = time,
			.fade_in = false
		};

		this->fade_inout_queue.push_back(d);
	}

	void fade_in(std::string name, float time) {
		FadeInOutData d = {
			.sound = &this->sound[name],
			.start_time = this->time,
			.duration = time,
			.fade_in = true
		};

		d.sound->setVolume(0.0);
		d.sound->play();

		this->fade_inout_queue.push_back(d);
	}

	void add_layer(std::string name, bool loop) {
		sf::Sound *s = &this->sound[name];

		s->setLoop(loop);
		if (s->getStatus() != sf::SoundSource::Status::Playing) {
			s->stop();
			this->play_queue.push(s);
		}
	}

	void stop_layer(std::string name, bool loop) {
		sf::Sound *s = &this->sound[name];

		s->setLoop(loop);
		if (s->getStatus() != sf::SoundSource::Status::Playing) {
			s->stop();
			this->stop_queue.push(s);
		}
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
	struct Actionable {
		int id;
		std::string action_message;
		sf::Vector2f position;
		void (*action)(RealLifeGame*, Actionable);
		int reach;

		bool isReachable(float x);
	};

	struct Animation {
		int length;
		sf::Vector2i rect_size;
		int current_frame;
		int frame_length;
		bool loop;
		bool inverted;
		float elapsed_since_last;

		Animation() {
			this->inverted = false;
			this->loop = true;
			this->frame_length = 1000;
			this->current_frame = 0;
		}

		sf::IntRect get_rekt() {
			return sf::IntRect(
				this->rect_size.x*this->current_frame,
				0,
				rect_size.x,
				rect_size.y
			);
		}

		void update(sf::Time t) {
			this->elapsed_since_last += t.asMilliseconds();
			if (this->elapsed_since_last > this->frame_length) {
				this->elapsed_since_last = this->frame_length - this->elapsed_since_last;
				if (this->inverted) {
					this->current_frame--;
				} else {
					this->current_frame++;
				}
				
				if (this->loop) {
					this->current_frame = this->current_frame % this->length;
					if (this->current_frame < 0 ) {
						this->current_frame = this->length-1;
					}
				}
				else if (this->current_frame <= 0) {
					this->current_frame = 0;
				}
				else if (this->current_frame >= this->length) {
					this->current_frame--;
				}
			}
		}

		void to_string() {
			printf("Animation: \ncurrent frame: %d\nloop: %d\ninverted: %d\n", this->current_frame, this->loop, this->inverted);
		}
	};

	enum PlayerState {
		SEATED,
		STANDING,
		SEATING,
		IDLE,
		WALKING
	};

	bool debug;
	Player *player;
	SoundPlayer *sp;

	std::vector<Actionable> actionables;
	std::vector<Actionable> reachable_actionables;
	int selected_reachable;
	
	sf::Texture scene_texture;
	sf::Sprite scene_sprite;

	sf::Texture vr_texture;
	sf::Sprite vr_sprite;
	float vr_top_pos;
	float vr_bottom_pos;
	float vr_pos_y;


	sf::Texture player_texture_seated;
	sf::Texture player_texture_walking;
	sf::Texture player_texture_seated_night;
	sf::Texture player_texture_walking_night;
	sf::Sprite player_sprite;
	Animation player_animation;
	PlayerState player_state;

	sf::RenderWindow &window;

	sf::Font action_font;
	sf::Text reachables_text;
	sf::Text action_text;
	sf::Text debug_text;

	float mov_speed;
	float action_reach;

	RealLifeGame(Player *p, sf::RenderWindow &window, SoundPlayer *sp);

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
void sit(RealLifeGame *rlg, RealLifeGame::Actionable a);
void pay_bills(RealLifeGame *rlg, RealLifeGame::Actionable a);

bool RealLifeGame::Actionable::isReachable(float x) {
	return abs(x - this->position.x) < this->reach;
}

RealLifeGame::RealLifeGame(Player *p, sf::RenderWindow &window, SoundPlayer *sp): window(window), actionables(0), sp(sp) {
	this->player = p;
	if (!this->scene_texture.loadFromFile("Images/room_dia.png")) {
		printf("The scene was not found!\n");
	}
	sf::Vector2u w_size = window.getSize();
	sf::Vector2u st_size = this->scene_texture.getSize();
	this->scene_sprite.setTexture(this->scene_texture);
	this->scene_sprite.setScale(w_size.x / float(st_size.x), w_size.y / float(st_size.y));
	this->player->real_life_pos.x = w_size.x *0.65;
	this->player->real_life_pos.y = w_size.y *0.66;
	if (!this->player_texture_seated.loadFromFile("Images/sprites_finals/sit.png")) {
		printf("sit not found!\n");
	}
	if (!this->player_texture_seated_night.loadFromFile("Images/sprites_finals/sit_nit.png")) {
		printf("sit nit not found!\n");
	}
	if (!this->player_texture_walking.loadFromFile("Images/sprites_finals/walk.png")) {
		printf("walk not found!\n");
	}
	if (!this->player_texture_walking_night.loadFromFile("Images/sprites_finals/walk_nit.png")) {
		printf("awlk nit!\n");
	}
	if (!this->vr_texture.loadFromFile("Images/vr.png")) {
		printf("vr!\n");
	}
	this->player_state = SEATED;
	this->player_animation.length = 4;
	this->player_animation.current_frame = 3;
	this->player_animation.loop = false;
	this->player_animation.frame_length = 400;
	this->player_animation.rect_size = sf::Vector2i(15, 30);
	this->player_sprite.setTexture(this->player_texture_seated);
	this->player_sprite.setTextureRect(this->player_animation.get_rekt());
	this->player_sprite.setScale(w_size.x / float(st_size.x), w_size.y / float(st_size.y));
	this->player_sprite.setOrigin(15/float(2), 30/float(2));
	
	this->vr_sprite.setTexture(this->vr_texture);
	this->vr_sprite.setScale(w_size.x / float(st_size.x), w_size.y / float(st_size.y));
	this->vr_pos_y =  w_size.y / float(st_size.y);
	this->vr_bottom_pos = 0;
	this->vr_top_pos = -100;

	this->mov_speed = 0.1;
	this->action_reach = 50;
	this->debug = false;

	this->selected_reachable = 0;
	/*
	this->actionables[0].id = 0;
	this->actionables[0].reach = this->action_reach;
	this->actionables[0].action = pay_respects;
	this->actionables[0].action_message = "Press 'F' to pay respects";
	this->actionables[0].position.x = 300;
	this->actionables[0].position.y = 400;
	*/
	Actionable couch;
	couch.id = 0;
	couch.reach = this->action_reach;
	couch.action = sit;
	couch.action_message = "Press 'x' to sit on the couch";
	couch.position.x = w_size.x *0.65;
	couch.position.y = w_size.x *0.66;

	this->actionables.push_back(couch);
	

	if (!this->action_font.loadFromFile("pixelart.ttf")) {
		printf("The font was not found!\n");
	}

	this->debug_text.setFont(this->action_font);
	this->debug_text.setPosition(0, w_size.y-50);
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
	// IF DUDE IS NOT SITTING, LET HIM DO SHITE
	if(this->player_state != SEATED) {
		//VR headset
		if (this->vr_pos_y > this->vr_top_pos) {
			this->vr_pos_y -= time.asMilliseconds();
		}
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
		} else {
			this->reachables_text.setString("");
		}

	} else {
		
		if (this->vr_pos_y < this->vr_bottom_pos) {
			this->vr_pos_y += time.asMilliseconds();
		}

		// IF DUDE IS SITING, nope
		this->action_text.setPosition(this->player->real_life_pos.x - 250, this->player->real_life_pos.y - 200);
		this->action_text.setString("Press 'x' to stand");
	}


	/// PROCESS INPUT
	if (input_array[LEFT]) {
		//DONT MOVE IF SEATED
		if(this->player_state == IDLE) {
			this->player_state = WALKING;
			this->player_animation.inverted = false;
			this->player_animation.loop = true;
		}
		if (this->player_state == WALKING) {
			this->player->real_life_pos.x = this->player->real_life_pos.x - this->mov_speed;
		}
	}
	else if (input_array[RIGHT]) {
		//DONT MOVE IF SEATED
		if(this->player_state == IDLE) {
			this->player_state = WALKING;
			this->player_animation.inverted = false;
			this->player_animation.loop = true;
		}
		if (this->player_state == WALKING) {
			this->player->real_life_pos.x = this->player->real_life_pos.x + this->mov_speed;
		}
	} else {
		if(this->player_state == WALKING) { 
			this->player_state = IDLE;
			this->player_animation.inverted = true;
			this->player_animation.loop = false;
			this->player_animation.current_frame = 0;
		}
	}
	if (once_array[ACTION]) {
		if (this->player_state == IDLE || this->player_state == WALKING) {
			if (this->selected_reachable >= 0) {
				Actionable a = this->reachable_actionables[this->selected_reachable];
				a.action(this, a);
			}
		} else if (this->player_state == SEATED) {
			this->player_state = STANDING;
			this->player_animation.inverted = true;
		}
	}
	else if (once_array[TOGGLE]) {
		this->selected_reachable = (this->selected_reachable+1) % this->reachable_actionables.size();
	}
	//APPLY PLAYER STATE TO ANIMATION (maybe bad idea every tick?)
	if(this->player_state == SEATED || this->player_state == STANDING || this->player_state == SEATING) {
		this->player_sprite.setTexture(this->player_texture_seated);
		if (this->player_state == STANDING && this->player_animation.current_frame == 0) {
			this->player_state = IDLE;
			this->player_sprite.setTexture(this->player_texture_walking);
		}
		if(this->player_state == SEATING && this->player_animation.current_frame == 3) {
			this->player_state = SEATED;
		}
	} else {
		this->player_sprite.setTexture(this->player_texture_walking);
	}
	this->player_animation.update(time);
	this->player_sprite.setTextureRect(this->player_animation.get_rekt());
	this->player_sprite.setPosition(this->player->real_life_pos);

	float x = 0;
	sf::Vector2u w_size = window.getSize();
	if (this->player_state == SEATED) {
		x = this->player->real_life_pos.x - w_size.x*0.66;
	} else {
		x = this->vr_sprite.getPosition().x;
	}

	this->vr_sprite.setPosition(x, this->vr_pos_y);

	if (this->debug) {
		this->update_debug();
	}
}

void RealLifeGame::render(){
	this->window.draw(this->scene_sprite);
	this->window.draw(this->player_sprite);
	this->window.draw(this->reachables_text);
	this->window.draw(this->action_text);
	this->window.draw(this->vr_sprite);

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
void sit(RealLifeGame *rlg, RealLifeGame::Actionable a) {
	rlg->player_state = RealLifeGame::PlayerState::SEATING;
	rlg->player_animation.inverted = false;
	rlg->player_animation.loop = false;
	rlg->player_animation.current_frame = 0;
	rlg->action_text.setString("");
}

struct ArcadeGame {
	struct Platform {
		int start;
		int height;
		float duration;
		bool played;
	};

	Player *player;
	SoundPlayer *sp;
	sf::RenderWindow &window;

	float current_good;

	float current_beat;
	float bpm;
	double time;
	bool paused;
	bool on_platform;
	bool dj_avail;

	bool menu;
	int menu_selected;

	int score;

	float ppb;

	bool reached_platforms;

	sf::Font action_font;
	sf::Text action_text;
	sf::Text debug_text;

	sf::Text play_text;
	sf::Text ranking_text;
	sf::Text exit_text;

	sf::Vector2f speed;

	sf::RectangleShape player_sprite;
	sf::RectangleShape platform_sprite;

	sf::VertexArray lines;

	std::vector<Platform> platforms;
	std::vector<sf::Color> colors;

	int curr_platform;

	ArcadeGame(Player *p, sf::RenderWindow &window, SoundPlayer *sp): window(window), lines(sf::Lines, 2), sp(sp) {
		this->player = p;
		this->menu = true;
		this->menu_selected = 0;

		colors.push_back(sf::Color(255, 0, 255));
		colors.push_back(sf::Color(125, 0, 125));
		colors.push_back(sf::Color(75, 0, 75));
		colors.push_back(sf::Color(30, 0, 30));
		colors.push_back(sf::Color(0, 0, 0));

		if (!this->action_font.loadFromFile("pixelart.ttf")) {
			printf("The font was not found!\n");
		}

		this->debug_text.setFont(this->action_font);
		this->debug_text.setPosition(0, window.getSize().y-50);

		this->action_text.setFont(this->action_font);
		this->action_text.setFillColor(sf::Color::White);
		this->action_text.setPosition(400, window.getSize().y-50);

		this->play_text.setFont(this->action_font);
		this->play_text.setFillColor(sf::Color::White);
		this->play_text.setPosition(W_WIDTH/2, W_HEIGHT/5.0);
		this->play_text.setString("PLAY");

		this->ranking_text.setFont(this->action_font);
		this->ranking_text.setFillColor(sf::Color::White);
		this->ranking_text.setPosition(W_WIDTH/2, 2*W_HEIGHT/5.0);
		this->ranking_text.setString("RANKING");

		this->exit_text.setFont(this->action_font);
		this->exit_text.setFillColor(sf::Color::White);
		this->exit_text.setPosition(W_WIDTH/2, 3*W_HEIGHT/5.0);
		this->exit_text.setString("EXIT");

		this->sp->play_from_arcade("intro_track_arcade");
	}

	void init() {
		this->sp->fade_out("intro_track_arcade", 1.0);

		this->current_good = 500.0;

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
		this->time += dt.asSeconds();

		if (this->menu) {
			if (once_array[RIGHT]) {
				this->menu_selected = (this->menu_selected + 1)%3;
			}
			if (once_array[LEFT]) {
				this->menu_selected = (this->menu_selected - 1)%3;
				if (this->menu_selected < 0) {
					this->menu_selected = 2;
				}
			}
			if (input_array[ACTION]) {
				this->menu = false;
				this->init();
			}
			
			return;
		}

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
		
		if (this->time > 1.0 && !this->reached_platforms) {
			this->reached_platforms = true;
		}

		sp->add_layer("Kick", true);
		sp->add_layer("Snare", true);
		sp->add_layer("Bass 1", true);
		if (this->current_good > 650 && this->current_good < 750) {
			sp->add_layer("Lead 1", true);
			sp->stop_layer("Lead 2", true);
			sp->stop_layer("Lead 3", true);
		}
		if (this->current_good >= 750 && this->current_good < 850) {
			sp->add_layer("Lead 2", true);
			sp->stop_layer("Lead 1", true);
			sp->stop_layer("Lead 3", true);
		}

		this->current_beat += (this->bpm/60)*dt.asSeconds();

		if (this->reached_platforms) {
			if (this->current_beat >= this->platforms[this->curr_platform].start + this->platforms[this->curr_platform].duration) {
				this->curr_platform += 1; 

				if (this->curr_platform >= this->platforms.size()) {
					this->curr_platform = 0;
					this->current_beat = 0.0;
					this->bpm += 200;
					this->current_good += 100;
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
				this->current_good += 5;
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
			sp->play_from_arcade("FX_Gameover");
			this->current_good -= 400;

			sp->stop_layer("Lead 1", true);
			sp->stop_layer("Lead 2", true);
			sp->stop_layer("Lead 3", true);

			this->init();
		}
	}

	void update(sf::Time dt) {}

	void render(sf::RenderTarget &target) {
		if (this->menu) {
			int color_index;

			sf::RectangleShape rect;
			color_index = int(sin(this->time*8.0 + 0.5)*5.0);

			rect.setPosition(W_WIDTH/2, W_HEIGHT/2);
			for (int i = 0; i < 5; ++i) {
				rect.setFillColor(colors[(i + color_index)%5]);
				rect.setSize(sf::Vector2f(W_WIDTH - (W_WIDTH/5)*i, W_HEIGHT - (W_HEIGHT/5)*i));
				rect.setOrigin(sf::Vector2f((W_WIDTH - (W_WIDTH/5)*i)/2, (W_HEIGHT - (W_HEIGHT/5)*i)/2));
				rect.rotate(color_index*i);
				target.draw(rect);
			}

			if (this->menu_selected == 0) {
				this->play_text.setFillColor(sf::Color(255, 255, 0));
				sf::FloatRect p_bounds = this->play_text.getGlobalBounds();
				this->play_text.setOrigin(sf::Vector2f(p_bounds.width/2.0, p_bounds.height/2.0));
				this->play_text.setPosition(sf::Vector2f(W_WIDTH/2, W_HEIGHT/2 - 5));

				this->ranking_text.setFillColor(sf::Color(255, 255, 255, 128));
				sf::FloatRect r_bounds = this->ranking_text.getGlobalBounds();
				this->ranking_text.setOrigin(sf::Vector2f(r_bounds.width/2.0, r_bounds.height/2.0));
				this->ranking_text.setPosition(sf::Vector2f(3*W_WIDTH/4, W_HEIGHT/2));
				this->ranking_text.setScale(0.6, 0.6);

				this->exit_text.setFillColor(sf::Color(255, 255, 255, 128));
				sf::FloatRect e_bounds = this->exit_text.getGlobalBounds();
				this->exit_text.setOrigin(sf::Vector2f(e_bounds.width/2.0, e_bounds.height/2.0));
				this->exit_text.setPosition(sf::Vector2f(W_WIDTH/4, W_HEIGHT/2));
				this->exit_text.setScale(0.6, 0.6);
			}
			else if (this->menu_selected == 1) {
				this->play_text.setFillColor(sf::Color(255, 255, 255));
				this->ranking_text.setFillColor(sf::Color(255, 255, 0));
				this->exit_text.setFillColor(sf::Color(255, 255, 255));
			}
			else if (this->menu_selected == 2) {
				this->play_text.setFillColor(sf::Color(255, 255, 255));
				this->ranking_text.setFillColor(sf::Color(255, 255, 255));
				this->exit_text.setFillColor(sf::Color(255, 255, 0));
			}

			target.draw(this->play_text);
			target.draw(this->ranking_text);
			target.draw(this->exit_text);

			return;
		}

		int size = this->platforms.size();
		for (int i = this->curr_platform; i < size * 2; ++i) {
			Platform *p = &this->platforms[i%size];

			float x = p->start * this->ppb + W_WIDTH/4  - this->ppb * this->current_beat;

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

				float x = p->start * this->ppb + W_WIDTH/4  - this->ppb * this->current_beat;

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
	SoundPlayer sp = SoundPlayer();

	Player player = Player();
	ArcadeGame ag = ArcadeGame(&player, window, &sp);
	RealLifeGame rlg = RealLifeGame(&player, window, &sp);


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

		sp.update(dt);

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

