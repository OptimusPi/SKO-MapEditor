#ifndef __SKO_MAPEDITOR_EditingMode_
#define __SKO_MAPEDITOR_EditingMode_

namespace SKO_MapEditor
{
	enum class EditingMode
	{
		// Editing map tiles in any layer
		Tile_Brush,
		Tile_MagicFill,
		Tile_Edit,
		Tile_Copy,
		Tile_Paste,
		Tile_Delete,

		// Editing collision rectangles
		Collision_Add,
		Collision_Edit,
		Collision_Copy,
		Collision_Delete,

		// Editing game content objects
		Npc_Add,
		Npc_Edit,
		Npc_Delete,
		Enemy_Add,
		Enemy_Edit,
		Enemy_Delete,
		Target_Add,
		Target_Edit,
		Target_Delete,
		Portal_Add,
		Portal_Edit,
		Portal_Delete,

		// Testing the map with a movable sprite
		Testing_Mode,
	};
}

#endif