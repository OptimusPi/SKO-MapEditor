#include "SelectorRect.h"
#include "SelectorRectTheme.h"

OPI_Gui::SelectorRect::SelectorRect(OPI_Gui::SelectorRectThemeType themeType, int x, int y,
	unsigned short width, unsigned short height, unsigned char thickness,
	unsigned char borderRed, unsigned char borderGreen, unsigned char borderBlue, unsigned char borderAlpha,
	unsigned char fillRed, unsigned char fillGreen, unsigned char fillBlue, unsigned char fillAlpha)
{
	this->theme = OPI_Gui::SelectorRectThemeFactory::GetTheme(themeType, thickness,
		borderRed, borderGreen, borderBlue, borderAlpha,
		fillRed, fillGreen, fillBlue, fillAlpha);
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	render();
}

OPI_Gui::SelectorRect::~SelectorRect() {}


void OPI_Gui::SelectorRect::setWidthHeight(unsigned short width, unsigned short height)
{
	if (this->width == width && this->height == height)
	{
		// return without rendering again
	}

	this->width = width;
	this->height = height;
	this->render();
};

void OPI_Gui::SelectorRect::render()
{
	this->theme->render(this);
}

bool OPI_Gui::SelectorRect::handleMouseMove(int mouseX, int mouseY)
{
	return false;
}

bool OPI_Gui::SelectorRect::handleMousePressLeft(int mouseX, int mouseY)
{
	return false;
}

bool OPI_Gui::SelectorRect::handleMousePressRight(int mouseX, int mouseY)
{
	return false;
}

bool OPI_Gui::SelectorRect::handleMouseReleaseRight(int mouseX, int mouseY)
{
	return false;
}

bool OPI_Gui::SelectorRect::handleMouseReleaseLeft(int mouseX, int mouseY)
{
	return false;
}


