#ifndef __SKO_MAPEDITOR_MAINMENUGUI_H_
#define __SKO_MAPEDITOR_MAINMENUGUI_H_

#include "GuiManager.h"
#include "FontManager.h"
#include "TextLabel.h"
#include "Button.h"
#include "Panel.h"
#include <functional>
#include "ButtonToggleGroup.h"
#include "MapEditorManager.h"

namespace SKO_MapEditor
{

	class Manager;
	class MapEditorGui
	{
	public:
		MapEditorGui(SKO_MapEditor::Manager *mapEditor);
		virtual ~MapEditorGui();

		// Helper functions
		void setCoords(std::string coordsValue);
		void setFps(std::string fpsValue);
		void setMeta(std::string metaValue);
		void setMapTitle(std::string mapTitle);
	private:
		OPI_Gui::GuiManager* guiManager = nullptr;
		SKO_MapEditor::Manager* mapEditor = nullptr;
		OPI_Text::TextComponent* coordsTextComponent = nullptr;
		OPI_Text::TextComponent* fpsTextComponent = nullptr;
		OPI_Text::TextComponent* mapTitleComponent = nullptr;
		OPI_Text::TextComponent* mapMetaComponent = nullptr;
		OPI_Gui::Panel* savePanel;

		// Create Gui Elements
		void setupCoordinates();
		void setupMapTitle();
		void setupFpsCounter();
		void setupMapMeta();
		void setupSaveBar();
	};
}
#endif
