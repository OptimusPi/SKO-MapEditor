#include "OPI_Image.h"
#include "PanelTheme.h"

#ifndef __OPI_GUI_PanelThemeIMAGE_
#define __OPI_GUI_PanelThemeIMAGE_

namespace OPI_Gui
{
	class PanelThemeImage : public PanelTheme
	{
	public:
		PanelThemeImage();
		virtual ~PanelThemeImage();
		int getMinimumWidth();
		int getMinimumHeight();
		int getMaximumWidth();
		int getMaximumHeight();
		void render(OPI_Gui::Element* element);
		OPI_Image* texture;
	private:
	};
}

#endif
