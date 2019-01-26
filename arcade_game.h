#ifndef __GAEM_ARCADE_GAME
#define __GAEM_ARCADE_GAME

#include <SFML/Graphics.hpp>

struct ArcadeGame {
	ArcadeGame();

	void update_active();
	void update();

	void render();
};

#endif
