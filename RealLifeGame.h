#pragma once

#include <vector>
#include <string>

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

struct Player;
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

	void update_active();


	void update();

	void render();
};