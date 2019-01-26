#ifndef __GAEM_REAL_LIFE_GAME
#define __GAEM_REAL_LIFE_GAME

#include <SFML/Graphics.hpp>

struct RealLifeGame {
	RealLifeGame();

	void set_player(Player *p);
	void update_active();
	void update();

	void render();
};

#endif
