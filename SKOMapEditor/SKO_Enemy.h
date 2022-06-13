#include <ctime>
#include <vector>
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <cmath>

#include "SKO_ItemObject.h"

#define MAX_CLIENTS 16

#ifndef __SKO_ENEMY_H_
#define __SKO_ENEMY_H_

bool Roll(float chance);

typedef struct
{
	unsigned int item_id;
	unsigned int amount;
	float chance;
} Loot;

class SKO_Enemy
{
	public:
		SKO_Enemy();
		SKO_Enemy(std::string type, int x1_in, int y1_in, int x2_in, int y2_in, int sx_in, int sy_in);  
		void Respawn();
		void addLoot(unsigned int item_id, unsigned int amount, float chance);
		SKO_ItemObject getLootItem();

		//What type of enemy is this?
		std::string type;

		//coords
		float x = 0.0f;
		float y = 0.0f;
		float x_speed = 0.0f;
		float y_speed = 0.0f;

		//spawn location
		int spawn_x = 0;
		int spawn_y = 0;

		//collision rect
		int x1,x2,y1,y2;


		// 
		unsigned int defense;
		unsigned int strength;
		unsigned int hp;
		unsigned int xp;
		unsigned int hp_max;
		int hp_draw;

		bool facing_right; 
		bool attacking;
		bool ground;
		int  current_frame;
		unsigned long long int animation_ticker;
		unsigned long long int attack_ticker;

		//loot
		int lootAmount;
		std::vector<Loot> lootItem;

		//keep track of players who hit me
		int dibsPlayer;
		unsigned long long int dibsTicker;
		int dibsDamage[MAX_CLIENTS];


		//AI
		bool dead;
		unsigned long long int respawn_ticker;
		unsigned long long int AI_ticker;
		unsigned long long int AI_period;
		int      AI_pos;      
      
};

#endif
