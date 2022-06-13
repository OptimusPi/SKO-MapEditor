#ifndef __OPI_GUI_PanelTheme_
#define __OPI_GUI_PanelTheme_

#include <string>
#include "Element.h"

namespace OPI_Gui
{
	class PanelTheme
	{
	public:
		PanelTheme() {};
		~PanelTheme() {};
		virtual void render(OPI_Gui::Element *element) = 0;
		virtual int getMinimumWidth() = 0;
		virtual int getMinimumHeight() = 0;
		virtual int getMaximumWidth() = 0;
		virtual int getMaximumHeight() = 0;
	private:

	};
}
  
#endif
