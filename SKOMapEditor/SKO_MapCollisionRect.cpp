#include "SKO_MapCollisionRect.h"

SKO_Map::CollisionRect::CollisionRect()
{
}

SKO_Map::CollisionRect::CollisionRect(int x, int y, int w, int h)
{
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
}
