#include "SKO_MapReader.h"
#include <fstream>
#include <exception>
#include <sstream>
#include "TextComponent.h"
#include "FontManager.h"


SKO_Map::Reader::Reader(std::map<std::string, SKO_Map::Tileset*> tilesets)
{
	this->tilesets = tilesets;
}

SKO_Map::Reader::~Reader()
{

}

SKO_Map::Map * SKO_Map::Reader::loadMap(std::string filePath)
{
	SKO_Map::Map *map = new SKO_Map::Map();

	// Open the file to read
	INIReader mapIni(filePath);
	if (mapIni.ParseError() < 0)
	{

		std::string error = "Failed to load INI file: " + filePath;
		throw new std::ios_base::failure(error);
	}

	map->filePath = filePath;

	loadMapName(map, mapIni);
	loadBackgroundTiles(map, mapIni);
	loadBackgroundMaskTiles(map, mapIni);
	loadFringeTiles(map, mapIni); 
	loadFringeMaskTiles(map, mapIni);
	loadCollisionRects(map, mapIni); 

	loadPortals(map, mapIni);
	loadSigns(map, mapIni);   
	loadEnemies(map, mapIni);
	loadStalls(map, mapIni);  
	loadShops(map, mapIni);   
	loadTargets(map, mapIni); 
	loadNpcs(map, mapIni);

	// TODO loadNatureEngine(); // tee hee
	
	return map;
}


std::vector<std::string> SKO_Map::Reader::parseCsv(std::string valuesCsv)
{
	std::stringstream sstream(valuesCsv);
	std::string tmp;
	std::vector<std::string> values;
	char delimiter = ',';

	while (std::getline(sstream, tmp, delimiter)) 
	{
		values.push_back(tmp);
	}

	return values;
}

std::vector<int> SKO_Map::Reader::parseCsvInt(std::string valuesCsv)
{
	std::stringstream sstream(valuesCsv);
	std::string tmp;
	std::vector<int> values;
	char delimiter = ',';

	while (std::getline(sstream, tmp, delimiter))
	{
		std::stringstream str(tmp);
		int number;
		if (!(str >> number).fail())
		{
			values.push_back(number);
		}
	}

	return values;
}

void SKO_Map::Reader::loadTileLayer(std::string name, std::map<std::string, OPI_VectorSet<Tile>> &tileLayer, INIReader mapIni)
{
	for (auto it = tilesets.begin(); it != tilesets.end(); ++it)
	{
		std::string tilesetKey = it->first;
		
		// Load tiles from this config section, if it exists
		std::string section = name + "_" + tilesetKey;

		for (int i = 0; ; ++i)
		{
			std::stringstream tileTag;
			tileTag << "tile" << i;
			std::string values = mapIni.Get(section, tileTag.str(), "null");

			if (values == "null")
			{
				break;
			}

			auto parsedValues = parseCsvInt(values);

			int x = parsedValues[0];
			int y = parsedValues[1];
			int tileset_row = parsedValues[2];
			int tileset_column = parsedValues[3];

			auto tile = SKO_Map::Tile(x, y, tileset_row, tileset_column);
			tileLayer[tilesetKey].insert(tile);
		}

	}
}

void SKO_Map::Reader::loadBackgroundTiles(SKO_Map::Map * map, INIReader mapIni)
{
	// load background tiles
	loadTileLayer("background_tiles", map->backgroundTiles, mapIni);
}

void SKO_Map::Reader::loadBackgroundMaskTiles(SKO_Map::Map* map, INIReader mapIni)
{
	// load background tiles
	loadTileLayer("background_mask_tiles", map->backgroundMaskTiles, mapIni);
}

void SKO_Map::Reader::loadFringeTiles(SKO_Map::Map * map, INIReader mapIni)
{
	// load background tiles
	loadTileLayer("fringe_tiles", map->fringeTiles, mapIni);
}

void SKO_Map::Reader::loadFringeMaskTiles(SKO_Map::Map* map, INIReader mapIni)
{
	// load background tiles
	loadTileLayer("fringe_mask_tiles", map->fringeMaskTiles, mapIni);
}

void SKO_Map::Reader::loadCollisionRects(SKO_Map::Map * map, INIReader mapIni)
{
	for (int i = 0; ; ++i)
	{
		std::stringstream rectTag;
		rectTag << "rect" << i;
		std::string values = mapIni.Get("collision_rects", rectTag.str(), "null");

		if (values == "null")
		{
			break;
		}

		auto parsedValues = parseCsvInt(values);

		int x = parsedValues[0];
		int y = parsedValues[1];
		int w = parsedValues[2];
		int h = parsedValues[3];

		// Add background tile to map collection
		auto collisionRect = SKO_Map::CollisionRect(x, y, w, h);

		map->collisionRects.insert(collisionRect);
	}

		
	
}

