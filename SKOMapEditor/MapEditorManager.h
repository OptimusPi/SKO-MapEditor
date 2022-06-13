#ifndef __SKO_MAPEDITOR_MANAGER_H_
#define __SKO_MAPEDITOR_MANAGER_H_

#include <string>

#include "SKO_MapTile.h"
#include "SKO_Map.h"
#include "OPI_Renderer.h"
#include "GuiManager.h"
#include "OPI_Clock.h"
#include "MapEditorGui.h"
#include "SKO_MapReader.h"
#include "SKO_MapTileset.h"
#include "TileLayerType.h"
#include "MapEditorEditingMode.h"
#include "MapEditorBrushModifier.h"
#include <unordered_set>
#include <set>
#include <map>
#include "SKO_MapCollisionRect.h"
#include "OPI_Stack.h"
#include "MapEditorGui.h"


namespace SKO_MapEditor
{
	//TODO Singleton
	class MapEditorGui;
	class Manager
	{
	public:
		Manager(OPI_Renderer * renderer);
		virtual ~Manager();
		void saveMap();
		void saveMap(std::string filePath);
		void loadMap(std::string filePath);

		// Draw: sky, map layers, and player test object
		void drawTiles(int camera_x, int camera_y, const OPI_VectorSet<SKO_Map::Tile>& tiles, const SKO_Map::Tileset &tileset);
		void drawSelectedTiles(int camera_x, int camera_y, int offset_x, int offset_y, const OPI_VectorSet<SKO_Map::Tile>& tiles, const SKO_Map::Tileset& tileset);
		void drawTileLayer(int camera_x, int camera_y, SKO_MapEditor::TileLayerType layer);
		void drawTileLayer(int camera_x, int camera_y, const std::map<std::string, OPI_VectorSet<SKO_Map::Tile>>& tileLayer);
		void DrawGameScene(int camera_x, int camera_y);
		void DrawEditorOperations(int camera_x, int camera_y);

		// Handle input
		bool HandleInput();

		// Process logic loop
		// return value: quit? true : false
		bool processLoop();

		void handleWindowEvent(SDL_WindowEvent windowEvent);
		void handleKeyUp(SDL_Keycode keyCode);
		void handleKeyDown(SDL_Keycode keyCode);
		void panCamera(int newCameraX, int newCameraY);

		// Load map editor images such as tilesets, stickman, background
		void loadImages();
		void loadTilesets();

		// Map editing-specific gui
		void addTilesetSelector();
		void addTileHotBar();
		void addLayersHotbar();

		//TODO hmm
		OPI_Gui::GuiManager* gui;
	private:
		SKO_Map::Reader *mapReader = nullptr;
		SDL_Event event;
		MapEditorGui *mainMenuGui = nullptr;
		OPI_Renderer * renderer = nullptr;
		OPI_Stack<SKO_Map::Map> mapStack;
		SKO_Map::Map& map()
		{
			return mapStack.current();
		}
		float camera_x = 0, camera_y = 0;
		int cursor_x = 0, cursor_y = 0;
		SKO_Map::Tile current_tile = SKO_Map::Tile();
		SKO_Map::CollisionRect current_rect;
		
		std::map<std::string, SKO_Map::Tileset*> tilesets;
		std::vector<std::string> tilesetKeys;
		std::map<std::string, OPI_Gui::SelectorRect*> tileSelectorElements;
		std::map<std::string, OPI_Gui::Panel*> tilesetSelectorElements;
		std::map<std::string, OPI_Gui::TextLabel*> tilesetSelectorLabels;
		SKO_Map::Tileset* current_tileset = nullptr;
		short current_tileset_key = 0;
		int current_tileset_row = 0;
		int current_tileset_column = 0;
		int current_tileset_rows = 0;
		int current_tileset_columns = 0;
		void seekTile(int x, int y);
		void seekTiles(int x, int y);
		void seekTilesetLeft();
		void seekTilesetRight();
		void deleteTileFromLayer(TileLayerType layer, std::string tilesetKey, SKO_Map::Tile& tile);
		void addTileToLayer(TileLayerType layer, std::string tilesetKey, SKO_Map::Tile& tile);
		int num_tile_images = 0;

