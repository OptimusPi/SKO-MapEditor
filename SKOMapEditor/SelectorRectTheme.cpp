#include "SelectorRectTheme.h"
#include "SelectorRect.h"

OPI_Gui::SelectorRectTheme::SelectorRectTheme(unsigned char fillRed, unsigned char fillGreen, unsigned char fillBlue, unsigned char fillAlpha)
{
	this->borderRed = borderRed;
	this->borderGreen = borderGreen;
	this->borderBlue = borderBlue;
	this->borderAlpha = borderAlpha;
	this->thickness = thickness;
}


OPI_Gui::SelectorRectTheme::SelectorRectTheme(unsigned char thickness, unsigned char borderRed, unsigned char borderGreen, unsigned char borderBlue, unsigned char borderAlpha,
	unsigned char fillRed, unsigned char fillGreen, unsigned char fillBlue, unsigned char fillAlpha)
{
	this->fillRed = fillRed;
	this->fillGreen = fillGreen;
	this->fillBlue = fillBlue;
	this->fillAlpha = fillAlpha;
	this->borderRed = borderRed;
	this->borderGreen = borderGreen;
	this->borderBlue = borderBlue;
	this->borderAlpha = borderAlpha;
	this->thickness = thickness;
}

OPI_Gui::SelectorRectTheme::SelectorRectTheme(unsigned char thickness, unsigned char borderRed, unsigned char borderGreen, unsigned char borderBlue, unsigned char borderAlpha)
{
	this->borderRed = borderRed;
	this->borderGreen = borderGreen;
	this->borderBlue = borderBlue;
	this->borderAlpha = borderAlpha;
	this->thickness = thickness;
}

OPI_Gui::SelectorRectTheme::SelectorRectTheme()
{
}

OPI_Gui::SelectorRectTheme::~SelectorRectTheme()
{
}

void OPI_Gui::SelectorRectTheme::render(OPI_Gui::SelectorRect* element)
{
	auto fill = OPI_Image::createColoredSurface(element->width, element->height, fillRed, fillGreen, fillBlue, fillAlpha);
	auto border = OPI_Image::createBorderSurface(element->width, element->height, borderRed, borderGreen, borderBlue, borderAlpha, thickness);
	
	auto texture = new OPI_Image(OPI_Image::copyOntoSurface(border, fill));

	//OPI_Image::createColoredBorderedSurface(32, 32, 0, 50, 250, 64, 1);
	//auto tileSelectorImg = new OPI_Image(tileSelectorSurface);

	element->setTexture(texture);
}
