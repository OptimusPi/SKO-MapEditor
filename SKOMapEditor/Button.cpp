#include "Button.h"
#include "ButtonTheme.h"
#include "ButtonToggleGroup.h"

OPI_Gui::Button::Button(std::string themeImage, int x, int y)
{
	this->theme = OPI_Gui::ButtonThemeFactory::GetTheme(OPI_Gui::ButtonThemeType::ButtonIcon, themeImage);
	this->x = x;
	this->y = y;
	this->width = width < this->theme->getMinimumWidth() ? this->theme->getMinimumWidth() : width;
	this->height = height < this->theme->getMinimumHeight() ? this->theme->getMinimumHeight() : height;
	this->width = this->width > this->theme->getMaximumWidth() && this->theme->getMaximumWidth() > 0 ? this->theme->getMinimumWidth() : this->width;
	this->height = this->height > this->theme->getMaximumHeight() && this->theme->getMaximumHeight() > 0 ? this->theme->getMinimumHeight() : this->height;
	this->theme->render(this);
}

OPI_Gui::Button::Button(std::string theme, int x, int y, OPI_Text::TextComponent * text)
{
	this->theme = OPI_Gui::ButtonThemeFactory::GetTheme(OPI_Gui::ButtonThemeType::ButtonTemplate, theme);
	this->x = x;
	this->y = y;
	this->width = width < this->theme->getMinimumWidth() ? this->theme->getMinimumWidth() : width;
	this->height = height < this->theme->getMinimumHeight() ? this->theme->getMinimumHeight() : height;
	this->width = this->width > this->theme->getMaximumWidth() && this->theme->getMaximumWidth() > 0 ? this->theme->getMinimumWidth() : this->width;
	this->height = this->height > this->theme->getMaximumHeight() && this->theme->getMaximumHeight() > 0 ? this->theme->getMinimumHeight() : this->height;
	this->theme->render(this);

	this->text = text;
	this->texture = nullptr;
	auto buttonText = text;
	int buttonTextX = (this->theme->getMinimumWidth() - buttonText->contentRender->width) / 2;
	int buttonTextY = (this->theme->getMinimumHeight() - buttonText->contentRender->height) / 2;
	auto textLabel = new OPI_Gui::TextLabel(buttonTextX, buttonTextY, buttonText);
	this->addElement(textLabel);
}

void OPI_Gui::Button::onMouseClick(std::function<void()> mouseClickCallback)
{
	this->mouseClickCallback = mouseClickCallback;
}

void OPI_Gui::Button::setCursor(OPI_Gui::CursorType cursor)
{
	OPI_Gui::GuiManager::setCursor(cursor);
}

bool OPI_Gui::Button::clickableContainsMouse(int mouseX, int mouseY)
{
	if (mouseX < this->x || mouseX >= this->x + this->width)
		return false;
	if (mouseY < this->y || mouseY >= this->y + this->height)
		return false;

	return true;
}

void OPI_Gui::Button::toggleOff()
{
	this->isToggleOn = false;
	this->theme->render(this);
}

void OPI_Gui::Button::toggleOn()
{
	this->isToggleOn = true;
	this->theme->render(this);
}

bool OPI_Gui::Button::handleMouseMove(int mouseX, int mouseY)
{
	if (!this->isEnabled)
		return false;

	if (clickableContainsMouse(mouseX, mouseY))
	{
		this->isSelected = true;
		this->theme->render(this);
		setCursor(OPI_Gui::CursorType::Hand);
		return true;
	}
	else
	{
		if (this->isSelected)
		{
			//setCursor(OPI_Gui::CursorType::Normal);
			this->isPressed = false;
			this->isSelected = false;
			this->theme->render(this);
		}
	}
	return false;
}

bool OPI_Gui::Button::handleMousePressLeft(int mouseX, int mouseY)
{
	if (!this->isEnabled)
		return false;

	if (clickableContainsMouse(mouseX, mouseY))
	{
		this->isPressed = true;
		this->theme->render(this);
		setCursor(OPI_Gui::CursorType::Hand);

		if (this->isToggle)
		{
			// Flip the switch on this toggle button.
			this->isToggleOn = !this->isToggleOn;

			// Process the entire group, if this is to be treated like a radio button.
			for (auto toggleGroup : this->toggleGroups)
			{
				toggleGroup->processToggle(this);
			}
		}

		return true;
	}
	return false;
}

bool OPI_Gui::Button::handleMouseReleaseLeft(int mouseX, int mouseY)
{
	if (!this->isEnabled)
		return false;

	if (clickableContainsMouse(mouseX, mouseY))
	{
		this->isPressed = false;
		this->theme->render(this);

		if (this->mouseClickCallback == nullptr)
			return false;

		this->mouseClickCallback();
		return true;
	}
	else
	{
		if (this->isPressed)
		{
			this->isPressed = false;
			this->isSelected = false;
			this->theme->render(this);
			return true;
		}
	}

	return false;
}


bool OPI_Gui::Button::handleMousePressRight(int mouseX, int mouseY)
{
	return false;
}

bool OPI_Gui::Button::handleMouseReleaseRight(int mouseX, int mouseY)
{
	return false;
}

OPI_Gui::Button::~Button()
{

}