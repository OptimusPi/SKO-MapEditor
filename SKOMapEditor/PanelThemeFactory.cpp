#include <typeinfo>

#include "PanelThemeFactory.h"
#include "PanelThemeGridRect.h"
#include "PanelThemeImage.h"
#include "PanelTheme.h"
#include <stdexcept>

/// Singleton instance
OPI_Gui::PanelThemeFactory * OPI_Gui::PanelThemeFactory::instance;
OPI_Gui::PanelThemeFactory * OPI_Gui::PanelThemeFactory::getInstance()
{
	if (!OPI_Gui::PanelThemeFactory::instance)
	{
		OPI_Gui::PanelThemeFactory::instance = new OPI_Gui::PanelThemeFactory;
	}

	return OPI_Gui::PanelThemeFactory::instance;
}

OPI_Gui::PanelTheme * OPI_Gui::PanelThemeFactory::GetTheme(OPI_Gui::PanelThemeType themeType, std::string theme)
{
	return 	getInstance()->getTheme(themeType, theme);
}

OPI_Gui::PanelTheme * OPI_Gui::PanelThemeFactory::getTheme(OPI_Gui::PanelThemeType themeType, std::string theme)
{
	// Create compisite key for the dictionary lookup in the theme cache
	// Example: "PanelThemeGridRect|ice" or "PanelThemeImage|stone"
	std::string key = generateKey(themeType, theme);
	OPI_Gui::PanelTheme *panelTheme = nullptr;

	// If not in memory cache, load from disk
	if (this->themes.count(key) == 0)
	{
		switch (themeType)
		{

		case PanelThemeType::GridRect:
			this->loadTheme_GridRect(theme);
			break;
		case PanelThemeType::Image: 
			 this->loadTheme_Image(theme);
			break;

		default:
			throw std::logic_error("Invalid type provided to OPI_Gui::PanelThemeFactory!");
		}
	}

	// Return Element Theme from memory cache
	return this->themes.at(key);
}

void OPI_Gui::PanelThemeFactory::loadTheme_Image(std::string theme)
{
	OPI_Gui::PanelThemeImage *panelThemeImage = new OPI_Gui::PanelThemeImage();
	std::string themePath = theme;
	panelThemeImage->texture = new OPI_Image(themePath);

	// Insert into cache
	std::string key = generateKey(OPI_Gui::PanelThemeType::Image, theme);
	this->themes.insert({ {key, panelThemeImage} });
}

