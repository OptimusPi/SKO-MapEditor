#include <typeinfo>
#include "ButtonThemeFactory.h"
#include "ButtonTheme.h"
#include <stdexcept>

/// Singleton instance
OPI_Gui::ButtonThemeFactory * OPI_Gui::ButtonThemeFactory::instance;
OPI_Gui::ButtonThemeFactory * OPI_Gui::ButtonThemeFactory::getInstance()
{
	if (!OPI_Gui::ButtonThemeFactory::instance)
	{
		OPI_Gui::ButtonThemeFactory::instance = new OPI_Gui::ButtonThemeFactory;
	}

	return OPI_Gui::ButtonThemeFactory::instance;
}

OPI_Gui::ButtonTheme * OPI_Gui::ButtonThemeFactory::GetTheme(OPI_Gui::ButtonThemeType type, std::string theme)
{
	return getInstance()->getTheme(type, theme);
}

OPI_Gui::ButtonTheme * OPI_Gui::ButtonThemeFactory::getTheme(OPI_Gui::ButtonThemeType type, std::string theme)
{
	// Create compisite key for the dictionary lookup in the theme cache
	// Example: "ButtonTemplate|ice" or "ButtonIcon|stone"
	std::string key = generateKey(type, theme);

	// If not in memory cache, load from disk
	if (this->themes.count(key) == 0)
	{
		switch (type)
		{
		case ButtonThemeType::ButtonTemplate:
			this->loadTheme_Button(theme);
			break;
		case ButtonThemeType::ButtonIcon:
			this->loadTheme_ButtonIcon(theme);
			break;
		default:
			throw std::logic_error("Invalid type provided to OPI_Gui::ButtonThemeFactory!");
		}
	}

	// Return Element Theme from memory cache
	return this->themes.at(key);
}

void OPI_Gui::ButtonThemeFactory::loadTheme_Button(std::string theme)
{
	OPI_Gui::ButtonTheme *PanelThemeButton = new OPI_Gui::ButtonTheme();
	std::string path = "IMG/GUI/themes/button/" + theme + "/";
	PanelThemeButton->textureEnabled = new OPI_Image(path + "enabled.png");
	PanelThemeButton->textureDisabled = new OPI_Image(path + "disabled.png");
	PanelThemeButton->texturePressed = new OPI_Image(path + "pressed.png");
	PanelThemeButton->textureSelected = new OPI_Image(path + "selected.png");
	PanelThemeButton->textureToggled = new OPI_Image(path + "toggled.png");
	PanelThemeButton->title = theme;

	//insert into cache
	std::string key = generateKey(OPI_Gui::ButtonThemeType::ButtonTemplate, theme);
	this->themes.insert({ {key, PanelThemeButton} });
}


void OPI_Gui::ButtonThemeFactory::loadTheme_ButtonIcon(std::string themeImage)
{
	OPI_Gui::ButtonTheme *PanelThemeButtonIcon = new OPI_Gui::ButtonTheme();
	std::string path = "IMG/GUI/themes/button_images/" + themeImage;
	auto surface = OPI_Image::getSurface(path + ".png");

	// Enabled button - normal
	PanelThemeButtonIcon->textureEnabled = new OPI_Image(surface);

	// Disabled button - greyed-out
	SDL_Surface *disabledSurface = OPI_Image::tintSurface(surface, 120, 120, 120, 250);
	PanelThemeButtonIcon->textureDisabled = new OPI_Image(disabledSurface);
	
	// Selected button
	SDL_Surface *selectedSurface = OPI_Image::tintSurface(surface, 200, 200, 255, 250);
	PanelThemeButtonIcon->textureSelected = new OPI_Image(selectedSurface);

	// Pressed button
	SDL_Surface *pressedSurface = OPI_Image::tintSurface(surface, 150, 150, 255, 200);
	PanelThemeButtonIcon->texturePressed = new OPI_Image(pressedSurface);

	// Toggled button
	SDL_Surface* toggledSurface = OPI_Image::tintSurface(surface, 180, 180, 255, 200);
	PanelThemeButtonIcon->textureToggled = new OPI_Image(toggledSurface);

	//insert into cache
	std::string key = generateKey(OPI_Gui::ButtonThemeType::ButtonIcon, themeImage);
	this->themes.insert({ {key, PanelThemeButtonIcon} });
}

std::string OPI_Gui::ButtonThemeFactory::generateKey(OPI_Gui::ButtonThemeType type, std::string theme)
{
	std::string key = "";

	switch (type)
	{
	case OPI_Gui::ButtonThemeType::ButtonTemplate:
		key += "ButtonTemplate|";
		break;
	case OPI_Gui::ButtonThemeType::ButtonIcon:
		key += "ButtonIcon|";
		break;
	default:
		throw std::invalid_argument("The provided ButtonThemeType is invalid.");
	}

	key += theme;
	return key;
}

OPI_Gui::ButtonThemeFactory::ButtonThemeFactory()
{
	
}


OPI_Gui::ButtonThemeFactory::~ButtonThemeFactory()
{
}
