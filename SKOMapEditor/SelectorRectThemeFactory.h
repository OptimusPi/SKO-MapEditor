#ifndef __OPI_GUI_SELECTORRECT_THEMEFACTORY_H_
#define __OPI_GUI_SELECTORRECT_THEMEFACTORY_H_

#include "SelectorRectThemeType.h"
#include "SelectorRectTheme.h"

namespace OPI_Gui
{
	class SelectorRectThemeFactory
	{
	public:
		static OPI_Gui::SelectorRectTheme *GetTheme(OPI_Gui::SelectorRectThemeType themeType,
			unsigned char thickness,
			unsigned char borderRed, unsigned char borderGreen, unsigned char borderBlue, unsigned char borderAlpha,
			unsigned char fillRed, unsigned char fillGreen, unsigned char fillBlue, unsigned char fillAlpha);
	private:
		SelectorRectThemeFactory();
		virtual ~SelectorRectThemeFactory();
	};
}

#endif