void OPI_Gui::PanelThemeFactory::loadTheme_GridRect(std::string theme)
{
	OPI_Gui::PanelThemeGridRect *panelThemeGridRect = new OPI_Gui::PanelThemeGridRect();

	std::string themePath = "IMG/GUI/themes/" + theme + "/";

	//Load template sprite, and then cut out the corners and middle
	SDL_Surface *panelTemplate = OPI_Image::getSurface(themePath + "panel.png");

	//Ensure it loaded
	if (panelTemplate == NULL)
	{
		throw "Panel Template is NULL. Maybe the image is missing from IMG/GUI/themes/" + theme + "/panel.png";
	}

	//TODO simply use the inner width somehow
	// Ensure it is a 3x3 "tileset" 
	if (panelTemplate->w % 3 > 0 || panelTemplate->h % 3 > 0)
	{
		throw "Panel template dimensions are incorrect! Width and Height must be divisible by 3.";
	}

	// Determine dimensions automatiacally from the template
	panelThemeGridRect->tileWidth = panelTemplate->w / 3;
	panelThemeGridRect->tileHeight = panelTemplate->h / 3;
	SDL_Rect tileClip{ 0, 0, panelThemeGridRect->tileWidth, panelThemeGridRect->tileHeight };

	// Top-Left corner
	panelThemeGridRect->corners[0] = OPI_Image::createBlankSurface(panelThemeGridRect->tileWidth, panelThemeGridRect->tileHeight);
	tileClip.x = 0;
	tileClip.y = 0;
	SDL_BlitSurface(panelTemplate, &tileClip, panelThemeGridRect->corners[0], NULL);

	// Top-Right corner
	panelThemeGridRect->corners[1] = OPI_Image::createBlankSurface(panelThemeGridRect->tileWidth, panelThemeGridRect->tileHeight);
	tileClip.x = panelThemeGridRect->tileWidth * 2;
	tileClip.y = 0;
	SDL_BlitSurface(panelTemplate, &tileClip, panelThemeGridRect->corners[1], NULL);

	// Bottom-Left corner
	panelThemeGridRect->corners[2] = OPI_Image::createBlankSurface(panelThemeGridRect->tileWidth, panelThemeGridRect->tileHeight);
	tileClip.x = 0;
	tileClip.y = panelThemeGridRect->tileHeight * 2;
	SDL_BlitSurface(panelTemplate, &tileClip, panelThemeGridRect->corners[2], NULL);

	// Bottom-Right corner
	panelThemeGridRect->corners[3] = OPI_Image::createBlankSurface(panelThemeGridRect->tileWidth, panelThemeGridRect->tileHeight);
	tileClip.x = panelThemeGridRect->tileWidth * 2;
	tileClip.y = panelThemeGridRect->tileHeight * 2;
	SDL_BlitSurface(panelTemplate, &tileClip, panelThemeGridRect->corners[3], NULL);

	// Top Edge
	panelThemeGridRect->edges[0] = OPI_Image::createBlankSurface(panelThemeGridRect->tileWidth, panelThemeGridRect->tileHeight);
	tileClip.x = panelThemeGridRect->tileWidth;
	tileClip.y = 0;
	SDL_BlitSurface(panelTemplate, &tileClip, panelThemeGridRect->edges[0], NULL);

	// Bottom Edge 
	panelThemeGridRect->edges[1] = OPI_Image::createBlankSurface(panelThemeGridRect->tileWidth, panelThemeGridRect->tileHeight);
	tileClip.x = panelThemeGridRect->tileWidth;
	tileClip.y = panelThemeGridRect->tileHeight * 2;
	SDL_BlitSurface(panelTemplate, &tileClip, panelThemeGridRect->edges[1], NULL);

	// Left Edge
	panelThemeGridRect->edges[2] = OPI_Image::createBlankSurface(panelThemeGridRect->tileWidth, panelThemeGridRect->tileHeight);
	tileClip.x = 0;
	tileClip.y = panelThemeGridRect->tileHeight;
	SDL_BlitSurface(panelTemplate, &tileClip, panelThemeGridRect->edges[2], NULL);

	// Right Edge
	panelThemeGridRect->edges[3] = OPI_Image::createBlankSurface(panelThemeGridRect->tileWidth, panelThemeGridRect->tileHeight);
	tileClip.x = panelThemeGridRect->tileWidth * 2;
	tileClip.y = panelThemeGridRect->tileHeight;
	SDL_BlitSurface(panelTemplate, &tileClip, panelThemeGridRect->edges[3], NULL);

	// Filler
	panelThemeGridRect->filler = OPI_Image::createBlankSurface(panelThemeGridRect->tileWidth, panelThemeGridRect->tileHeight);
	tileClip.x = panelThemeGridRect->tileWidth;
	tileClip.y = panelThemeGridRect->tileHeight;
	SDL_BlitSurface(panelTemplate, &tileClip, panelThemeGridRect->filler, NULL);

	SDL_FreeSurface(panelTemplate);

	std::string key = generateKey(OPI_Gui::PanelThemeType::GridRect, theme);
	this->themes.insert({ {key, panelThemeGridRect} });
}

std::string OPI_Gui::PanelThemeFactory::generateKey(OPI_Gui::PanelThemeType themeType, std::string theme)
{
	std::string key = "";

	switch (themeType)
	{
	case OPI_Gui::PanelThemeType::Image:
		key += "Image|";
		break;
	case OPI_Gui::PanelThemeType::GridRect:
		key += "GridRect|";
		break;
	default:
		throw std::invalid_argument("The provided PanelThemeType is invalid.");
	}

	key += theme;
	return key;
}

OPI_Gui::PanelThemeFactory::PanelThemeFactory()
{
}


OPI_Gui::PanelThemeFactory::~PanelThemeFactory()
{
}
