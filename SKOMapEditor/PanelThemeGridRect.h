

#ifndef __OPI_GUI_PanelThemeGRIDRECT_
#define __OPI_GUI_PanelThemeGRIDRECT_

#include "PanelTheme.h"

namespace OPI_Gui
{
	class PanelThemeGridRect : public PanelTheme
	{
	public:
		PanelThemeGridRect();
		virtual ~PanelThemeGridRect();
		int getMinimumWidth();
		int getMinimumHeight();
		int getMaximumWidth();
		int getMaximumHeight();
		void render(OPI_Gui::Element* element);
		unsigned short int tileWidth;
		unsigned short int tileHeight;
		SDL_Surface *corners[4];
		SDL_Surface *edges[4];
		SDL_Surface *filler;
	private:
		
	};
}


#endif