void SKO_Map::Reader::loadPortals(SKO_Map::Map * map, INIReader mapIni)
{
	// load portals
	int num_portals = mapIni.GetInteger("meta", "portals", 0);
	printf("num_portals is %i", num_portals);

	for (int i = 0; i < num_portals; ++i)
	{
		std::stringstream ss;
		ss << "portal" << i;

		// load portal parameters
		SKO_Portal * portal = new SKO_Portal();
		std::string values = mapIni.Get(ss.str(), "coords", "null");

		if (values == "null")
		{
			break;
		}

		auto parsedValues = parseCsvInt(values);

		portal->x = parsedValues[0];
		portal->y = parsedValues[1];
		portal->w = parsedValues[2];
		portal->h = parsedValues[3]; 
		portal->mapId = mapIni.GetInteger(ss.str(), "map_id", 0);
		portal->spawn_x = mapIni.GetInteger(ss.str(), "spawn_x", 0);
		portal->spawn_y = mapIni.GetInteger(ss.str(), "spawn_y", 0);
		portal->level_required = mapIni.GetInteger(ss.str(), "level_required", 0);

		// Add portal to map collection
		map->portals.push_back(portal);
	}
}

void SKO_Map::Reader::loadSigns(SKO_Map::Map * map, INIReader mapIni)
{
	//load signs
	int num_signs = mapIni.GetInteger("meta", "signs", 0);
	printf("num_signs is %i\n", num_signs);

	for (int i = 0; i < num_signs; ++i) 
	{
		std::stringstream ss; 
		ss << "sign" << i;    

		SKO_Sign *sign = new SKO_Sign(); 
		std::string values = mapIni.Get(ss.str(), "coords", "null");

		if (values == "null")
		{
			break;
		}

		auto parsedValues = parseCsvInt(values);

		sign->x = parsedValues[0];
		sign->y = parsedValues[1];
		sign->w = parsedValues[2];
		sign->h = parsedValues[3];

		printf("Sign[sign].NUM_LINES is %i\n", SKO_Sign::NUM_LINES);

		//get all the lines of the text
		for (int line = 0; line < SKO_Sign::NUM_LINES; line++) {
			std::stringstream ss1;
			ss1 << "line" << line;
			auto txt = mapIni.Get(ss.str(), ss1.str(), "");

			// Add new dialogue lines one at a time
			// TODO - save Font in .ini file for this object
			auto textComponent = new OPI_Text::TextComponent(txt, OPI_Text::FontManager::getFont("RobotoMono-Regular"));

			sign->lines.push_back(textComponent);
			printf("%s is: %s\n", ss1.str().c_str(), txt.c_str());
		}

		map->signs.push_back(sign);
	}
}

void SKO_Map::Reader::loadEnemies(SKO_Map::Map * map, INIReader mapIni)
{
	// load enemies
	int num_enemies = mapIni.GetInteger("meta", "enemies", 0);
	printf("num_enemies is %i", num_enemies);

	for (int i = 0; i < num_enemies; ++i)
	{
		std::stringstream ss;
		ss << "enemy" << i;

		// load portal parameters
		SKO_Enemy * enemy = new SKO_Enemy();
		enemy->type = mapIni.Get(ss.str(), "sprite", "");

		if (enemy->type == "")
		{
			break;
		}

		enemy->spawn_x = mapIni.GetInteger(ss.str(), "spawn_x", 0);
		enemy->spawn_y = mapIni.GetInteger(ss.str(), "spawn_y", 0);

		enemy->hp_max = mapIni.GetInteger(ss.str(), "hp", 0);
		enemy->strength = mapIni.GetInteger(ss.str(), "strength", 0);
		enemy->defense = mapIni.GetInteger(ss.str(), "defense", 0);
		enemy->xp = mapIni.GetInteger(ss.str(), "xp", 0);
		
		enemy->lootAmount = mapIni.GetInteger(ss.str(), "loots_dropped", 0);

		int loot_count = mapIni.GetInteger(ss.str(), "loot_count", 0);

		for (int loot = 0; loot < loot_count; loot++)
		{
			std::stringstream ss1;
			ss1 << "loot" << loot << "_item";

			std::stringstream ss2;
			ss2 << "loot" << loot << "_amount";

			std::stringstream ss3;
			ss3 << "loot" << loot << "_chance";

			int loot_item =	mapIni.GetInteger(ss.str(), ss1.str(), 0);
			int loot_amount = mapIni.GetInteger(ss.str(), ss2.str(), 0);
			int loot_chance = mapIni.GetInteger(ss.str(), ss3.str(), 0);

			enemy->addLoot(loot_item, loot_amount, loot_chance);
		}

		enemy->x = enemy->spawn_x;
		enemy->y = enemy->spawn_y;
		enemy->hp = enemy->hp_max;

		// Add portal to map collection
		map->enemies.push_back(enemy);
	}
}

