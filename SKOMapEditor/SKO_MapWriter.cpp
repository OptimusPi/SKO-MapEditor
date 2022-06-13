#include "SKO_MapWriter.h"

void SKO_Map::Writer::saveMap(SKO_Map::Map * map, std::map<std::string, SKO_Map::Tileset*> tilesets)
{
	// Open the output, overwriting the entire file.
	std::ofstream* file = new std::ofstream(map->filePath); // TODO - remove this, tempoorarily being used to convert old maps to new format... +".2.ini");

	// Save map header and count
	saveMetaData(map, file);

	// Save dimensions and graphics
	*file << std::endl;
	*file << "# Map Dimensions: Background Tiles, Fringe Tiles, and Collision Rectangles" << std::endl;
	*file << std::endl;
	saveBackgroundTiles(map, tilesets, file);
	saveBackgroundMaskTiles(map, tilesets, file);
	saveFringeTiles(map, tilesets, file);
	saveFringeMaskTiles(map, tilesets, file);
	saveCollisionRects(map, file);

	// Save game objects
	*file << std::endl;
	*file << "# Game Content: Portals, Signs, Enemies, Stalls, Shops, Targets, Npcs" << std::endl;
	*file << std::endl;
	savePortals(map, file);
	saveSigns(map, file);
	saveEnemies(map, file);
	saveStalls(map, file);
	saveShops(map, file);
	saveTargets(map, file);
	saveNpcs(map, file);
}

void SKO_Map::Writer::saveMetaData(SKO_Map::Map * map, std::ofstream * file)
{
	// Map Version
	*file << "# Stick Knights Online Map File" << std::endl;
	*file << "# This is an auto-generated file" << std::endl;
	*file << "# Any added comments will be removed if this file is opened and saved in the map editor." << std::endl;
	*file << std::endl;
	*file << "[version]" << std::endl;
	*file << "version = " << VERSION_MAJOR << "." << VERSION_MINOR << std::endl;
	*file << std::endl;

	// Count of all the types
	*file << "[meta]" << std::endl;
	*file << "name = " << map->name << std::endl;
	*file << "background_tiles = " << countTiles(map->backgroundTiles) << std::endl;
	*file << "background_mask_tiles = " << countTiles(map->backgroundMaskTiles) << std::endl;
	*file << "fringe_tiles = " << countTiles(map->fringeTiles) << std::endl;
	*file << "fringe__mask_tiles = " << countTiles(map->fringeMaskTiles) << std::endl;
	*file << "collision_rects = " << map->collisionRects.size() << std::endl;
	*file << "portals = " << map->portals.size() << std::endl;
	*file << "signs = " << map->signs.size() << std::endl;
	*file << "enemies = " << map->enemies.size() << std::endl;
	*file << "stalls = " << map->stalls.size() << std::endl;
	*file << "shops = " << map->shops.size() << std::endl;
	*file << "targets = " << map->targets.size() << std::endl;
	*file << "npcs = " << map->npcs.size() << std::endl;
}

int SKO_Map::Writer::countTiles(std::map<std::string, OPI_VectorSet<SKO_Map::Tile>> tiles)
{
	int tileCount = 0;

	for (auto it = tiles.begin(); it != tiles.end(); ++it)
	{
		auto tileset = it->first;
		auto tiles = it->second;

		tileCount += tiles.size();
	}

	return tileCount;
}


void SKO_Map::Writer::saveTileLayer(std::string name, std::map<std::string, OPI_VectorSet<Tile>> tileLayer, std::map<std::string, SKO_Map::Tileset*> tilesets, std::ofstream* file)
{
	for (auto it = tileLayer.begin(); it != tileLayer.end(); ++it)
	{
		auto tilesetKey = it->first;
		auto tiles = it->second;

		if (tiles.size() > 0)
		{
			*file << "; " << tilesets[tilesetKey]->name << std::endl;
			*file << "[" << name << "_" << tilesetKey << "]" << std::endl;
		}

		int tileI = 0;
		for (auto tile : tiles)
		{
			*file << "tile" << tileI << " = "
				<< tile.x << ","
				<< tile.y << ","
				<< tile.tileset_row << ","
				<< tile.tileset_column << std::endl;

			++tileI;
		}

		*file << std::endl;
	}
	*file << std::endl;
}

