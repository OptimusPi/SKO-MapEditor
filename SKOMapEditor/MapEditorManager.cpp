#include "MapEditorManager.h"
#include "SKO_MapWriter.h"
#include "Font.h"
#include "SelectorRect.h"
#include "TileLayerType.h"
#include <mutex>
#include <execution>
#include <functional>

SKO_MapEditor::Manager::Manager(OPI_Renderer * renderer)
{
	// Initialize map loader
	auto map = new SKO_Map::Map();
	this->mapStack.init(*map);
	this->renderer = renderer;
	this->gui = OPI_Gui::GuiManager::getInstance();
	this->mainMenuGui = new MapEditorGui(this);

	//TODO move to another class like MapEditorGui
	this->loadImages();
	this->loadTilesets();
	this->addTileHotBar();
	this->addLayersHotbar();
	this->addTilesetSelector();

	this->mapReader = new SKO_Map::Reader(this->tilesets);
}

SKO_MapEditor::Manager::~Manager()
{

}
 
void SKO_MapEditor::Manager::loadImages()
{
	this->background.setImage("IMG/back.png");
	this->stickman_img.setImage("IMG/stickman.png");
	
}

void SKO_MapEditor::Manager::loadTilesets()
{
	std::string filePath = "DAT/tilesets.ini";

	// Open the file to read
	INIReader tilesetIni(filePath);
	if (tilesetIni.ParseError() < 0)
	{
		std::string error = "SKO_MapEditor::Manager::loadTilesets() Failed to load INI file: " + filePath;
		throw new std::ios_base::failure(error);
	}

	// load background tiles
	int tilesetsCount = tilesetIni.GetInteger("tilesets", "count", 0);

	for (int i = 0; i < tilesetsCount; ++i)
	{
		std::stringstream ss;
		ss << "tileset" << i;
		std::string name = tilesetIni.Get(ss.str(), "name", "");
		std::string key = tilesetIni.Get(ss.str(), "key", "00000000-0000-0000-0000-000000000000");
		std::string filePath = tilesetIni.Get(ss.str(), "filepath", "");
		unsigned short tile_width = tilesetIni.GetInteger(ss.str(), "tile_width", 32);
		unsigned short tile_height = tilesetIni.GetInteger(ss.str(), "tile_height", 32);
		unsigned char rows = tilesetIni.GetInteger(ss.str(), "rows", 32);
		unsigned char columns = tilesetIni.GetInteger(ss.str(), "columns", 32);

		try
		{
			auto image = new OPI_Image(filePath);
			auto tileset = new SKO_Map::Tileset(image, name, key, tile_width, tile_height, rows, columns);
			tilesets.insert({ tileset->key, tileset });
			tilesetKeys.push_back(tileset->key);
		}
		catch (...)
		{
			continue;
		}
	}

	this->current_tileset = tilesets[tilesetKeys[this->current_tileset_key]];
}

void SKO_MapEditor::Manager::addTilesetSelector()
{
	this->panelTilesetPicker = new OPI_Gui::Panel(OPI_Gui::PanelThemeType::GridRect, "ice", 0, 0, 350, 314);

	int padding = 32;

	OPI_Gui::Element* tilesetElement = nullptr;

	for (auto tileset : this->tilesets)
	{
		std::string tilesetKey = tileset.first;
		SKO_Map::Tileset* tilesetValue = tileset.second;

		auto tilesetLabel = new OPI_Gui::TextLabel(padding / 2, padding / 2, new OPI_Text::TextComponent(tilesetValue->name, OPI_Text::FontManager::getFont("RobotoMono-Regular"), 18));
		tilesetLabel->isVisible = false;

		auto tilesetElement = new OPI_Gui::Panel(OPI_Gui::PanelThemeType::Image, tilesetValue->image->fileLocation, padding / 2, padding / 2 + tilesetLabel->height);
		tilesetElement->isVisible = false;

		auto tileSelectorElement = new OPI_Gui::SelectorRect(OPI_Gui::SelectorRectThemeType::BorderFill,
			0, 0,
			tilesetValue->tile_width,
			tilesetValue->tile_height,
			1,
			200, 10, 10, 240,
			100, 50, 210, 60
		);
		tileSelectorElement->isVisible = true;

		if (panelTilesetPicker->width < tilesetElement->width + padding)
		{
			panelTilesetPicker->setWidth(tilesetElement->width + padding);
		}

		if (panelTilesetPicker->height < tilesetElement->height + padding + tilesetLabel->height)
		{
			panelTilesetPicker->setHeight(tilesetElement->height + padding + tilesetLabel->height);
		}

		tilesetElement->addElement(tileSelectorElement);
		panelTilesetPicker->addElement(tilesetLabel);

		tilesetElement->onMouseClick([this](int x, int y) {
			seekTile(x, y);
		});
		tilesetElement->onMouseMotion([this](int x, int y) {
			if (LCLICK)
			{
				seekTiles(x, y);
			}
		});
		panelTilesetPicker->addElement(tilesetElement);


		tileSelectorElements.insert({ tilesetKey, tileSelectorElement });
		tilesetSelectorElements.insert({ tilesetKey, tilesetElement });
		tilesetSelectorLabels.insert({ tilesetKey, tilesetLabel });
	}

	// Where to place all the buttons on this tileset picker panel
	auto buttonsYSpot = panelTilesetPicker->height - padding / 2;

	auto buttonTilesetSeekLeft = new OPI_Gui::Button("basic", 8, buttonsYSpot,
		new OPI_Text::TextComponent("<----", OPI_Text::FontManager::getFont("RobotoMono-Regular"), 14));
	buttonTilesetSeekLeft->onMouseClick([this]() {
		seekTilesetLeft();
		});

	auto buttonTilesetSeekRight = new OPI_Gui::Button("basic", 8 + buttonTilesetSeekLeft->width, buttonsYSpot,
		new OPI_Text::TextComponent("---->", OPI_Text::FontManager::getFont("RobotoMono-Regular"), 14));
	buttonTilesetSeekRight->onMouseClick([this]() {
		seekTilesetRight();
		});


	 
	// Make the panel a little bigger, so it can hold the buttons
	panelTilesetPicker->setHeight(panelTilesetPicker->height + buttonTilesetSeekLeft->height);

	// Add the buttons for tile seek
	panelTilesetPicker->addElement(buttonTilesetSeekLeft);
	panelTilesetPicker->addElement(buttonTilesetSeekRight);

	// add magic button - auto puzzle piece tiles together 
	// either a single tile fill or a multi-selection
	auto buttonMagicFill = new OPI_Gui::Button("tileMagicFill", 8+buttonTilesetSeekRight->x + buttonTilesetSeekRight->width, buttonsYSpot);
	buttonMagicFill->isToggle = true;
	buttonMagicFill->toggleOn();
	buttonMagicFill->onMouseClick([this]() {
		editor__mode = EditingMode::Tile_MagicFill;
		});

	// add paint brush button - smear any tile selection
	auto buttonPaintBrush = new OPI_Gui::Button("tilePaintBrush", buttonMagicFill->x + buttonMagicFill->width, buttonsYSpot);
	buttonPaintBrush->isToggle = true;
	buttonPaintBrush->onMouseClick([this]() {
		editor__mode = EditingMode::Tile_Brush;
		});

	// Toggle group for paint mode
	new OPI_Gui::ButtonToggleGroup({ {buttonMagicFill, buttonPaintBrush} });

	// Add the buttons for tile brush or magic mode
	panelTilesetPicker->addElement(buttonMagicFill);
	panelTilesetPicker->addElement(buttonPaintBrush);

	// Add tile with collision rect hybrid
	auto buttonTileCollisionRect = new OPI_Gui::Button("tileCollisionRect", buttonPaintBrush->x + buttonPaintBrush->width, buttonsYSpot);
	buttonTileCollisionRect->isToggle = true;
	buttonTileCollisionRect->onMouseClick([this]() {
		editor__brushModifier = editor__brushModifier == BrushModifier::None ?
			BrushModifier::CollisionRect : BrushModifier::None;
		});

	// Add button for tile with collision rect hybrid to gui
	panelTilesetPicker->addElement(buttonTileCollisionRect);

	this->tileSelectorElements[this->tilesetKeys[this->current_tileset_key]]->isVisible = true;
	this->tilesetSelectorElements[this->tilesetKeys[this->current_tileset_key]]->isVisible = true;
	this->tilesetSelectorLabels[this->tilesetKeys[this->current_tileset_key]]->isVisible = true;

	panelTilesetPicker->isVisible = true;
	panelTilesetPicker->isMovable = true;


	// TODO snap panel
	panelTilesetPicker->x = this->hotbarPanel->x - panelTilesetPicker->width;
	panelTilesetPicker->y = this->hotbarPanel->y + this->hotbarPanel->height - panelTilesetPicker->height;


	this->gui->addElement(panelTilesetPicker);

}

