#ifndef __SKO_MAP_COLLISIONRECT_
#define __SKO_MAP_COLLISIONRECT_

#include <tuple>


namespace SKO_Map
{
	class CollisionRect
	{
	public:

		CollisionRect();
		CollisionRect(int x, int y, int w, int h);

		int x = 0;
		int y = 0;
		int w = 0;
		int h = 0;

		constexpr bool operator<(const CollisionRect& rectB) const
		{
			if (x != rectB.x)
			{
				return x > rectB.x;
			}
			else if (y != rectB.y)
			{
				return y > rectB.y;
			}
			else if (w != rectB.w)
			{
				return w > rectB.w;
			}
			else if (h != rectB.h)
			{
				return h > rectB.h;
			}

			return false;
		}

		constexpr bool operator==(const CollisionRect& rectB) const
		{
			return x == rectB.x && y == rectB.y && w == rectB.w && h == rectB.h;
		}

	};
}

#endif
