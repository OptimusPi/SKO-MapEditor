#ifndef __OPI_GUI_SELECTORRECT_
#define __OPI_GUI_SELECTORRECT_

#include "OPI_Image.h"
#include "Element.h"
#include "SelectorRectThemeFactory.h"
#include "GuiManager.h"

#include "SDL_headers.h"

#include <vector>
#include <string>


namespace OPI_Gui
{
	class SelectorRectTheme;
	class SelectorRect : public Element
	{
	public:
		SelectorRect(OPI_Gui::SelectorRectThemeType themeType, int x, int y, 
			unsigned short width, unsigned short height, unsigned char thickness,
			unsigned char borderRed, unsigned char borderGreen, unsigned char borderBlue, unsigned char borderAlpha, 
			unsigned char fillRed = 0, unsigned char fillGreen = 0, unsigned char fillBlue = 0, unsigned char fillAlpha = 0);
		virtual ~SelectorRect();
		void setWidthHeight(unsigned short width, unsigned short height);
		void render();

		// Input handlers
		bool handleMouseMove(int mouseX, int mouseY);
		bool handleMousePressLeft(int mouseX, int mouseY);
		bool handleMousePressRight(int mouseX, int mouseY);
		bool handleMouseReleaseRight(int mouseX, int mouseY);
		bool handleMouseReleaseLeft(int mouseX, int mouseY);

	private:
		SelectorRectTheme* theme;

	};
}

#endif