void SKO_MapEditor::Manager::addLayersHotbar()
{
	// 4 buttons with dimensions 64x64
	auto panelWidth = 4 * 64;
	auto panelHeight = 1 * 64 + 16;
	// center horizontally, place at bottom of the screen
	auto panelX = gui->getScreenWidth() / 2 - panelWidth / 2;
	auto panelY = gui->getScreenHeight() - panelHeight - 100;

	// Create invisible panel 
	//auto hotbarPanel = new OPI_Gui::Panel(panelX, panelY, panelWidth, panelHeight);
	auto hotbarPanel = new OPI_Gui::Panel(OPI_Gui::PanelThemeType::GridRect, "ice", panelX, panelY, panelWidth, panelHeight);
	hotbarPanel->isClosable = false;
	hotbarPanel->isMovable = true;
	hotbarPanel->isResizable = false;

	auto buttonToggleFringe = new OPI_Gui::Button("toggleFringe", 64 * 0, 16);
	buttonToggleFringe->isToggle = true;
	buttonToggleFringe->onMouseClick([this]() {
		editor__currentLayer = TileLayerType::Fringe;
	});
	auto buttonToggleFringeMask = new OPI_Gui::Button("toggleFringeMask", 64 * 1, 16);
	buttonToggleFringeMask->isToggle = true;
	buttonToggleFringeMask->onMouseClick([this]() {
		editor__currentLayer = TileLayerType::FringeMask;
	});
	auto buttonToggleBackground = new OPI_Gui::Button("toggleBackground", 64 * 2, 16);
	buttonToggleBackground->isToggle = true;
	buttonToggleBackground->toggleOn(); // default for this button toggle group
	buttonToggleBackground->onMouseClick([this]() {
		editor__currentLayer = TileLayerType::Background;
	});
	auto buttonToggleBackgroundMask = new OPI_Gui::Button("toggleBackgroundMask", 64 * 3, 16);
	buttonToggleBackgroundMask->isToggle = true;
	buttonToggleBackgroundMask->onMouseClick([this]() {
		editor__currentLayer = TileLayerType::BackgroundMask;
	});

	// Add the 8 buttons to the hotbar panel
	hotbarPanel->addElement(buttonToggleFringe);
	hotbarPanel->addElement(buttonToggleFringeMask);
	hotbarPanel->addElement(buttonToggleBackground);
	hotbarPanel->addElement(buttonToggleBackgroundMask);

	new OPI_Gui::ButtonToggleGroup({ buttonToggleFringe, buttonToggleFringeMask, buttonToggleBackground, buttonToggleBackgroundMask });

	// Add hotbar panel to gui manager
	this->gui->addElement(hotbarPanel);
}

void SKO_MapEditor::Manager::addTileHotBar()
{
	// 8 buttons with dimensions 64x64
	auto panelWidth = 7 * 64;
	auto panelHeight = 1 * 64 + 16;
	// center horizontally, place at bottom of the screen
	auto panelX = gui->getScreenWidth() / 2 - panelWidth / 2;
	auto panelY = gui->getScreenHeight() - panelHeight;

	this->hotbarPanel = new OPI_Gui::Panel(OPI_Gui::PanelThemeType::GridRect, "ice", panelX, panelY, panelWidth, panelHeight);
	hotbarPanel->isClosable = false;
	hotbarPanel->isMovable = true;
	hotbarPanel->isResizable = false;
	hotbarPanel->isVisible = true;


	// Create the 8 main menu buttons with actions
	auto* buttonAddTile = new OPI_Gui::Button("addTile", 64 * 0, 16);
	buttonAddTile->isToggle = true;
	buttonAddTile->toggleOn(); // default for this button toggle group
	buttonAddTile->onMouseClick([this]() {
		editor__mode = EditingMode::Tile_Brush;
		this->panelTilesetPicker->isVisible = true;
		// TODO snap panel
		panelTilesetPicker->x = this->hotbarPanel->x - panelTilesetPicker->width;
		panelTilesetPicker->y = this->hotbarPanel->y + this->hotbarPanel->height - panelTilesetPicker->height;
		});
	auto* buttonEditTile = new OPI_Gui::Button("editTile", 64 * 1, 16);
	buttonEditTile->isToggle = true;
	buttonEditTile->onMouseClick([this]() {
		editor__mode = EditingMode::Tile_Edit;
		this->panelTilesetPicker->isVisible = false;
		});
	auto* buttonDeleteTile = new OPI_Gui::Button("deleteTile", 64 * 2, 16);
	buttonDeleteTile->isToggle = true;
	buttonDeleteTile->onMouseClick([this]() {
		editor__mode = EditingMode::Tile_Delete;
		this->panelTilesetPicker->isVisible = false;
		});

	auto* buttonAddCollision = new OPI_Gui::Button("addCollision", 64 * 3, 16);
	buttonAddCollision->isToggle = true;
	buttonAddCollision->onMouseClick([this]() {
		editor__mode = EditingMode::Collision_Add;
		this->panelTilesetPicker->isVisible = false;
		});
	auto* buttonEditCollision = new OPI_Gui::Button("editCollision", 64 * 4, 16);
	buttonEditCollision->isToggle = true;
	buttonEditCollision->onMouseClick([this]() {
		editor__mode = EditingMode::Collision_Edit;
		this->panelTilesetPicker->isVisible = false;
		});
	auto* buttonDeleteCollision = new OPI_Gui::Button("deleteCollision", 64 * 5, 16);
	buttonDeleteCollision->isToggle = true;
	buttonDeleteCollision->onMouseClick([this, buttonEditCollision]() {
		editor__mode = EditingMode::Collision_Delete;
		this->panelTilesetPicker->isVisible = false;
		});

	auto* buttonToggleTest = new OPI_Gui::Button("toggleTest", 64 * 6, 16);
	buttonToggleTest->isToggle = true;
	buttonToggleTest->onMouseClick([this]() {
		editor__mode = EditingMode::Testing_Mode;
		});

	

	// Consider these buttons as radio button group
	new OPI_Gui::ButtonToggleGroup({ buttonAddTile, buttonDeleteTile, buttonEditTile, buttonAddCollision, buttonDeleteCollision, buttonEditCollision });

	// Add the 8 buttons to the hotbar panel
	hotbarPanel->addElement(buttonAddTile);
	hotbarPanel->addElement(buttonEditTile);
	hotbarPanel->addElement(buttonDeleteTile);
	hotbarPanel->addElement(buttonAddCollision);
	hotbarPanel->addElement(buttonEditCollision);
	hotbarPanel->addElement(buttonDeleteCollision);
	hotbarPanel->addElement(buttonToggleTest);

	// Add hotbar panel to gui manager
	this->gui->addElement(hotbarPanel);
}

void SKO_MapEditor::Manager::seekTile(int x, int y)
{
	auto tileWidth = this->tilesets[this->tilesetKeys[this->current_tileset_key]]->tile_width;
	auto tileHeight = this->tilesets[this->tilesetKeys[this->current_tileset_key]]->tile_height;
	auto tileSelector = this->tileSelectorElements[this->tilesetKeys[this->current_tileset_key]];

	// resize tileset selector
	tileSelector->setWidthHeight(tileWidth, tileHeight);
	
	// move visual indicator 
	tileSelector->x = x / tileWidth * tileWidth;
	tileSelector->y = y / tileHeight * tileHeight;

	// set current tile selection
	this->current_tileset_column = x / tileWidth;
	this->current_tileset_row = y / tileHeight;
	this->current_tileset_columns = 1;
	this->current_tileset_rows = 1;

}

void SKO_MapEditor::Manager::seekTiles(int x, int y)
{
	auto tileWidth = this->tilesets[this->tilesetKeys[this->current_tileset_key]]->tile_width;
	auto tileHeight = this->tilesets[this->tilesetKeys[this->current_tileset_key]]->tile_height;
	auto tileSelector = this->tileSelectorElements[this->tilesetKeys[this->current_tileset_key]];

	// Find if more were selected yet (if the mouse moved far enough)
	auto newColumn = x / tileWidth;
	auto newRow = y / tileHeight;

	if (newColumn > current_tileset_column) 
	{
		// Don't get a negative number
		this->current_tileset_columns = newColumn - this->current_tileset_column + 1;
	}
	else 
	{
		this->current_tileset_columns = 1;
	}

	if (newRow > current_tileset_row)
	{
		// Don't get a negative number
		this->current_tileset_rows = newRow - current_tileset_row + 1;
	}
	else
	{
		this->current_tileset_rows = 1;
	}

	// resize visual indicator 
	tileSelector->setWidthHeight(tileWidth * current_tileset_columns,  tileHeight * current_tileset_rows);
}

void SKO_MapEditor::Manager::seekTilesetLeft()
{	
	// Make the previous tileset invisible in the gui
	this->tilesetSelectorElements[this->tilesetKeys[this->current_tileset_key]]->isVisible = false;
	this->tilesetSelectorLabels[this->tilesetKeys[this->current_tileset_key]]->isVisible = false;

	this->current_tileset_key--;

	if (this->current_tileset_key < 0)
	{
		this->current_tileset_key = tilesetKeys.size() - 1;
	}

	// set current tileset
	this->current_tileset = tilesets[tilesetKeys[this->current_tileset_key]];
	// because it also has its own selector, set the current selected tile for editing
	auto tileSelector = this->tileSelectorElements[this->tilesetKeys[this->current_tileset_key]];
	this->current_tileset_column = tileSelector->x / tileSelector->width;
	this->current_tileset_row = tileSelector->y / tileSelector->height;

	// Make the current selected tileset visible in the gui
	this->tilesetSelectorElements[this->tilesetKeys[this->current_tileset_key]]->isVisible = true;
	this->tilesetSelectorLabels[this->tilesetKeys[this->current_tileset_key]]->isVisible = true;
	this->panelTilesetPicker->render();
}

