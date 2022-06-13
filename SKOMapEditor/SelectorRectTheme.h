#ifndef __OPI_GUI_SelectorRectTheme_
#define __OPI_GUI_SelectorRectTheme_

#include "SDL_headers.h"
#include "OPI_Image.h"

namespace OPI_Gui
{
	class SelectorRect;

	class SelectorRectTheme
	{
	public:
		SelectorRectTheme();
		virtual ~SelectorRectTheme();
		void render(OPI_Gui::SelectorRect* element);

		// Borderless
		SelectorRectTheme(unsigned char fillRed, unsigned char fillGreen, unsigned char fillBlue, unsigned char fillAlpha);

		// BorderFill
		SelectorRectTheme(unsigned char thickness, unsigned char borderRed, unsigned char borderGreen, unsigned char borderBlue, unsigned char borderAlpha,
			unsigned char fillRed, unsigned char fillGreen, unsigned char fillBlue, unsigned char fillAlpha);

		// BorderOnly
		SelectorRectTheme(unsigned char thickness, 
			unsigned char borderRed, unsigned char borderGreen, unsigned char borderBlue, unsigned char borderAlpha);
		
	private:
		unsigned char thickness = 0;
		unsigned char borderRed = 0;
		unsigned char borderGreen = 0;
		unsigned char borderBlue = 0;
		unsigned char borderAlpha = 0;
		unsigned char fillRed = 0;
		unsigned char fillGreen = 0;
		unsigned char fillBlue = 0;
		unsigned char fillAlpha = 0;
	};
}

#endif