void SKO_Map::Writer::saveBackgroundTiles(SKO_Map::Map * map, std::map<std::string, SKO_Map::Tileset*> tilesets, std::ofstream * file)
{
	saveTileLayer("background_tiles", map->backgroundTiles, tilesets, file);
}

void SKO_Map::Writer::saveBackgroundMaskTiles(SKO_Map::Map* map, std::map<std::string, SKO_Map::Tileset*> tilesets, std::ofstream* file)
{
	saveTileLayer("background_mask_tiles", map->backgroundMaskTiles, tilesets, file);
}

void SKO_Map::Writer::saveFringeTiles(SKO_Map::Map * map, std::map<std::string, SKO_Map::Tileset*> tilesets, std::ofstream * file)
{
	saveTileLayer("fringe_tiles", map->fringeTiles, tilesets, file);
}

void SKO_Map::Writer::saveFringeMaskTiles(SKO_Map::Map* map, std::map<std::string, SKO_Map::Tileset*> tilesets, std::ofstream* file)
{
	saveTileLayer("fringe_mask_tiles", map->fringeMaskTiles, tilesets, file);
}

void SKO_Map::Writer::saveRects(std::string name, std::set<SKO_Map::CollisionRect> rects, std::ofstream* file)
{
	if (rects.size() < 1)
	{
		return;
	}

	*file << "[" << name << "]" << std::endl;

	int rectI = 0;
	for (auto rect: rects)
	{
		*file << "rect" << rectI << " = "
			<< rect.x << ","
			<< rect.y << ","
			<< rect.w << ","
			<< rect.h << std::endl;

		++rectI;
	}
	*file << std::endl;
}

void SKO_Map::Writer::saveCollisionRects(SKO_Map::Map * map, std::ofstream * file)
{
	saveRects("collision_rects", map->collisionRects, file);
}

void SKO_Map::Writer::savePortals(SKO_Map::Map * map, std::ofstream * file)
{
	for (int i = 0; i < map->portals.size(); ++i)
	{
		*file << "[portal" << i << "]" << std::endl;
		*file << "coords = " 
			<< map->portals[i]->x << ","
			<< map->portals[i]->y << ","
			<< map->portals[i]->w << ","
			<< map->portals[i]->h << std::endl;
		*file << "map_id = " << (int)map->portals[i]->mapId << std::endl; // TODO READER CHANGE - need to use map guid
		*file << "spawn_x = " << map->portals[i]->spawn_x << std::endl;
		*file << "spawn_y = " << map->portals[i]->spawn_y << std::endl; 
		*file << "level_required = " << map->portals[i]->level_required << std::endl;
		*file << std::endl;
	}
	*file << std::endl;
}

void SKO_Map::Writer::saveSigns(SKO_Map::Map * map, std::ofstream * file)
{
	for (int i = 0; i < map->signs.size(); ++i)
	{
		*file << "[sign" << i << "]" << std::endl;
		*file << "coords = "
			<< map->signs[i]->x << ","
			<< map->signs[i]->y << ","
			<< map->signs[i]->w << ","
			<< map->signs[i]->h << std::endl;

		for (int line = 0; line < map->signs[i]->lines.size(); line++)
		{
			*file << "line" << line << " = \"" << map->signs[i]->lines[line]->content << std::endl;
		}
		*file << std::endl;
	}
	*file << std::endl;
}

void SKO_Map::Writer::saveEnemies(SKO_Map::Map * map, std::ofstream * file)
{
	// Save enemy spawn points. 
	// Actual enemy type definitions are in a separate file 
	// TODO What file? right now it's DAT/enemySprites.ini
	for (int i = 0; i < map->enemies.size(); ++i)
	{
		*file << "[enemy" << i << "]" << std::endl;
		*file << "sprite = " << map->enemies[i]->type << std::endl;
		*file << "spawn_x = " << map->enemies[i]->spawn_x << std::endl;
		*file << "spawn_y = " << map->enemies[i]->spawn_y << std::endl;
		*file << std::endl;

		*file << "hp = " << map->enemies[i]->hp << std::endl;
		*file << "strength = " << map->enemies[i]->strength << std::endl;
		*file << "defense = " << map->enemies[i]->defense << std::endl;
		*file << "xp = " << map->enemies[i]->xp << std::endl;
		*file << std::endl;

		*file << "loots_dropped = " << map->enemies[i]->lootAmount << std::endl;
		unsigned int loot_count = map->enemies[i]->lootItem.size();
		*file << "loot_count = " << loot_count << std::endl;
		*file << std::endl;

		for (int loot = 0; loot < loot_count; loot++)
		{
			*file << "loot" << loot << "_item = " << map->enemies[i]->lootItem[loot].item_id << std::endl;
			*file << "loot" << loot << "_amount = " << map->enemies[i]->lootItem[loot].amount << std::endl;
			*file << "loot" << loot << "_chance = " << (int)map->enemies[i]->lootItem[loot].chance << std::endl;
			*file << std::endl;
		}
	}
	*file << std::endl;
}

