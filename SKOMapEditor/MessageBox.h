#ifndef	__OPI_GUIMESSAGEBOX_
#define __OPI_GUIMESSAGEBOX_

#include "Element.h"
#include "MessageBoxType.h"
#include "PanelThemeType.h"
#include "PanelThemeFactory.h"
#include "ButtonThemeType.h"
#include "ButtonThemeFactory.h"
#include "OPI_Image.h"
#include "TextComponent.h"
#include "TextLabel.h"
#include "Button.h"
#include "Panel.h"
#include "Font.h"

namespace OPI_Gui
{
	class MessageBox : public Panel
	{
	public:
		MessageBox(std::string message, OPI_Text::Font* font, 
			bool wordWrap = false, PanelThemeType themeType = PanelThemeType::GridRect, std::string theme = "default");
		MessageBox(std::string message, MessageBoxType messageBoxType, OPI_Text::Font* font, 
			bool wordWrap = false, PanelThemeType themeType = PanelThemeType::GridRect, std::string theme = "default");
		virtual ~MessageBox();
		void setText(std::string message);
		
		// Input handlers
		bool isInteracting();

		PanelTheme *theme;
		MessageBoxType type = MessageBoxType::Okay;

		OPI_Text::TextComponent *message;
		OPI_Text::Font *font;
		bool wordWrap = false;
		const int DefaultPadding = 24;
		int buttonRowHeight = 0;
		int buttonHeight = 0;
		int buttonWidth = 0;

	private:
		void addButtons();
		void addText();
	};
}

#endif

