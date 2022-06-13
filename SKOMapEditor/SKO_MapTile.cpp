#include "SKO_MapTile.h"

SKO_Map::Tile::Tile()
{

}

SKO_Map::Tile::Tile(int x, int y, unsigned int tileset_row, unsigned int tileset_column)
{
	this->x = x;
	this->y = y;
	this->tileset_row = tileset_row;
	this->tileset_column = tileset_column;
}

SKO_Map::Tile::~Tile()
{
}

