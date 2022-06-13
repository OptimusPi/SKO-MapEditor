#include "Panel.h"

OPI_Gui::Panel::Panel(int x, int y, int width, int height)
{
	this->theme = nullptr;
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
}

OPI_Gui::Panel::Panel(OPI_Gui::PanelThemeType type, std::string theme, int x, int y, int width, int height)
{
	this->theme = OPI_Gui::PanelThemeFactory::GetTheme(type, theme);
	this->x = x;
	this->y = y;
	this->width = width < this->theme->getMinimumWidth() ? this->theme->getMinimumWidth() : width;
	this->height = height < this->theme->getMinimumHeight() ? this->theme->getMinimumHeight() : height;
	this->width = this->width > this->theme->getMaximumWidth() && this->theme->getMaximumWidth() > 0 ? this->theme->getMinimumWidth() : this->width;
	this->height = this->height > this->theme->getMaximumHeight() && this->theme->getMaximumHeight() > 0 ? this->theme->getMinimumHeight() : this->height;
	render();
}

OPI_Gui::Panel::~Panel() {}

void OPI_Gui::Panel::onMouseClick(std::function<void(int x, int y)> mouseClickCallback)
{
	this->mouseClickCallback = mouseClickCallback;
}

void OPI_Gui::Panel::onMouseMotion(std::function<void(int x, int y)> mouseMotionCallback)
{
	this->mouseMotionCallback = mouseMotionCallback;
}

void OPI_Gui::Panel::render()
{
	this->theme->render(this);
}

void OPI_Gui::Panel::setWidth(short int width)
{
	int minimumWidth = this->theme->getMinimumWidth();
	if (width < minimumWidth)
		width = minimumWidth;

	this->width = width;
	render();
}

void OPI_Gui::Panel::setHeight(short int height)
{
	int minimumHeight = this->theme->getMinimumHeight();
	if (height < minimumHeight)
		height = minimumHeight;

	this->height = height;
	render();
}

void OPI_Gui::Panel::setWidthHeight(short int width, short int height)
{
	int minimumWidth = this->theme->getMinimumWidth();
	if (width < minimumWidth)
		width = minimumWidth;

	this->width = width;

	int minimumHeight = this->theme->getMinimumHeight();
	if (height < minimumHeight)
		height = minimumHeight;

	this->height = height;

	render();
}

bool OPI_Gui::Panel::containsMouse(int mouseX, int mouseY, int x, int y, int w, int h)
{
	// If it's off to one side, it is not contained.
	if (mouseX < x || mouseX >= x+w)
		return false;

	// If it's above or below, it is not contained. 
	if (mouseY < y || mouseY >= y+h)
		return false;

	return true;
}

// TODO - allow customizable moveable grab area
bool OPI_Gui::Panel::movableContainsMouse(int mouseX, int mouseY)
{
	//if (moveableGrabArea == NULL)
	//	return false;

	return containsMouse(mouseX, mouseY, this->x, this->y, this->width - 24, 24);
}

// TODO - allow customizable resizable grab area
bool OPI_Gui::Panel::resizableContainsMouse(int mouseX, int mouseY)
{
	//if (resizableGrabArea == NULL)
	//	return false;

	return containsMouse(mouseX, mouseY, this->x + this->width - 24, this->y + this->height - 24, 24, 24);
}

// TODO - allow customizable closable grab area
bool OPI_Gui::Panel::closableContainsMouse(int mouseX, int mouseY)
{
	//if (closableGrabArea == NULL)
	//	return false;

	return containsMouse(mouseX, mouseY, this->x + this->width - 24, this->y, 24, 24);
}


bool OPI_Gui::Panel::handleSection_Move(int mouseX, int mouseY)
{
	if (!this->isVisible)
	{
		return false;
	}

	if (this->isMoving)
	{
		this->x = moveOriginX + mouseX - moveOriginGrabX;
		this->y = moveOriginY + mouseY - moveOriginGrabY;

		// Contain Element inside the screen
		ensureBounds();

		// Signal that event has been handled
		return true;
	}

	// Set move cursor if inside the grabbable move bar
	if (movableContainsMouse(mouseX, mouseY))
	{
		OPI_Gui::GuiManager::setCursor(CursorType::Move);
		// Signal that event has been handled
		return true;
	}

	// Signal to keep processing this event
	return false;
}

bool OPI_Gui::Panel::handleSection_Resize(int mouseX, int mouseY)
{
	if (!this->isVisible)
	{
		return false;
	}

	if (this->isResizing)
	{
		setWidth(mouseX - this->x);
		setHeight(mouseY - this->y);
		render();
		// Signal that event has been handled
		return true;
	}

	// Set resize cursor if inside lower-right corner
	if (resizableContainsMouse(mouseX, mouseY))
	{
		OPI_Gui::GuiManager::setCursor(CursorType::Resize);
		// Signal that event has been handled
		return true;
	}

	// Reset to normal cursor
	if (!this->isMoving)
		OPI_Gui::GuiManager::setCursor(CursorType::Normal);

	// Signal to keep processing this event
	return false;
}


