#ifndef __SKO_MAPEDITOR_BrushModifier_
#define __SKO_MAPEDITOR_BrushModifier_

namespace SKO_MapEditor
{
	enum class BrushModifier
	{
		// Brush modifier
		None, // default behavior
		CollisionRect, // snap collision rect to tiles that were drawn
	};
}

#endif