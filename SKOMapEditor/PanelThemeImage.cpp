#include "PanelThemeImage.h"


OPI_Gui::PanelThemeImage::PanelThemeImage()
{
}


OPI_Gui::PanelThemeImage::~PanelThemeImage()
{
}

int OPI_Gui::PanelThemeImage::getMinimumWidth()
{
	return this->texture->width;
}

int OPI_Gui::PanelThemeImage::getMinimumHeight()
{
	return this->texture->height;
}

int OPI_Gui::PanelThemeImage::getMaximumWidth()
{
	return this->texture->width;
}

int OPI_Gui::PanelThemeImage::getMaximumHeight()
{
	return this->texture->height;
}

void OPI_Gui::PanelThemeImage::render(OPI_Gui::Element * element)
{
	element->setTexture(this->texture);
}
