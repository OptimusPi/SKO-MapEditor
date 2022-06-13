#ifndef __SKOMAPTILE_H_
#define __SKOMAPTILE_H_

#include "OPI_Image.h"
#include "SKO_MapTileset.h"

namespace SKO_Map
{
	class Tile
	{
	public:
		Tile();
		Tile(int x, int y, unsigned int tileset_row, unsigned int tileset_column);

		virtual ~Tile();
		int x = 0;
		int y = 0;
		unsigned int tileset_row = 0;
		unsigned int tileset_column = 0;


		constexpr bool operator<(const Tile& tileB) const
		{
			if (x != tileB.x)
			{
				return x > tileB.x;
			}
			else if (y != tileB.y)
			{
				return y > tileB.y;
			}
			else if (tileset_row != tileB.tileset_row)
			{
				return tileset_row > tileB.tileset_row;
			}
			else if (tileset_column != tileB.tileset_column)
			{
				return tileset_column > tileB.tileset_column;
			}

			return false;
			//return std::tie(tileA.x, tileA.y, tileA.tileset_column, tileA.tileset_row) 
			//	< std::tie(tileB.x, tileB.y, tileB.tileset_column, tileB.tileset_row);
		}
		
		constexpr bool operator==(const Tile& tileB) const
		{
			return !(tileB < *this || *this < tileB);
		}
	};
}


#endif