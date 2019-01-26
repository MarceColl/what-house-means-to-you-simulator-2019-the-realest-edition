#ifndef __GAEM_ARCADE_GAME
#define __GAEM_ARCADE_GAME

#include <SFML/Graphics.hpp>

struct Player;

struct ArcadeGame {
	ArcadeGame();

	void set_player(Player *p);
	void update_active();
	void update();

	void render();
};

#endif
