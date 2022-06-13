#ifndef __SKO_MAP_H_
#define __SKO_MAP_H_

#include "SDL_headers.h"
#include <string>
#include <vector>
#include <map>
#include <fstream>

// SKO Map objects
#include "SKO_MapTile.h"
#include "SKO_MapCollisionRect.h"

// SKO game object Models
#include "SKO_Portal.h"
#include "SKO_Sign.h"
#include "SKO_Enemy.h" 
#include "SKO_Stall.h"
#include "SKO_Shop.h"
#include "SKO_Target.h"
#include "SKO_Npc.h"
#include <unordered_set>
#include "OPI_VectorSet.h"

namespace SKO_Map
{
	class Map
	{ 
	public:  
		Map();
		virtual ~Map();

		// Static map objects
		std::map<std::string, OPI_VectorSet<Tile>> backgroundTiles;
		std::map<std::string, OPI_VectorSet<Tile>> backgroundMaskTiles;
		std::map<std::string, OPI_VectorSet<Tile>> fringeTiles;
		std::map<std::string, OPI_VectorSet<Tile>> fringeMaskTiles;
		// collision  rectangles
		std::set<SKO_Map::CollisionRect> collisionRects;
		// todo collision  lines (like a ladder, you can jump up through it but land on it) 

		// Living game objects inside this map
		std::vector<SKO_Portal*>portals;
		std::vector<SKO_Sign*>signs;
		std::vector<SKO_Enemy*>enemies;
		std::vector<SKO_Stall*>stalls;
		std::vector<SKO_Shop*>shops;
		std::vector<SKO_Target*>targets;
		std::vector<SKO_Npc*>npcs;

		// TODO - Move to Reader/Writer classes

		
		// New version to save as INI and read INI
		std::string filePath = "debug.map.ini";
		std::string name = "Untitled map";
	private:
		
	};
}

#endif