void SKO_MapEditor::Manager::seekTilesetRight()
{
	// Make the previous tileset invisible in the gui
	this->tilesetSelectorElements[this->tilesetKeys[this->current_tileset_key]]->isVisible = false;
	this->tilesetSelectorLabels[this->tilesetKeys[this->current_tileset_key]]->isVisible = false;

	this->current_tileset_key++;

	if (this->current_tileset_key >= tilesetKeys.size())
	{
		this->current_tileset_key = 0;
	}

	this->current_tileset = tilesets[tilesetKeys[this->current_tileset_key]];
	// because it also has its own selector, set the current selected tile for editing
	auto tileSelector = this->tileSelectorElements[this->tilesetKeys[this->current_tileset_key]];
	this->current_tileset_column = tileSelector->x / tileSelector->width;
	this->current_tileset_row = tileSelector->y / tileSelector->height;

	// Make the current selected tileset visible in the gui
	this->tilesetSelectorElements[this->tilesetKeys[this->current_tileset_key]]->isVisible = true;
	this->tilesetSelectorLabels[this->tilesetKeys[this->current_tileset_key]]->isVisible = true;
	this->panelTilesetPicker->render();
}

void SKO_MapEditor::Manager::saveMap(std::string filePath)
{
	// Cleanup small rects you can't see
	//this->cleanupInvisibleRects(this->map);

	// Remove duplicate tiles that are exactly the same
	//this->removeDuplicateTiles(this->map);

	// Save the map at the given file location
	this->map().filePath = filePath;
	SKO_Map::Writer::saveMap(&(this->map()), tilesets);
}

void SKO_MapEditor::Manager::saveMap()
{
	// Cleanup small rects you can't see
	//this->cleanupInvisibleRects(this->map);

	// Remove duplicate tiles that are exactly the same
	//this->removeDuplicateTiles(this->map);

	// Save the map to its previous file location
	SKO_Map::Writer::saveMap(&this->map(), tilesets);
}


void SKO_MapEditor::Manager::loadMap(std::string fileName)
{
	// Load map
	this->map() = *this->mapReader->loadMap(fileName);

	// Set map title
	this->mainMenuGui->setMapTitle(this->map().name);
}

void SKO_MapEditor::Manager::drawSelectedTiles(int camera_x, int camera_y, int offset_x, int offset_y, const OPI_VectorSet<SKO_Map::Tile>& tiles, const SKO_Map::Tileset& tileset)
{
	
	for (const auto& tile : tiles)
	{
		int drawX = tile.x - camera_x + offset_x;
		int drawY = tile.y - camera_y + offset_y;

		SDL_Rect selection;
		selection.x = tile.tileset_column * tileset.tile_width;
		selection.y = tile.tileset_row * tileset.tile_height;
		selection.w = tileset.tile_width;
		selection.h = tileset.tile_height;
		
		renderer->drawImage(drawX, drawY, tileset.image, selection);
	}
}

void SKO_MapEditor::Manager::drawTiles(int camera_x, int camera_y, const OPI_VectorSet<SKO_Map::Tile>& tiles, const SKO_Map::Tileset &tileset)
{
	for (const auto &tile : tiles)
	{
		int drawX = tile.x - camera_x;
		int drawY = tile.y - camera_y;

		SDL_Rect selection;
		selection.x = tile.tileset_column * tileset.tile_width;
		selection.y = tile.tileset_row * tileset.tile_height;
		selection.w = tileset.tile_width;
		selection.h = tileset.tile_height;

		renderer->drawImage(drawX, drawY, tileset.image, selection);
	}
}

void SKO_MapEditor::Manager::drawTileLayer(int camera_x, int camera_y, const std::map<std::string, OPI_VectorSet<SKO_Map::Tile>>& tileLayer)
{
	for (auto it = tileLayer.begin(); it != tileLayer.end(); ++it)
	{
		auto tilesetKey = it->first;
		auto tileset = tilesets[tilesetKey];
		auto tiles = &it->second;

		drawTiles(camera_x, camera_y, *tiles, *tileset);
	}
}

void SKO_MapEditor::Manager::drawTileLayer(int camera_x, int camera_y, SKO_MapEditor::TileLayerType layer)
{
	auto tileLayer = getTileLayer(layer);

	//draw finished map tiles in this map layer
	for (auto it = tileLayer.begin(); it != tileLayer.end(); ++it)
	{
		auto tilesetKey = it->first;
		auto tileset = tilesets[tilesetKey];
		auto tiles = &it->second;
		
		drawTiles(camera_x, camera_y, *tiles, *tileset);
	}

	//draw selected tiles in edit mode
	if (editor__mode == EditingMode::Tile_Edit && editor__currentLayer == layer) {

		for (auto it = selectedTiles.begin(); it != selectedTiles.end(); ++it)
		{
			auto tilesetKey = it->first;
			auto tileset = tilesets[tilesetKey];
			auto tiles = &it->second;

			renderer->setColorMask(150, 150, 255, 100);
			drawSelectedTiles(camera_x, camera_y, 0, 0, *tiles, *tileset);
			renderer->resetColorMask();
		}
	}

	//draw current tiles in hand for copy/paste operations
	if (editor__mode == EditingMode::Tile_Copy)
	{
		for (auto it = selectedTiles.begin(); it != selectedTiles.end(); ++it)
		{
			auto tilesetKey = it->first;
			auto tileset = tilesets[tilesetKey];
			auto tiles = &it->second;

			drawSelectedTiles(copyRectMarker_x/32*32, copyRectMarker_y /32*32, cursor_x/32*32, cursor_y/32*32, *tiles, *tileset);
		}
	}
}


void SKO_MapEditor::Manager::DrawEditorOperations(int camera_x, int camera_y)
{
	// draw current selection rectangle for editing
	if (LCLICK && (this->tileEditSelectionRunning || this->collisionEditSelectionRunning))
	{
		auto rect = SKO_Map::CollisionRect{ this->editRectMarker_x, this->editRectMarker_y, (cursor_x + (int)camera_x) - editRectMarker_x, (cursor_y + (int)camera_y) - editRectMarker_y };
		snapCollisionRect(&rect, 4);

		SDL_Rect newRect;

		newRect.x = rect.x - (int)camera_x;
		newRect.y = rect.y - (int)camera_y;
		newRect.h = rect.h;
		newRect.w = rect.w;

		renderer->drawFilledRect(newRect, 25, 45, 245, 65);
	}

	//draw current tile selection in your hand
	if (editor__mode == EditingMode::Tile_Brush || editor__mode == EditingMode::Tile_MagicFill && !this->magicMarkerRunning) {

		int gridLines = editor__mode == EditingMode::Tile_Brush ? 32 : 32;
		int drawX = cursor_x / gridLines * gridLines;
		int drawY = cursor_y / gridLines * gridLines;

		SDL_Rect selection;
		selection.x = current_tileset_column * current_tileset->tile_width;
		selection.y = current_tileset_row * current_tileset->tile_height;
		selection.w = current_tileset->tile_width * current_tileset_columns;
		selection.h = current_tileset->tile_height * current_tileset_rows;

		renderer->drawImage(drawX, drawY, current_tileset->image, selection);

		if (editor__brushModifier == BrushModifier::CollisionRect)
		{
			// draw the collision rect around this tile to make it obvious before drawing
			SDL_Rect rect;

			rect.x = drawX;
			rect.y = drawY;
			rect.h = selection.h;
			rect.w = selection.w;

			renderer->drawFilledRect(rect, 240, 10, 10, 60);
		}
	}

	// Draw current magic fill operation
	if (editor__mode == EditingMode::Tile_MagicFill && LCLICK && this->magicMarkerRunning)
	{
		drawTiles(camera_x, camera_y, this->magicTiles, *this->current_tileset);

		if (editor__brushModifier == BrushModifier::CollisionRect)
		{
			SDL_Rect rect;
			auto first = *this->magicTiles.begin();
			auto last = *(this->magicTiles.end() - 1);

			int gridLines = editor__mode == EditingMode::Tile_Brush ? 32 : 32;
			int drawX = first.x / gridLines * gridLines - camera_x;
			int drawY = first.y / gridLines * gridLines - camera_y;

			rect.x = drawX;
			rect.y = drawY;
			rect.w = last.x - first.x + current_tileset->tile_width;
			rect.h = last.y - first.y + current_tileset->tile_height;

			renderer->drawFilledRect(rect, 240, 10, 10, 60);
		}
	}

	// Draw current collision rect operation
	if (editor__mode == EditingMode::Collision_Add && LCLICK && this->collisionRectRunning)
	{
		auto rect = SKO_Map::CollisionRect{ this->collisionRectMarker_x, this->collisionRectMarker_y, (cursor_x + (int)camera_x) - collisionRectMarker_x, (cursor_y + (int)camera_y) - collisionRectMarker_y };
		snapCollisionRect(&rect);

		SDL_Rect newRect;

		newRect.x = rect.x - (int)camera_x;
		newRect.y = rect.y - (int)camera_y;
		newRect.h = rect.h;
		newRect.w = rect.w;

		renderer->drawFilledRect(newRect, 245, 25, 25, 65);
	}

	// Draw collision rects in hand for copy/paste operations
	if (editor__mode == EditingMode::Collision_Copy)
	{
		//draw collision rects, only on screen
		for (const auto& rect : selectedCollisionRects)
		{
			int alpha = 180;

			SDL_Rect newRect;
			newRect.x = rect.x - (copyRectMarker_x / 32 * 32) + (cursor_x / 32 * 32);
			newRect.y = rect.y - (copyRectMarker_y / 32 * 32) + (cursor_y / 32 * 32);
			newRect.h = rect.h;
			newRect.w = rect.w;

			if (newRect.x >= 0 - newRect.w &&
				newRect.x < renderer->originalWindowWidth &&
				newRect.y < renderer->originalWindowHeight &&
				newRect.y >= 0 - newRect.h)
				renderer->drawFilledRect(newRect, 240, 140, 40, alpha);
		}
	}

	//Draw tiles in hand for copy/paste operations
	//look in drawTileLayer
}

