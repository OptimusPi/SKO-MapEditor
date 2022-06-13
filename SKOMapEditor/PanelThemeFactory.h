#ifndef __OPI_GUI_PanelThemeFactory_
#define __OPI_GUI_PanelThemeFactory_

#include <string>
#include <map>

#include "PanelTheme.h"
#include "PanelThemeType.h"

namespace OPI_Gui
{
	/// Singleton
	class PanelThemeFactory
	{
	public:
		static OPI_Gui::PanelTheme *GetTheme(OPI_Gui::PanelThemeType themeType, std::string theme);
		
	private:
		static PanelThemeFactory* getInstance();
		static PanelThemeFactory *instance;
		PanelThemeFactory();
		virtual ~PanelThemeFactory();

		// Grab theme from cache if it exists, 
		// else load it from disk.
		OPI_Gui::PanelTheme *getTheme(OPI_Gui::PanelThemeType themeType, std::string theme);

		// Load themes from disk and add them to cache
		void loadTheme_GridRect(std::string theme);
		void loadTheme_Image(std::string theme);

		// Get a unique key to look up in the dictionary
		std::string generateKey(OPI_Gui::PanelThemeType themeType, std::string theme);

		// Dictionary to cache all the themes
		std::map<std::string, OPI_Gui::PanelTheme*> themes;
	};
}

#endif