void SKO_Map::Writer::saveStalls(SKO_Map::Map * map, std::ofstream * file)
{
	for (int i = 0; i < map->stalls.size(); ++i)
	{
		*file << "[stall" << i << "]" << std::endl;
		*file << "type = " << map->stalls[i]->type << std::endl;

		if (map->stalls[i]->type == "shop")
		{
			*file << "shopId = " << map->stalls[i]->shopId << std::endl;
		}

		*file << "coords = "
			<< map->stalls[i]->x << ","
			<< map->stalls[i]->y << ","
			<< map->stalls[i]->w << ","
			<< map->stalls[i]->h << std::endl;
		*file << std::endl;
	}
	*file << std::endl;
}

void SKO_Map::Writer::saveShops(SKO_Map::Map * map, std::ofstream * file)
{
	for (int i = 0; i < map->shops.size(); ++i) 
	{
		*file << "[shop" << i << "]" << std::endl;

		// title of the shop
		*file << "name = " << map->shops[i]->name << std::endl;

		// Items in the shop
		for (int itemX = 0; itemX < 6; itemX++)
		{
			for (int itemY = 0; itemY < 4; itemY++)
			{
				if (map->shops[i]->item[itemX][itemY][0] > 0)
				{
					// TODO make function to clean these complex writes up! 
					*file << "item"
						<< "_x" << itemX
						<< "_y" << itemY
						<< " = " << map->shops[i]->item[itemX][itemY][0]
						<< std::endl;
					*file << "price"
						<< "_x" << itemX
						<< "_y" << itemY
						<< " = " << map->shops[i]->item[itemX][itemY][1]
						<< std::endl;
				}
			}
		}
		*file << std::endl;
	}
	*file << std::endl;
}

void SKO_Map::Writer::saveTargets(SKO_Map::Map * map, std::ofstream * file)
{
	for (int i = 0; i < map->targets.size(); ++i)
	{
		*file << "[target" << i << "]" << std::endl;
		*file << "coords = "
			<< map->targets[i]->x << ","
			<< map->targets[i]->y << ","
			<< map->targets[i]->w << ","
			<< map->targets[i]->h << std::endl;
		*file << "type = " << map->targets[i]->type << std::endl;
		*file << std::endl;
	}
	*file << std::endl;
}

void SKO_Map::Writer::saveNpcs(SKO_Map::Map * map, std::ofstream * file)
{
	for (int i = 0; i < map->npcs.size(); ++i) 
	{
		*file << "[npc" << i << "]" << std::endl;

		// Name and look of the npc
		*file << "name = " << map->npcs[i]->name << std::endl;
		*file << "sprite = " << map->npcs[i]->sprite << std::endl;

		// Spawn position
		*file << "x = " << map->npcs[i]->x << std::endl;
		*file << "y = " << map->npcs[i]->y << std::endl;

		// number of quest pages
		*file << "pages = " << map->npcs[i]->pages.size() << std::endl;

		// Quest Id
		*file << "quest = " << map->npcs[i]->quest << std::endl;
		*file << std::endl;

		// Pages of quest dialog
		for (int page = 0; page < map->npcs[i]->pages.size(); page++)
		{
			// Lines per page
			for (int line = 0; line < map->npcs[i]->pages[page]->lines.size(); line++)
			{
				// print all lines even if blank
				*file << "page_" << page << "_"
					<< "line_" << line
					<< "= \"" << map->npcs[i]->pages[page]->lines[line]->content << std::endl;
			}
		}

		// Final Page or Question in the dialog
		*file << "final = \"" << map->npcs[i]->finalPage << std::endl;
	}
	*file << std::endl;
}