void SKO_MapEditor::Manager::DrawGameScene(int camera_x, int camera_y)
{
	for (int x = 0; x <= renderer->windowWidth; x+= 1024)
	{
		for (int y = 0; y <= renderer->windowHeight; y += 1024)
		{
			renderer->drawImage(x, y, &background);
		}
	}

	drawTileLayer(camera_x, camera_y, TileLayerType::Background);
	drawTileLayer(camera_x, camera_y, TileLayerType::BackgroundMask);

	//stickman!
	if (stickman_toggle) //TODO - mapEditorState.stickmanVisible
		renderer->drawImage(stickman.x - 25 - camera_x, stickman.y - camera_y, &stickman_img);

	drawTileLayer(camera_x, camera_y, TileLayerType::Fringe);
	drawTileLayer(camera_x, camera_y, TileLayerType::FringeMask);

	//draw collision rects, only on screen
	for (const auto& rect : this->map().collisionRects)
	{

		int alpha = 60;

		if (this->selectedCollisionRects.contains(rect) && editor__mode == EditingMode::Collision_Edit)
		{
			alpha = 140;
		}

		SDL_Rect newRect;

		newRect.x = rect.x - (int)camera_x;
		newRect.y = rect.y - (int)camera_y;
		newRect.h = rect.h;
		newRect.w = rect.w;

		if (newRect.x >= 0 - newRect.w &&
			newRect.x < renderer->originalWindowWidth &&
			newRect.y < renderer->originalWindowHeight &&
			newRect.y >= 0 - newRect.h)
			renderer->drawFilledRect(newRect, 240, 10, 10, alpha);
	}

}


void SKO_MapEditor::Manager::handleWindowEvent(SDL_WindowEvent windowEvent)
{
	switch (windowEvent.event)
	{
	case SDL_WINDOWEVENT_SHOWN:
		SDL_Log("Window %d shown", event.window.windowID);
		break;
	case SDL_WINDOWEVENT_HIDDEN:
		SDL_Log("Window %d hidden", event.window.windowID);
		break;
	case SDL_WINDOWEVENT_EXPOSED:
		SDL_Log("Window %d exposed", event.window.windowID);
		break;
	case SDL_WINDOWEVENT_MOVED:
		SDL_Log("Window %d moved to %d,%d",
			event.window.windowID, event.window.data1,
			event.window.data2);
		renderer->movedScreen();
		break;


		// call this to trigger SDL_WINDOWEVENT_SIZE_CHANGED
		// SDL_SetWindowSize(window, windowWidth, windowHeight);
	case SDL_WINDOWEVENT_RESIZED:
		SDL_Log("Window %d resized to %dx%d",
			event.window.windowID, event.window.data1,
			event.window.data2);

		// TODO - here, when resizing, you can select the largest in the same aspect ratio
		renderer->windowWidth = event.window.data1;
		renderer->windowHeight = event.window.data2;
		renderer->originalWindowWidth = event.window.data1;
		renderer->originalWindowHeight = event.window.data2;
		renderer->resizeScreen();

		break;
	}
}

void SKO_MapEditor::Manager::handleKeyDown(SDL_Keycode keyCode)
{
	switch (keyCode)
	{

	case SDLK_LSHIFT:
		SHIFT_HELD = true;
		break;

	case SDLK_LCTRL:
		CTRL_HELD = true;
		break;

	case  SDLK_UP:
		if (SHIFT_HELD)
			panCamera(camera_x, camera_y - 1080);
		else
			panCamera(camera_x, camera_y - 64);
		break;

	case  SDLK_DOWN:
		if (SHIFT_HELD)
			panCamera(camera_x, camera_y + 1080);
		else
			panCamera(camera_x, camera_y + 64);
		break;

	case  SDLK_LEFT:
		if (SHIFT_HELD)
			panCamera(camera_x - renderer->windowWidth, camera_y);
		else
			panCamera(camera_x - 64, camera_y);
		break;

	case  SDLK_RIGHT:
		if (SHIFT_HELD)
			panCamera(camera_x + renderer->windowHeight, camera_y);
		else
			panCamera(camera_x + 64, camera_y);
		break;

	case SDLK_HOME:
		panCamera(0, 0);
		break;

	case SDLK_ESCAPE:
	{
		if (editor__mode == EditingMode::Tile_Copy)
		{
			editor__mode = EditingMode::Tile_Edit;
			this->selectedTiles.clear();
		}
		if (editor__mode == EditingMode::Collision_Copy)
		{
			editor__mode = EditingMode::Collision_Edit;
			this->selectedCollisionRects.erase(selectedCollisionRects.begin(), selectedCollisionRects.end());
		}
	}break;

	case SDLK_PAGEUP: // TODO: next / prev tile function
	{
		if (current_tileset_column == 0)
		{
			current_tileset_column = current_tileset->columns - 1;

			if (current_tileset_row == 0)
			{
				current_tileset_row = current_tileset->rows - 1;
			}
			else
			{
				current_tileset_row--;
			}
		}
		else
		{
			current_tileset_column--;
		}

	}break;

	case SDLK_PAGEDOWN: // TODO: next / prev tile function
	{
		current_tileset_column++;

		if (current_tileset_column >= current_tileset->columns)
		{
			current_tileset_column = 0;
			current_tileset_row++;

			if (current_tileset_row >= current_tileset->rows)
			{
				current_tileset_row = 0;
			}
		}
	}break;

	case SDLK_y:
	{
		if (CTRL_HELD)
		{
			this->mapStack.forward();
		}

	}break;

	case SDLK_z:
	{
		if (CTRL_HELD)
		{
			this->mapStack.back();
		}
		
	}break;

	case SDLK_v:
	{
		//draw current tiles in hand for copy/paste operations
		if (CTRL_HELD)
		{
			if (editor__mode == EditingMode::Tile_Copy)
			{
				pasteTiles();
			}
			else if (editor__mode == EditingMode::Collision_Copy)
			{
				pasteCollisions();
			}
		}
	}break;
	case SDLK_c:
		if (CTRL_HELD)
		{
			if (editor__mode == EditingMode::Collision_Edit && !collisionEditSelectionRunning)
			{
				// selected collision rects are already in hand
				// Copy mode means it's ready to paste as many times as needed
				editor__mode = EditingMode::Collision_Copy;
			}
			else if (editor__mode == EditingMode::Tile_Edit && !tileEditSelectionRunning)
			{
				editor__mode = EditingMode::Tile_Copy;
			}
		}
		break;


	case SDLK_x:
		if (CTRL_HELD)
		{
			if (editor__mode == EditingMode::Collision_Edit && !collisionEditSelectionRunning)
			{
				cutCollisions();

				// Copy mode means it's ready to paste as many times as needed
				editor__mode = EditingMode::Collision_Copy;
			}
			else if (editor__mode == EditingMode::Tile_Edit && !tileEditSelectionRunning)
			{
				cutTiles();

				// Copy mode means it's ready to paste as many times as needed
				editor__mode = EditingMode::Tile_Copy;
			}
		}
		break;

	case 'w':
		//if (current_tile != nullptr && editor__mode == EditingMode::Tile_Brush)
		//	current_tile->y--;

		//// Jump character
		//if (editor__mode == EditingMode::Testing_Mode)
		//	y_speed = -6;

		//if (editor__mode == EditingMode::Collision_Add && current_rect != nullptr)
		//{
		//	if (current_rect->y > 0)
		//	{
		//		current_rect->y--;
		//		current_rect->h++;
		//	}
		//}
		break;
	case 'a':

		/*if (current_tile != nullptr && editor__mode == EditingMode::Tile_Brush)
		{
			current_tile->x--;
		}

		LEFT = true;

		if (editor__mode == EditingMode::Collision_Add && current_rect != nullptr)
		{
			if (current_rect->x > 0)
			{
				current_rect->x--;
				current_rect->w++;
			}
		}*/
		break;
	case 's':
		/*if (current_tile != nullptr && editor__mode == EditingMode::Tile_Brush)
			current_tile->y++;

		if (editor__mode == EditingMode::Collision_Add && current_rect != nullptr)
		{
			current_rect->y++;
			current_rect->h--;
		}*/

		break;
	case 'd':

		/*if (current_tile != nullptr && editor__mode == EditingMode::Tile_Brush)
			current_tile->x++;

		RIGHT = true;

		if (editor__mode == EditingMode::Collision_Add && current_rect != nullptr)
		{
			current_rect->x++;
			current_rect->w--;
		}*/
		break;

		//smaller rects
	case 'i':
		/*if (editor__mode == EditingMode::Collision_Add && current_rect != nullptr)
		{
			if (current_rect->h > 0)
			{
				current_rect->h--;
			}
		}*/
		break;
	case 'j':
		/*if (editor__mode == EditingMode::Collision_Add && current_rect != nullptr)
		{
			if (current_rect->w > 0)
			{
				current_rect->w--;
			}
		}*/
		break;
	case 'k':
		/*if (editor__mode == EditingMode::Collision_Add && current_rect != nullptr)
		{
			current_rect->h++;
		}*/
		break;
	case 'l':
		//if (editor__mode == EditingMode::Collision_Add && current_rect != nullptr)
		//{
		//	current_rect->w++;
		//}
		break;

	case SDLK_DELETE:
		if (editor__mode == EditingMode::Collision_Edit && !collisionEditSelectionRunning)
		{
			cutCollisions();

			this->selectedCollisionRects.erase(selectedCollisionRects.begin(), selectedCollisionRects.end());
		} else if (editor__mode == EditingMode::Tile_Edit && !tileEditSelectionRunning)
		{
			cutTiles();

			this->selectedTiles.clear();
		}
		break;

	default:
		//printf("unknown key is: %i\n", (int)event.key.keysym.sym);
		break;
		
	}
}

