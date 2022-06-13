#include <typeinfo>
#include "SelectorRectThemeFactory.h"
#include "SelectorRectTheme.h"
#include <stdexcept>

OPI_Gui::SelectorRectTheme *
OPI_Gui::SelectorRectThemeFactory::GetTheme(OPI_Gui::SelectorRectThemeType themeType,
		unsigned char thickness, unsigned char borderRed, unsigned char borderGreen, unsigned char borderBlue, unsigned char borderAlpha, 
		unsigned char fillRed, unsigned char fillGreen, unsigned char fillBlue, unsigned char fillAlpha)
{
	OPI_Gui::SelectorRectTheme* theme = nullptr;
	switch (themeType)
	{
		case OPI_Gui::SelectorRectThemeType::BorderFill:
			return new SelectorRectTheme(thickness,  borderRed,  borderGreen,  borderBlue,  borderAlpha,
				 fillRed,  fillGreen,  fillBlue,  fillAlpha);
		break;

		case OPI_Gui::SelectorRectThemeType::Borderless:
			return new SelectorRectTheme(fillRed, fillGreen, fillBlue, fillAlpha);
		break;

		case OPI_Gui::SelectorRectThemeType::BorderOnly:
			return new SelectorRectTheme(thickness, borderRed, borderGreen, borderBlue, borderAlpha);
		break;

		default:
			throw std::logic_error("Invalid type provided to OPI_Gui::SelectorRectThemeFactory!");
	}
}

OPI_Gui::SelectorRectThemeFactory::SelectorRectThemeFactory()
{

}


OPI_Gui::SelectorRectThemeFactory::~SelectorRectThemeFactory()
{
}