bool OPI_Gui::Panel::handleSection_Close(int mouseX, int mouseY)
{
	if (!this->isVisible)
	{
		return false;
	}

	// Set resize cursor if inside lower-right corner
	if (closableContainsMouse(mouseX, mouseY))
	{
		OPI_Gui::GuiManager::setCursor(CursorType::Hand);
		// Signal that event has been handled
		return true;
	}

	// Signal to keep processing this event
	return false;
}

bool OPI_Gui::Panel::handleMouseMove(int mouseX, int mouseY)
{
	if (!this->isVisible)
	{
		return false;
	}

	// First check if mouse input is inside this Panel
	bool panelContainsMouse = containsMouse(mouseX, mouseY, this->x, this->y, this->width, this->height);

	//handle child mouse moves first
	for (auto i = this->children.rbegin(); i != this->children.rend(); ++i)
	{
		OPI_Gui::Element *element = *i;
		element->handleMouseMove(mouseX - this->x, mouseY - this->y);
	}

	if (this->isMovable && handleSection_Move(mouseX, mouseY))
	{
		return true;
	}

	if (this->isResizable && handleSection_Resize(mouseX, mouseY))
	{
		return true;
	}

	if (this->isClosable && handleSection_Close(mouseX, mouseY))
	{
		return true;
	}

	if (panelContainsMouse && this->mouseMotionCallback != nullptr)
	{
		this->mouseMotionCallback(mouseX - this->x, mouseY - this->y);
	}

	return panelContainsMouse;
}

bool OPI_Gui::Panel::handleMousePressLeft(int mouseX, int mouseY)
{
	if (!this->isVisible)
	{
		return false;
	}

	// First check if mouse input is inside this Panel
	bool panelContainsMouse = containsMouse(mouseX, mouseY, this->x, this->y, this->width, this->height);

	if (!panelContainsMouse) {
		return false;
	}

	//handle child mouse presses first
	for (auto i = this->children.rbegin(); i != this->children.rend(); ++i)
	{
		OPI_Gui::Element *element = *i;

		if (element->handleMousePressLeft(mouseX - this->x, mouseY - this->y))
			break;
	}

	if (this->isMovable && handleSection_Move(mouseX, mouseY))
	{
		isMoving = true;
		moveOriginX = this->x;
		moveOriginY = this->y;
		moveOriginGrabX = mouseX;
		moveOriginGrabY = mouseY;
		return true;
	}

	if (this->isResizable && handleSection_Resize(mouseX, mouseY))
	{
		isResizing = true;
		return true;
	}

	if (this->isClosable && handleSection_Close(mouseX, mouseY))
	{
		isVisible = false;
		OPI_Gui::GuiManager::setCursor(CursorType::Normal);
		return true;
	}

	if (this->mouseClickCallback != nullptr)
	{
		this->mouseClickCallback(mouseX - this->x, mouseY - this->y );
	}

	return panelContainsMouse;
}

bool OPI_Gui::Panel::handleMouseReleaseLeft(int mouseX, int mouseY)
{
	if (!this->isVisible)
	{
		return false;
	}

	// First check if mouse input is inside this Panel
	bool panelContainsMouse = containsMouse(mouseX, mouseY, this->x, this->y, this->width, this->height);

	//handle child mouse presses first
	for (auto i = this->children.rbegin(); i != this->children.rend(); ++i)
	{
		OPI_Gui::Element *element = *i;
		if (element->handleMouseReleaseLeft(mouseX - this->x, mouseY - this->y))
			break;
	}

	if (this->isMovable && handleSection_Move(mouseX, mouseY))
	{
		isMoving = false;
		return true;
	}

	if (this->isResizable && handleSection_Resize(mouseX, mouseY))
	{
		isResizing = false;
		return true;
	}

	return panelContainsMouse;
}


bool OPI_Gui::Panel::handleMousePressRight(int mouseX, int mouseY)
{
	if (!this->isVisible)
	{
		return false;
	}

	// First check if mouse input is inside this Panel
	bool panelContainsMouse = containsMouse(mouseX, mouseY, this->x, this->y, this->width, this->height);


	//handle child mouse presses first
	for (auto i = this->children.rbegin(); i != this->children.rend(); ++i)
	{
		OPI_Gui::Element *element = *i;
		if (element->handleMousePressRight(mouseX - this->x, mouseY - this->y))
			break;
	}

	return panelContainsMouse;
}

bool OPI_Gui::Panel::handleMouseReleaseRight(int mouseX, int mouseY)
{
	if (!this->isVisible)
	{
		return false;
	}

	// First check if mouse input is inside this Panel
	bool panelContainsMouse = containsMouse(mouseX, mouseY, this->x, this->y, this->width, this->height);

	//handle child mouse presses first
	for (auto i = this->children.rbegin(); i != this->children.rend(); ++i)
	{
		OPI_Gui::Element *element = *i;
		if (element->handleMouseReleaseRight(mouseX - this->x, mouseY - this->y))
			break;
	}
	return panelContainsMouse;
}

bool OPI_Gui::Panel::isInteracting()
{
	if (!this->isVisible)
	{
		return false;
	}

	return (this->isMoving || this->isResizing);
}