void SKO_MapEditor::Manager::handleKeyUp(SDL_Keycode keyCode)
{
	switch (keyCode)
	{
	case 'a':
		LEFT = false;
		break;

	case 'd':
		RIGHT = false;
		break;

	case SDLK_LSHIFT:
		SHIFT_HELD = false;
		break;

	case SDLK_LCTRL:
		CTRL_HELD = false;
		break;

	default:
		break;
	}
}

void SKO_MapEditor::Manager::panCamera(int newCameraX, int newCameraY)
{
	//reset camera  if it goes off screen
	if (newCameraX < 0)
		newCameraX = 0;
	if (newCameraY < 0)
		newCameraY = 0;

	// TODO animate
	camera_x = newCameraX;
	camera_y = newCameraY;
	//camera_x_pan = newCameraX;
	//camera_y_pan = newCameraY;

}

bool SKO_MapEditor::Manager::HandleInput()
{
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_WINDOWEVENT)
		{
			handleWindowEvent(event.window);
		}

		if (event.type == SDL_QUIT)
		{
			// returning true will signal quit to main
			return true;
		}

		if (event.type == SDL_KEYUP)
		{
			handleKeyUp(event.key.keysym.sym);
		}

		if (event.type == SDL_KEYDOWN && event.key.repeat == 0)
		{
			handleKeyDown(event.key.keysym.sym);
		}

		if (event.type == SDL_MOUSEMOTION)
		{
			//Get the mouse offsets, scaled from screen space to window space
			cursor_x = renderer->getScaledMouseX(event.motion.x);
			cursor_y = renderer->getScaledMouseY(event.motion.y);


			// Don't run through GUI if the user clicked on the map and is dragging underneath a panel
			if (!dragOperationRunning)
			{
				// If the gui caught a mouse event
				bool eventHandled = gui->handleMouseMove(cursor_x, cursor_y);
				
				// do not keep processing mouse motion down to the map layer. if there's no draw operation running.
				continue;
			}

			// Go through map layer events
			int mapX = (int)(cursor_x + camera_x);
			int mapY = (int)(cursor_y + camera_y);

			switch (editor__mode)
			{
			case EditingMode::Collision_Add:
				handleMouseMotion_addCollisionRect();
					break;
			case EditingMode::Collision_Edit:
				handleMouseMotion_editCollisionRect();
				break;
			case EditingMode::Tile_Edit:
				handleMouseMotion_editTile();
				break;
			case EditingMode::Tile_Brush:
				handleMouseMotion_addTile();
					break;
			case EditingMode::Tile_MagicFill:
				handleMouseMotion_addTilesMagicFill();
				break;
			///
			// TODO - Tile Edit and Collision Rect Edit
			///
			default:
				break;
			}
		}

		if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && !LCLICK)
		{
			bool eventHandled = gui->handleMousePressLeft(cursor_x, cursor_y);
			LCLICK = true;

			if (eventHandled)
				continue;

			dragOperationRunning = true;

			switch (editor__mode)
			{
			case EditingMode::Tile_Brush:
				handleMouseClick_addTile();
				break;

			case EditingMode::Tile_MagicFill:
				handleMouseClick_addMagicMarker();
				break;
			case EditingMode::Tile_Copy:
				pasteTiles();
				break;
			case EditingMode::Tile_Delete: 
				handleMouseClick_deleteTile();
				break;

			case EditingMode::Tile_Edit:
				handleMouseClick_editTile();
				break;

			case EditingMode::Collision_Add: 
				handleMouseClick_addCollisionRect();
				break;
			case EditingMode::Collision_Copy:
				pasteCollisions();
				break;
			case EditingMode::Collision_Delete:
				handleMouseClick_deleteCollisionRect();
				break;
			case EditingMode::Collision_Edit:
				handleMouseClick_editCollisionRect();
				break;
			case EditingMode::Testing_Mode:
				handleMouseClick_Testing_Mode();
				break;
			}
		}

		if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT && !RCLICK)
		{
			bool eventHandled = gui->handleMousePressRight(cursor_x, cursor_y);
			RCLICK = true;

			if (eventHandled)
				continue;
		}
		
		if (event.type == SDL_MOUSEBUTTONUP)
		{
			if (event.button.button == SDL_BUTTON_LEFT && LCLICK)
			{
				LCLICK = false;

				// Don't run through GUI if the user clicked on the map and is dragging underneath a panel
				if (!dragOperationRunning)
				{
					bool eventHandled = gui->handleMouseReleaseLeft(cursor_x, cursor_y);

					if (eventHandled)
						continue;
				}

				dragOperationRunning = false;

				switch (editor__mode)
				{
				case EditingMode::Tile_MagicFill:
					handleMouseRelease_addTilesMagicFill();
					break;
				case EditingMode::Tile_Brush:
					handleMouseMotion_addTile();
					break;
				case EditingMode::Tile_Edit:
					handleMouseRelease_editTile();
					break;

				case EditingMode::Collision_Add:
					handleMouseRelease_addCollisionRect();
					break;
				case EditingMode::Collision_Edit:
					handleMouseRelease_editCollisionRect();
					break;

				default:
					break;
				}
			}

			if (event.button.button == SDL_BUTTON_RIGHT && RCLICK)
			{
				bool eventHandled = gui->handleMouseReleaseRight(cursor_x, cursor_y);
				RCLICK = false;

				if (eventHandled)
					continue;
			}
		}
	} //if poll event


	//repeat one final throttled mouse movement
	if (throttle_mouse_motion_last_time > 0 && OPI_Clock::milliseconds() > throttle_mouse_motion_last_time)
	{
		switch (editor__mode)
		{
		case EditingMode::Collision_Add:
			handleMouseMotion_addCollisionRect();
			break;
		case EditingMode::Collision_Edit:
			handleMouseMotion_editCollisionRect();
			break;
		case EditingMode::Tile_Edit:
			handleMouseMotion_editTile();
			break;
		case EditingMode::Tile_Brush:
			handleMouseMotion_addTile();
			break;
		case EditingMode::Tile_MagicFill:
			handleMouseMotion_addTilesMagicFill();
			break;
			///
			// TODO - Tile Edit and Collision Rect Edit
			///
		default:
			break;
		}

		//reset after we finish so this doesn't happen again.
		throttle_mouse_motion_last_time = 0;
	}

	return false;
}

bool SKO_MapEditor::Manager::processLoop()
{
	//auto inputTime = OPI_Clock::milliseconds();
	if (HandleInput()) {
		return true;
	}

	if (OPI_Clock::milliseconds() - coordsTicker > 100)
	{
		//draw coords
		std::stringstream ss;
		ss << "(" << cursor_x + camera_x << ", " << cursor_y + camera_y << ")";

		mainMenuGui->setCoords(ss.str());

		//reset ticker
		coordsTicker = OPI_Clock::milliseconds();
	}


	auto gDrawTime = OPI_Clock::microseconds();
	graphics();
	gDrawTime = OPI_Clock::microseconds() - gDrawTime;
	fpsCounter++;

	//inputTime = OPI_Clock::milliseconds() - inputTime;

	// TODO delete I think: if (stickman_toggle)
	physics();

	if (OPI_Clock::microseconds() - fpsTicker > 1'000'000)
	{
		double fpsTime = OPI_Clock::microseconds() - fpsTicker;
		int fps = (double)fpsCounter / fpsTime * 1'000'000;
		//draw coords
		std::stringstream ss;
		ss << "FPS: " << fps;
		//ss << " " << "Graphics Draw Time: " << gDrawTime << " " << "Input Handler Time: " << inputTime;

		mainMenuGui->setFps(ss.str());

		//reset ticker
		fpsTicker = OPI_Clock::microseconds();

		//reset frame copunt
		fpsCounter = 0;
	}

	if (OPI_Clock::milliseconds() - metaTicker > 1000)
	{
		std::stringstream ss;
		ss << "meta: background=" << SKO_Map::Writer::countTiles(this->map().backgroundTiles)
			<< ", background_mask=" << SKO_Map::Writer::countTiles(this->map().backgroundMaskTiles)
			<< ", fringe=" << SKO_Map::Writer::countTiles(this->map().fringeTiles)
			<< ", fringe_mask=" << SKO_Map::Writer::countTiles(this->map().fringeMaskTiles);
		//ss << " " << "Graphics Draw Time: " << gDrawTime << " " << "Input Handler Time: " << inputTime;

		mainMenuGui->setMeta(ss.str());

		//reset ticker
		metaTicker = OPI_Clock::milliseconds();
	}
	

	//prevent repeats
	this->event.type = 0;

	return false;
}

