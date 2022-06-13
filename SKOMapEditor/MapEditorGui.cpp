#include "MapEditorGui.h"
#include "MapEditorManager.h"

SKO_MapEditor::MapEditorGui::MapEditorGui(SKO_MapEditor::Manager* mapEditor)
{
	this->guiManager = mapEditor->gui;
	this->mapEditor = mapEditor;

	setupCoordinates();
	setupMapTitle();
	setupMapMeta();
	setupFpsCounter();
	setupSaveBar();

}

SKO_MapEditor::MapEditorGui::~MapEditorGui()
{

}

void SKO_MapEditor::MapEditorGui::setCoords(std::string coordsValue)
{ 
	if (coordsValue != this->coordsTextComponent->content)
	{
		this->coordsTextComponent->setText(coordsValue);
	}
} 

void SKO_MapEditor::MapEditorGui::setFps(std::string fpsValue)
{
	if (fpsValue != this->fpsTextComponent->content)
	{
		this->fpsTextComponent->setText(fpsValue);
	}
}

void SKO_MapEditor::MapEditorGui::setMeta(std::string metaValue)
{
	this->mapMetaComponent->setText(metaValue);
}

void SKO_MapEditor::MapEditorGui::setMapTitle(std::string mapTitle)
{
	this->mapTitleComponent->setText(mapTitle);
}

void SKO_MapEditor::MapEditorGui::setupCoordinates()
{
	// Set up coordinates text
	this->coordsTextComponent = new OPI_Text::TextComponent("(0, 0)", OPI_Text::FontManager::getFont("RobotoMono-Regular"), 20);

	this->coordsTextComponent->visible = true;
	this->coordsTextComponent->x = 2;
	this->coordsTextComponent->y = 2;
	this->coordsTextComponent->R = 0.90f;
	this->coordsTextComponent->G = 0.90f;
	this->coordsTextComponent->B = 1.00f;

	// Set up text label
	OPI_Gui::TextLabel * coordsLabel = new OPI_Gui::TextLabel(this->coordsTextComponent->x, this->coordsTextComponent->y, this->coordsTextComponent);

	// Add coords to gui manager
	this->guiManager->addElement(coordsLabel);
}


void SKO_MapEditor::MapEditorGui::setupSaveBar()
{
	// 8 buttons with dimensions 64x64
	auto panelWidth = 100;
	auto panelHeight = 100;

	// center horizontally, place at bottom of the screen
	auto panelX = guiManager->getScreenWidth() - panelWidth;
	auto panelY = 16;


	this->savePanel = new OPI_Gui::Panel(OPI_Gui::PanelThemeType::GridRect, "ice", panelX, panelY, panelWidth, panelHeight);
	savePanel->isClosable = false;
	savePanel->isMovable = true;
	savePanel->isResizable = false;
	savePanel->isVisible = true;
	auto buttonSave = new OPI_Gui::Button("save", 18, 18);

	buttonSave->onMouseClick([this]() {
		this->mapEditor->saveMap();
	});
	savePanel->addElement(buttonSave);

	// Add hotbar panel to gui manager
	this->guiManager->addElement(savePanel);
}

void SKO_MapEditor::MapEditorGui::setupMapTitle()
{
	// Set up coordinates text
	this->mapTitleComponent = new OPI_Text::TextComponent("Untitled Map", OPI_Text::FontManager::getFont("RobotoMono-Regular"), 20);

	this->mapTitleComponent->visible = true;
	this->mapTitleComponent->x = 600;
	this->mapTitleComponent->y = 2;
	this->mapTitleComponent->R = 0.90f;
	this->mapTitleComponent->G = 0.90f;
	this->mapTitleComponent->B = 1.00f;

	// Set up text label
	OPI_Gui::TextLabel* mapTitleLabel = new OPI_Gui::TextLabel(this->mapTitleComponent->x, this->mapTitleComponent->y, this->mapTitleComponent);

	// Add coords to save panel
	this->guiManager->addElement(mapTitleLabel);
}

void SKO_MapEditor::MapEditorGui::setupMapMeta()
{
	// Set up coordinates text
	this->mapMetaComponent = new OPI_Text::TextComponent("Meta: ", OPI_Text::FontManager::getFont("RobotoMono-Regular"), 20);

	this->mapMetaComponent->visible = true;
	this->mapMetaComponent->x = this->coordsTextComponent->x;
	this->mapMetaComponent->y = this->coordsTextComponent->y + this->coordsTextComponent->height + 2;
	this->mapMetaComponent->R = 0.90f;
	this->mapMetaComponent->G = 0.90f;
	this->mapMetaComponent->B = 1.00f;

	// Set up text label
	OPI_Gui::TextLabel* mapMetaLabel = new OPI_Gui::TextLabel(this->mapMetaComponent->x, this->mapMetaComponent->y, this->mapMetaComponent);

	// Add coords to gui manager
	this->guiManager->addElement(mapMetaLabel);
}


void SKO_MapEditor::MapEditorGui::setupFpsCounter()
{
	// Set up coordinates text
	this->fpsTextComponent = new OPI_Text::TextComponent("0 FPS", OPI_Text::FontManager::getFont("RobotoMono-Regular"), 20);

	this->fpsTextComponent->visible = true;
	this->fpsTextComponent->x = this->mapMetaComponent->x;
	this->fpsTextComponent->y = this->mapMetaComponent->y + this->mapMetaComponent->height + 2;
	this->fpsTextComponent->R = 0.90f;
	this->fpsTextComponent->G = 0.90f;
	this->fpsTextComponent->B = 1.00f;

	// Set up text label
	OPI_Gui::TextLabel* fpsLabel = new OPI_Gui::TextLabel(this->fpsTextComponent->x, this->fpsTextComponent->y, this->fpsTextComponent);

	// Add coords to gui manager
	this->guiManager->addElement(fpsLabel);
}


