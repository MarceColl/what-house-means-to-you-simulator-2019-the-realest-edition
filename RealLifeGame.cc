#include "RealLifeGame.h"

////////////////////
/// ACTION CALLBACKS
////////////////////
void pay_respects(RealLifeGame *rlg, RealLifeGame::Actionable a) {
	printf("Respects payed \n");
}

void pay_bills(RealLifeGame *rlg, RealLifeGame::Actionable a){
	printf("Bills payed \n");
	rlg->removeActionable(a.id);
}

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

void RealLifeGame::update_active(){
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
			a.action(this, a.id);
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