// Simulate Physics of test stickman
bool SKO_MapEditor::Manager::isBlocked(SDL_Rect rect)
{
	for (auto collisionRect : this->map().collisionRects)
	{
		SDL_Rect mapRect = SDL_Rect{ collisionRect.x, collisionRect.y, collisionRect.w, collisionRect.h };
		if (SDL_HasIntersection(&rect, &mapRect))
			return true;
	}

	return false;
}

void SKO_MapEditor::Manager::graphics()
{
	renderer->startDraw();

	// Draw map, tiles, etc
	DrawGameScene((int)this->camera_x, (int)this->camera_y);

	// Draw editor operations that go on top of the game scene
	DrawEditorOperations((int)this->camera_x, (int)this->camera_y);

	// Draw Map Editor Gui
	gui->DrawGui();

	//update screen
	renderer->updateScreen();
}

std::map<std::string, OPI_VectorSet<SKO_Map::Tile>>& SKO_MapEditor::Manager::getTileLayer(SKO_MapEditor::TileLayerType layer)
{
	switch (layer)
	{
	case TileLayerType::Fringe:
		return this->map().fringeTiles;
		break;
	case TileLayerType::FringeMask:
		return this->map().fringeMaskTiles;
		break;
	case TileLayerType::Background:
		return this->map().backgroundTiles;
		break;
	case TileLayerType::BackgroundMask:
		return this->map().backgroundMaskTiles;
		break;
	}
}

void SKO_MapEditor::Manager::physics()
{
	// fall
	if (y_speed < 10)
	{
		y_speed += GRAVITY;
	}

	//verical collision detection
	SDL_Rect stickmanRect;
	stickmanRect.x = stickman.x;
	stickmanRect.y = stickman.y + y_speed;
	stickmanRect.w = 14;
	stickmanRect.h = 63;
	bool block_y = isBlocked(stickmanRect);

	//vertical movement
	if (!block_y)
	{
		//not blocked, fall
		stickman.y += y_speed;
	}
	else
	{
		y_speed = 0;
	}

	//horizontal collision detection
	stickmanRect.x = stickman.x + x_speed;
	stickmanRect.y = stickman.y;
	bool block_x = isBlocked(stickmanRect);

	//horizontal movement
	if (!block_x)
	{
		//not blocked, walk
		stickman.x += (x_speed);
	}

	if (LEFT && x_speed != -2)
	{
		x_speed = -2;
	}
	if (RIGHT && x_speed != 2)
	{
		x_speed = 2;
	}

	if (LEFT == RIGHT)
	{
		if (x_speed != 0)
			x_speed = 0;
	}
}

void SKO_MapEditor::Manager::snapCollisionRect(SKO_Map::CollisionRect* rect, int gridSize)
{
	auto xRem = rect->x % gridSize;
	auto xwRem = (rect->x + rect->w) % gridSize;
	auto yRem = rect->y % gridSize;
	auto yhRem = (rect->y + rect->h) % gridSize;
	auto xMoved = 0;

	if (xRem < (gridSize / 2))
	{
		//  snap left
		rect->x = rect->x - xRem;
		xMoved = -xRem;
	}
	else
	{
		// move right then snap left
		rect->x = rect->x + gridSize - xRem;
		xMoved = gridSize - xRem;
	}

	if (xwRem < (gridSize / 2))
	{
		//  snap left
		rect->w = rect->w - xwRem - xMoved;
	}
	else
	{
		// move right then snap left
		rect->w = rect->w + gridSize - xwRem - xMoved;
	}

	if (yRem < (gridSize / 2))
	{
		//  snap left
		rect->y = rect->y - yRem;
		xMoved = -yRem;
	}
	else
	{
		// move right then snap left
		rect->y = rect->y + gridSize - yRem;
		xMoved = gridSize - yRem;
	}

	if (yhRem < (gridSize / 2))
	{
		//  snap left
		rect->h = rect->h - yhRem - xMoved;
	}
	else
	{
		// move right then snap left
		rect->h = rect->h + gridSize - yhRem - xMoved;
	}

	// Invert negative width/height
	if (rect->w < 0) {
		rect->x = rect->x + rect->w;
		rect->w = -(rect->w);
	}
	if (rect->h < 0) {
		rect->y = rect->y + rect->h;
		rect->h = -(rect->h);
	}
}

void SKO_MapEditor::Manager::deleteTileFromLayer(TileLayerType layer, std::string tilesetKey, SKO_Map::Tile& tile)
{
	switch (layer)
	{
	case TileLayerType::Fringe:
		this->map().fringeTiles[tilesetKey].erase(tile);
		break;
	case TileLayerType::FringeMask:
		this->map().fringeMaskTiles[tilesetKey].erase(tile);
		break;
	case TileLayerType::Background:
		this->map().backgroundTiles[tilesetKey].erase(tile);
		break;
	case TileLayerType::BackgroundMask:
		this->map().backgroundMaskTiles[tilesetKey].erase(tile);
		break;
	default:
		break;
	}
}

void SKO_MapEditor::Manager::addTileToLayer(TileLayerType layer, std::string tilesetKey, SKO_Map::Tile& tile)
{
	switch (layer)
	{
	case TileLayerType::Background:
		this->map().backgroundTiles[tilesetKey].upsert(tile);
		break;
	case TileLayerType::BackgroundMask:
		this->map().backgroundMaskTiles[tilesetKey].upsert(tile);
		break;
	case TileLayerType::Fringe:
		this->map().fringeTiles[tilesetKey].upsert(tile);
		break;
	case TileLayerType::FringeMask:
		this->map().fringeMaskTiles[tilesetKey].upsert(tile);
		break;
	}
}