void SKO_Map::Reader::loadStalls(SKO_Map::Map * map, INIReader mapIni)
{
	//load shops
	int num_stalls = mapIni.GetInteger("meta", "stalls", 0);
	printf("num_stalls is %i\n", num_stalls);

	for (int i = 0; i < num_stalls; ++i) 
	{
		std::stringstream ss1;
		ss1 << "stall" << i;
		std::string stallStr = ss1.str();

		SKO_Stall * stall = new SKO_Stall();
		std::string values = mapIni.Get(stallStr, "coords", "null");

		if (values == "null")
		{
			break;
		}

		auto parsedValues = parseCsvInt(values);

		stall->x = parsedValues[0];
		stall->y = parsedValues[1];
		stall->w = parsedValues[2];
		stall->h = parsedValues[3];

		stall->type = mapIni.Get(stallStr, "type", "bank");

		if (stall->type == "shop")
		{
			stall->shopId = mapIni.GetInteger(stallStr, "shopId", 0);
		}
		map->stalls.push_back(stall);
	}
}

void SKO_Map::Reader::loadShops(SKO_Map::Map * map, INIReader mapIni)
{
	//load shops
	int num_shops = mapIni.GetInteger("meta", "shops", 0);
	printf("num_shops is %i\n", num_shops);

	for (int i = 0; i < num_shops; ++i)
	{
		SKO_Shop *shop = new SKO_Shop();
		std::stringstream ss1;
		ss1 << "shop" << i;
		std::string shopStr = ss1.str();

		// Get shop name
		shop->name = mapIni.Get(shopStr.c_str(), "name", "null");
		
		//loop all the X Y places
		for (int x = 0; x < 6; x++)
		{
			for (int y = 0; y < 4; y++)
			{
				std::stringstream ss2, ss3;
				ss2 << "item" << "_" << "x" << x << "_" << "y" << y;
				ss3 << "price" << "_" << "x" << x << "_" << "y" << y;

				//x  y  (item[0], price[1])
				std::string itemStr = ss2.str().c_str();
				std::string priceStr = ss3.str().c_str();

				//now load from config file the items and prices
				shop->item[x][y][0]
					= mapIni.GetInteger(shopStr.c_str(), itemStr.c_str(), 0);
				shop->item[x][y][1]
					= mapIni.GetInteger(shopStr.c_str(), priceStr.c_str(), 0);
			}
		}
		
		map->shops.push_back(shop);
	}
}

void SKO_Map::Reader::loadTargets(SKO_Map::Map * map, INIReader mapIni)
{
	//load targets
	int num_targets = mapIni.GetInteger("meta", "targets", 0);
	printf("num_targets is %i\n", num_targets);

	for (int i = 0; i < num_targets; ++i) {
		std::stringstream ss1;
		ss1 << "target" << i;
		std::string targetStr = ss1.str();

		SKO_Target *target = new SKO_Target();
		target->picId = mapIni.GetInteger(targetStr, "type", 0);
		std::string values = mapIni.Get(targetStr, "coords", "null");

		if (values == "null")
		{
			break;
		}

		auto parsedValues = parseCsvInt(values);

		target->x = parsedValues[0];
		target->y = parsedValues[1];
		target->w = parsedValues[2];
		target->h = parsedValues[3];
		map->targets.push_back(target);
	}
}

void SKO_Map::Reader::loadNpcs(SKO_Map::Map * map, INIReader mapIni)
{
	//load npcs
	int num_npcs = mapIni.GetInteger("meta", "npcs", 0);
	printf("num_npcs is %i\n", num_npcs);

	for (int i = 0; i < num_npcs; ++i) 
	{
		std::stringstream ss1;
		ss1 << "npc" << i;
		std::string npcStr = ss1.str();

		SKO_Npc * npc = new SKO_Npc();
		npc->name = mapIni.Get(npcStr, "name", "Noname");
		npc->sprite = mapIni.Get(npcStr, "sprite", "stickman");
		npc->x = npc->spawn_x = mapIni.GetInteger(npcStr, "x", 0);
		npc->y = npc->spawn_y = mapIni.GetInteger(npcStr, "y", 0);
		npc->finalPage = mapIni.Get(npcStr, "final", "");
		int num_pages = mapIni.GetInteger(npcStr, "pages", 0);
		npc->quest = mapIni.GetInteger(npcStr, "quest", -1); //-1 for non quest NPCs

		//get all the lines of the text
		for (int page = 0; page < num_pages; page++)
		{
			// Add new page to NPC
			npc->pages.push_back(new DialogPage());

			for (int line = 0; line < 10; line++) // TODO change this to use number of lines
			{
				std::stringstream ss1;
				ss1 << "page_" << page << "_";
				ss1 << "line_" << line;
				std::string txt = mapIni.Get(npcStr, ss1.str(), "");

				// Add new dialogue lines one at a time
				auto textComponent = new OPI_Text::TextComponent(txt, OPI_Text::FontManager::getFont("RobotoMono-Regular"));
				npc->pages[page]->lines.push_back(textComponent);

				printf("NPC %s is: %s\n", ss1.str().c_str(), txt.c_str());
			}
		}

		map->npcs.push_back(npc);
	}
}

void SKO_Map::Reader::loadMapName(SKO_Map::Map* map, INIReader mapIni)
{
	// load enemies
	map->name = mapIni.Get("meta", "name", "Untitled Map");
}