		//images
		OPI_Image background;
		OPI_Image stickman_img;
		OPI_Gui::Panel* panelTilesetPicker;
		OPI_Gui::Panel* hotbarPanel;
		

		//mouse buttons
		bool RCLICK = false;
		bool LCLICK = false;
		bool SHIFT_HELD = false;
		bool CTRL_HELD = false;
		bool stickman_toggle = false;
		unsigned long long coordsTicker = 0;
		unsigned long long metaTicker = 0;
		unsigned long long fpsTicker = 0;
		unsigned short int fpsCounter = 0;

		SDL_Rect stickman;// TODO bruh
		float x_speed, y_speed;
		const float GRAVITY = 0.175;
		bool RIGHT = false, LEFT = false;

		// Simulate Physics of test stickman
		bool isBlocked(SDL_Rect rect);
		void physics();
		void graphics();

		// Helpers for the magic brush feature
		bool magicMarkerRunning = false;
		int magicMarker_x = 0;
		int magicMarker_y = 0;
		int currentMagicTilesWidth = 0;
		int currentMagicTilesHeight = 0;
		OPI_VectorSet<SKO_Map::Tile> magicTiles;

		// Helpers for the edit selection functions
		bool tileEditSelectionRunning = false;
		bool collisionEditSelectionRunning = false;
		int editRectMarker_x = -1;
		int editRectMarker_y = -1;
		int copyRectMarker_x = -1;
		int copyRectMarker_y = -1;
		std::set<SKO_Map::CollisionRect> selectedCollisionRects;
		std::map<std::string, OPI_VectorSet<SKO_Map::Tile>> selectedTiles;
		std::map<std::string, OPI_VectorSet<SKO_Map::Tile>>& getTileLayer(SKO_MapEditor::TileLayerType layer);
		void cutTiles();
		void cutCollisions();
		void pasteTiles();
		void pasteCollisions();

		// Anchor for the collision rectangle drawer
		bool collisionRectRunning = false;
		int collisionRectMarker_x = -1;
		int collisionRectMarker_y = -1;
		void snapCollisionRect(SKO_Map::CollisionRect* rect, int gridSize = 32);

		bool dragOperationRunning = false;

		// TODO - move somewhere to an editor state class maybe
		// todo refactor
		EditingMode editor__mode = EditingMode::Tile_MagicFill;
		BrushModifier editor__brushModifier = BrushModifier::None;
		TileLayerType editor__currentLayer = TileLayerType::Background;

		void addTile();
		void handleMouseClick_addTile();
		void handleMouseClick_addMagicMarker();
		void handleMouseClick_editTile();
		void handleMouseClick_deleteTile();
		void handleMouseClick_addCollisionRect();
		void handleMouseClick_editCollisionRect();
		void handleMouseClick_deleteCollisionRect();
		void handleMouseClick_Testing_Mode();

		void handleMouseRelease_addCollisionRect();
		void handleMouseRelease_editCollisionRect();
		void handleMouseRelease_editTile();
		void handleMouseRelease_addTilesMagicFill();
		bool handleMouseMotion_Throttle();
		void handleMouseMotion_addTile();
		void handleMouseMotion_editTile();
		void handleMouseMotion_deleteTile();
		void handleMouseMotion_addCollisionRect();
		void handleMouseMotion_editCollisionRect();
		void handleMouseMotion_deleteCollisionRect();
		void handleMouseMotion_addTilesMagicFill();

		unsigned long long int throttle_mouse_motion_ticker = OPI_Clock::milliseconds();
		unsigned long long int throttle_mouse_motion_last_time = 0;
		unsigned long long int throttle_time = 15;
		
	};
}

#endif