// Functions that do not alter map history
void SKO_MapEditor::Manager::handleMouseMotion_addTilesMagicFill()
{
	// Do not handle motion input if the magic marker isn't running
	if (!magicMarkerRunning)
	{
		return;
	}

	// Get current placement grid spread out from the marker (x,y) to current mouse (x, y)
	int lastX = (cursor_x + ((int)camera_x)) / 32 * 32;
	int lastY = (cursor_y + ((int)camera_y)) / 32 * 32;
	int startX = magicMarker_x;
	int startY = magicMarker_y;

	// See if it changed since last time, to avoid more reprocessing
	auto magicTilesWidth = lastX > startX ?
		(lastX - startX) / 32
		: (startX - lastX) / 32;
	auto magicTilesHeight = lastY > startY ?
		(lastY - startY) / 32
		: (startY - lastY) / 32;

	// avoid reprocessing if the grid-based width and height didn't change
	if (this->currentMagicTilesWidth == magicTilesWidth && this->currentMagicTilesHeight == magicTilesHeight)
	{
		return;
	}

	// Now that we know it changed, mark the new width for next time
	this->currentMagicTilesWidth = magicTilesWidth;
	this->currentMagicTilesHeight = magicTilesHeight;

	int padColumns = 1;
	int padRows = 1;
	if (lastX < startX)
	{
		padColumns += current_tileset_columns - 1;
		startX = lastX;
		lastX = magicMarker_x + current_tileset_columns;
	}

	if (lastY < startY)
	{
		padRows += current_tileset_rows - 1;
		startY = lastY;
		lastY = magicMarker_y + current_tileset_rows;
	}

	int finalColumn = (lastX - startX) / 32 + padColumns;
	int finalRow = (lastY - startY) / 32 + padRows;


	// Always draw or place current selection amount, not less, because that wouldn't make sense.
	if (finalColumn < current_tileset_columns)
	{
		finalColumn = current_tileset_columns;
	}

	// Always draw or place current selection amount, not less, because that wouldn't make sense.
	if (finalRow < current_tileset_rows)
	{
		finalRow = current_tileset_rows;
	}

	//int startingX = current_tile->x - (current_tileset_columns-1) * current_tileset->tile_width;
	//int startingY = current_tile->y - (current_tileset_rows-1) * current_tileset->tile_height;

	// Clear out the current selection of magic tiles in hand
	this->magicTiles.clear();

	// Construct all the magic tiles to draw while placing, and to palce into the map after mouse release
	for (int column = 0; column < finalColumn; ++column)
	{
		for (int row = 0; row < finalRow; ++row)
		{
			int drawX = startX + column * current_tileset->tile_width;
			int drawY = startY + row * current_tileset->tile_height;

			// choose tileset clip size
			SDL_Rect ts;
			ts.w = current_tileset->tile_width;
			ts.h = current_tileset->tile_height;

			// default
			ts.x = current_tileset_column * current_tileset->tile_width; // left-most
			ts.y = current_tileset_row * current_tileset->tile_height; // top-most

			// Only try to puzzle edges and stuff for a "3x3"+ size
			if (current_tileset_rows > 1 || current_tileset_columns > 1)
			{
				///
				// choose tileset clip area, which corner or side to draw
				// the magic happens here
				///

				// top-left corner
				if (column == 0 && row == 0)
				{
					ts.x = current_tileset_column * current_tileset->tile_width; // left-most
					ts.y = current_tileset_row * current_tileset->tile_height; // top-most
				}
				// top-right corner
				else if (column == finalColumn - 1 && row == 0)
				{
					ts.x = (current_tileset_column + current_tileset_columns - 1) * current_tileset->tile_width; // right-most
					ts.y = current_tileset_row * current_tileset->tile_height; // top-most
				}
				// bottom right corner
				else if (column == finalColumn - 1 && row == finalRow - 1)
				{
					ts.x = (current_tileset_column + current_tileset_columns - 1) * current_tileset->tile_width; // right-most
					ts.y = (current_tileset_row + current_tileset_rows - 1) * current_tileset->tile_height; // bottom-most
				}
				// bottom left corner
				else if (column == 0 && row == finalRow - 1)
				{
					ts.x = current_tileset_column * current_tileset->tile_width; // left-most
					ts.y = (current_tileset_row + current_tileset_rows - 1) * current_tileset->tile_height; // bottom-most
				}
				/////// SIDES ////////
				// top side
				else if (row == 0)
				{
					ts.x = current_tileset_columns > 1 ? (current_tileset_column + 1) * current_tileset->tile_width // "center" of puzzle side
						: current_tileset_column * current_tileset->tile_width;
				}
				// botton side
				else if (row == finalRow - 1)
				{
					ts.x = current_tileset_columns > 1 ? (current_tileset_column + 1) * current_tileset->tile_width // "center" of puzzle side
						: current_tileset_column * current_tileset->tile_width;
					ts.y = (current_tileset_row + current_tileset_rows - 1) * current_tileset->tile_height; // bottom
				}
				// right side
				else if (column == finalColumn - 1 && current_tileset_columns > 1)
				{
					ts.x = (current_tileset_column + current_tileset_columns - 1) * current_tileset->tile_width; /// right
					ts.y = current_tileset_rows > 1 ? (current_tileset_row + 1) * current_tileset->tile_height // "center" of puzzle side
						: (current_tileset_row)*current_tileset->tile_height;
				}
				// left side
				else if (column == 0)
				{
					/// left
					ts.x = current_tileset_column * current_tileset->tile_width;
					ts.y = current_tileset_rows > 1 ? (current_tileset_row + 1) * current_tileset->tile_height // "center" of puzzle side
						: current_tileset_row * current_tileset->tile_height;
				}
				// filler
				else
				{
					// attempt to use top-left, x+1 and y+1 because that's probably the filer, in the "middle" of the selection
					ts.y = current_tileset_rows > 1 ?
						(current_tileset_row + 1) * current_tileset->tile_height :
						ts.y;

					ts.x = current_tileset_columns > 1 ?
						(current_tileset_column + 1) * current_tileset->tile_width :
						ts.x;
				}
			}

			// Make a new tile, add it to the list of in-hand magic tiles 
			auto tile = SKO_Map::Tile();
			tile.tileset_column = ts.x / current_tileset->tile_width;
			tile.tileset_row = ts.y / current_tileset->tile_height;
			tile.x = drawX;
			tile.y = drawY;
			this->magicTiles.upsert(tile);
		}
	}
}
void SKO_MapEditor::Manager::handleMouseClick_addMagicMarker()
{
	// Kick off the magic fill operation
	this->magicMarkerRunning = true;
	this->magicMarker_x = (int)(cursor_x + camera_x) / 32 * 32;
	this->magicMarker_y = (int)(cursor_y + camera_y) / 32 * 32;

	// Reset (or initialize) the magic fill grid across mouse motion 
	this->currentMagicTilesWidth = -1;
	this->currentMagicTilesHeight = -1;


	// Set the initial size of the magic fill operation
	handleMouseMotion_addTilesMagicFill();
}
void SKO_MapEditor::Manager::handleMouseClick_editTile()
{
	//Get the mouse offsets, scaled from screen space to window space
	cursor_x = renderer->getScaledMouseX(event.motion.x);
	cursor_y = renderer->getScaledMouseY(event.motion.y);

	//Get the mouse offsets and set anchor point for rect drawing
	editRectMarker_x = (cursor_x + (int)camera_x);
	editRectMarker_y = (cursor_y + (int)camera_y);

	tileEditSelectionRunning = true;
}
void SKO_MapEditor::Manager::handleMouseClick_addCollisionRect()
{
	//Get the mouse offsets, scaled from screen space to window space
	cursor_x = renderer->getScaledMouseX(event.motion.x);
	cursor_y = renderer->getScaledMouseY(event.motion.y);

	//Get the mouse offsets and set anchor point for rect drawing
	collisionRectMarker_x = (cursor_x + (int)camera_x);
	collisionRectMarker_y = (cursor_y + (int)camera_y);

	collisionRectRunning = true;
}
void SKO_MapEditor::Manager::handleMouseClick_editCollisionRect()
{
	//Get the mouse offsets, scaled from screen space to window space
	cursor_x = renderer->getScaledMouseX(event.motion.x);
	cursor_y = renderer->getScaledMouseY(event.motion.y);

	//Get the mouse offsets and set anchor point for rect drawing
	editRectMarker_x = (cursor_x + (int)camera_x);
	editRectMarker_y = (cursor_y + (int)camera_y);

	collisionEditSelectionRunning = true;
}
void SKO_MapEditor::Manager::handleMouseClick_Testing_Mode()
{
	stickman_toggle = true;
	stickman.x = cursor_x + camera_x;
	stickman.y = cursor_y + camera_y;
}
void SKO_MapEditor::Manager::handleMouseRelease_editCollisionRect()
{
	// Just in case release was slightly off from last move
	handleMouseMotion_editCollisionRect();

	//Reset after we're done selecting.
	collisionEditSelectionRunning = false;
}
void SKO_MapEditor::Manager::handleMouseRelease_editTile()
{
	tileEditSelectionRunning = false;
}
void SKO_MapEditor::Manager::handleMouseMotion_editTile()
{
	// throttle
	if (handleMouseMotion_Throttle()) {
		return;
	}

	auto tileLayer = getTileLayer(editor__currentLayer);
	
	//start with empty
	this->selectedTiles.clear();

	for (auto it = tileLayer.begin(); it != tileLayer.end(); ++it)
	{
		auto tilesetKey = it->first;
		auto tileset = tilesets[tilesetKey];
		auto tiles = &it->second;
		auto width = tileset->tile_width;
		auto height = tileset->tile_height;

		auto filteredTiles = tiles->filter([width, height, this](SKO_Map::Tile tile) {

			SKO_Map::CollisionRect s{
				editRectMarker_x,
				editRectMarker_y,
				(int)camera_x + cursor_x - editRectMarker_x,
				(int)camera_y + cursor_y - editRectMarker_y
			};

			snapCollisionRect(&s, 1);

			SDL_Rect selectionRect{ s.x, s.y, s.w, s.h };
			SDL_Rect checkRect{ tile.x, tile.y, width, height };

			bool intersects = SDL_HasIntersection(&selectionRect, &checkRect);

			return intersects;
		});

		selectedTiles[tilesetKey].upsert(filteredTiles);

		if (filteredTiles.size())
		{
			copyRectMarker_x = filteredTiles.at(0).x;
			copyRectMarker_y = filteredTiles.at(0).y;
		}
	}
}
void SKO_MapEditor::Manager::handleMouseMotion_deleteTile()
{
}
void SKO_MapEditor::Manager::handleMouseMotion_addCollisionRect()
{
	//if (current_rect != nullptr && LCLICK)
	//{
	//	//variables for easy determining
	//	int cx = cursor_x + (int)camera_x;
	//	int cy = cursor_y + (int)camera_y;
	//	int x1, x2, y1, y2;

	//	//
	//	// X
	//	//
	//	if (cx > collisionRectMarker_x)
	//	{
	//		x1 = current_rect->x;
	//		x2 = cx;
	//	}
	//	else
	//	{
	//		x1 = cx;
	//		x2 = collisionRectMarker_x;
	//	}

	//	//
	//	// Y
	//	//
	//	if (cy > collisionRectMarker_y)
	//	{
	//		y1 = current_rect->y;
	//		y2 = cy;
	//	}
	//	else
	//	{
	//		y1 = cy;
	//		y2 = collisionRectMarker_y;
	//	}

	//	//adjust the width and height
	//	current_rect->x = x1;
	//	current_rect->w = x2 - x1;
	//	current_rect->y = y1;
	//	current_rect->h = y2 - y1;
	//}

}
void SKO_MapEditor::Manager::handleMouseMotion_editCollisionRect()
{
	// throttle
	if (handleMouseMotion_Throttle()) {
		return;
	}

	this->selectedCollisionRects = OPI_VectorSet<SKO_Map::CollisionRect>::filter_set(
		this->map().collisionRects, [this](SKO_Map::CollisionRect c) {

			snapCollisionRect(&c, 1);

			SKO_Map::CollisionRect s{
				editRectMarker_x,
				editRectMarker_y,
				(int)camera_x + cursor_x - editRectMarker_x,
				(int)camera_y + cursor_y - editRectMarker_y
			};

			snapCollisionRect(&s, 1);
			SDL_Rect selectionRect{ s.x, s.y, s.w, s.h };

			SDL_Rect checkRect{ c.x, c.y, c.w, c.h };


			bool intersects = SDL_HasIntersection(&selectionRect, &checkRect);

			return intersects;
		});

	if (selectedCollisionRects.size())
	{
		copyRectMarker_x = (*selectedCollisionRects.begin()).x;
		copyRectMarker_y = (*selectedCollisionRects.begin()).y;
	}
}
void SKO_MapEditor::Manager::handleMouseMotion_deleteCollisionRect()
{
}


// Functions that do a batch operation on the map.
// for history purposes please use the following: 
//		this->mapStack.saveHistory();

void SKO_MapEditor::Manager::addTile()
{
	auto tileWidth = this->tilesets[this->tilesetKeys[this->current_tileset_key]]->tile_width;
	auto tileHeight = this->tilesets[this->tilesetKeys[this->current_tileset_key]]->tile_height;

	for (int row = 0; row < current_tileset_rows; ++row)
	{
		for (int column = 0; column < current_tileset_columns; ++column)
		{
			auto x = (int)(cursor_x + camera_x) / 32 * 32;
			auto y = (int)(cursor_y + camera_y) / 32 * 32;

			// Add on the row/column to the x and y so it draws an area that is selected
			x += column * tileWidth;
			y += row * tileWidth;

			auto tile = SKO_Map::Tile(x, y, current_tileset_row + row, current_tileset_column + column);

			addTileToLayer(editor__currentLayer, current_tileset->key, tile);

			// TODO  -- current TILES .. plural...
			current_tile = tile;//glob
		}
	}

	if (editor__brushModifier == BrushModifier::CollisionRect)
	{
		// Make collision rect around it
		auto rect = SKO_Map::CollisionRect(
			(int)(cursor_x + camera_x) / 32 * 32,
			(int)(cursor_y + camera_y) / 32 * 32,
			current_tileset_columns * tileWidth,
			current_tileset_rows * tileHeight
		);

		this->map().collisionRects.insert(rect);
		current_rect = rect;
	}


	// TODO  -- current TILES .. plural...
	//current_tiles = tile[something]<orotheridk>;
}

void SKO_MapEditor::Manager::handleMouseClick_addTile()
{
	//Save map history for ctrl+z
	this->mapStack.saveHistory();

	addTile();
}

void SKO_MapEditor::Manager::handleMouseClick_deleteTile()
{
	//Save map history for ctrl+z
	this->mapStack.saveHistory();

	// find a tile to delete
	int x = cursor_x + camera_x;
	int y = cursor_y + camera_y;
	auto tileLayer = getTileLayer(editor__currentLayer);
	
	for (auto it = tileLayer.begin(); it != tileLayer.end(); ++it)
	{
		auto tilesetKey = it->first;
		auto tileset = tilesets[tilesetKey];
		auto tiles = &it->second;
		int width = tileset->tile_width;
		int height = tileset->tile_height;

		for (auto tile : *tiles)
		{
			if (x > tile.x && x < tile.x + width &&
				y > tile.y && y < tile.y + height)
			{
				tiles->erase(tile);
				break;
			}
		}
	}
}

void SKO_MapEditor::Manager::handleMouseClick_deleteCollisionRect()
{
	//Save map history for ctrl+z
	this->mapStack.saveHistory();

	//find a collision to delete
	SDL_Point p = SDL_Point{ (int)(cursor_x + camera_x) , (int)(cursor_y + camera_y) };

	for (auto collisionRect : this->map().collisionRects)
	{
		auto rect = SDL_Rect(collisionRect.x, collisionRect.y, collisionRect.w, collisionRect.h);

		if (SDL_PointInRect(&p, &rect))
		{
			this->map().collisionRects.erase(collisionRect);
			break;
		}
	}
}

void SKO_MapEditor::Manager::handleMouseMotion_addTile()
{
	if (LCLICK)
	{
		//Map history already saved inside here
		addTile();
	}
}

void SKO_MapEditor::Manager::handleMouseRelease_addTilesMagicFill()
{
	//Save map history for ctrl+z
	this->mapStack.saveHistory();
	for (const auto &tile : this->magicTiles)
	{
		switch (editor__currentLayer)
		{
		case TileLayerType::Background:
			this->map().backgroundTiles[current_tileset->key].upsert(tile);
			break;
		case TileLayerType::BackgroundMask:
			this->map().backgroundMaskTiles[current_tileset->key].upsert(tile);
			break;
		case TileLayerType::Fringe:
			this->map().fringeTiles[current_tileset->key].upsert(tile);
			break;
		case TileLayerType::FringeMask:
			this->map().fringeMaskTiles[current_tileset->key].upsert(tile);
			break;
		}

		// TODO  -- current TILES .. plural...
		//current_tile = &tile; // glob
	}

	if (editor__brushModifier == BrushModifier::CollisionRect)
	{
		auto first = *(this->magicTiles.begin());
		auto last = *(this->magicTiles.end() - 1);

		auto w = last.x - first.x + current_tileset->tile_width;
		auto h = last.y - first.y + current_tileset->tile_height;
		auto collisionRect = SKO_Map::CollisionRect(first.x, first.y, w, h);

		this->map().collisionRects.insert(collisionRect);
		current_rect = collisionRect;
	}

	this->magicTiles.clear();
	this->magicMarkerRunning = false;
}

bool SKO_MapEditor::Manager::handleMouseMotion_Throttle()
{
	if (OPI_Clock::milliseconds() - throttle_mouse_motion_ticker > throttle_time)
	{
		// reset
		throttle_mouse_motion_last_time = 0;
		throttle_mouse_motion_ticker = OPI_Clock::milliseconds();
		return false;
	}

	throttle_mouse_motion_last_time = OPI_Clock::milliseconds() + throttle_time;

	//return true to signal it is throttling and the caller should return.
	return true;
}

void SKO_MapEditor::Manager::handleMouseRelease_addCollisionRect()
{
	//Get the mouse offsets, scaled from screen space to window space
	cursor_x = renderer->getScaledMouseX(event.motion.x);
	cursor_y = renderer->getScaledMouseY(event.motion.y);

	auto newRect = SKO_Map::CollisionRect{ this->collisionRectMarker_x, this->collisionRectMarker_y, (cursor_x + (int)camera_x) - collisionRectMarker_x, (cursor_y + (int)camera_y) - collisionRectMarker_y };
	snapCollisionRect(&newRect);	
	SDL_Rect testRect{ newRect.x, newRect.y, newRect.w, newRect.h };

	// Prevent empty rects
	if (SDL_RectEmpty(&testRect))
		return;

	//Save map history for ctrl+z
	this->mapStack.saveHistory();

	this->map().collisionRects.insert(newRect);
	current_rect = newRect;

	collisionRectRunning = false;
	collisionRectMarker_x = -1;
	collisionRectMarker_y = -1;
}

void SKO_MapEditor::Manager::pasteTiles()
{
	//Save map history for ctrl+z
	this->mapStack.saveHistory();

	for (auto it = selectedTiles.begin(); it != selectedTiles.end(); ++it)
	{
		auto tilesetKey = it->first;
		auto tileset = tilesets[tilesetKey];
		auto tiles = &it->second;

		for (auto& tile : *tiles)
		{
			SKO_Map::Tile pasteTile(tile.x, tile.y, tile.tileset_row, tile.tileset_column);
			pasteTile.x += camera_x - (copyRectMarker_x / 32 * 32) + (cursor_x / 32 * 32);
			pasteTile.y += camera_y - (copyRectMarker_y / 32 * 32) + (cursor_y / 32 * 32);

			addTileToLayer(editor__currentLayer, tilesetKey, pasteTile);
		}
	}
}

void SKO_MapEditor::Manager::cutTiles()
{
	//Save map history for ctrl+z
	this->mapStack.saveHistory();

	// Delete the selected tiles in the current map, but leave them in hand
	for (auto it = selectedTiles.begin(); it != selectedTiles.end(); ++it)
	{
		auto tilesetKey = it->first;
		auto tileset = tilesets[tilesetKey];
		auto tiles = &it->second;

		for (auto tile : *tiles)
		{
			deleteTileFromLayer(editor__currentLayer, tilesetKey, tile);
		}
	}
}

void SKO_MapEditor::Manager::pasteCollisions()
{
	//Save map history for ctrl+z
	this->mapStack.saveHistory();

	for (auto& rect : selectedCollisionRects)
	{
		auto pasteRect = SKO_Map::CollisionRect(rect.x, rect.y, rect.w, rect.h);
		pasteRect.x += camera_x - (copyRectMarker_x / 32 * 32) + (cursor_x / 32 * 32);
		pasteRect.y += camera_y - (copyRectMarker_y / 32 * 32) + (cursor_y / 32 * 32);

		this->map().collisionRects.insert(pasteRect);

		// todo currentRects or nah
		current_rect = pasteRect;

		//collisionRectRunning = false;
		//collisionRectMarker_x = -1;
		//collisionRectMarker_y = -1;
	}
}

void SKO_MapEditor::Manager::cutCollisions()
{
	//Save map history for ctrl+z
	this->mapStack.saveHistory();

	// Delete the selected rectangles in the current map, but leave them in hand
	for (auto& selectedRect : this->selectedCollisionRects)
	{
		this->map().collisionRects.erase(selectedRect);
	}
}

