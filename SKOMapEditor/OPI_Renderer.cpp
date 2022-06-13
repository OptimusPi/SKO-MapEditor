#include "OPI_Renderer.h"


OPI_Renderer::OPI_Renderer(std::string title, int windowWidth, int windowHeight)
{
	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;
	this->title = title;
}

OPI_Renderer::~OPI_Renderer()
{
	SDL_GL_DeleteContext(glContext);
}

void OPI_Renderer::initScreen()
{
	window = SDL_CreateWindow(this->title.c_str(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		windowWidth, windowHeight,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	glContext = SDL_GL_CreateContext(window);

	SDL_GL_SetSwapInterval(1);

	sizeScreen();
}

void OPI_Renderer::sizeScreen()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	
	glViewport(0, 0, windowWidth, windowHeight);
	glClear(GL_COLOR_BUFFER_BIT);

	glOrtho(0.0f, windowWidth, windowHeight, 0.0f, -1.0f, 1.0f);


	movedScreen();

	// og scaling (not preserve aspect ratio)
	//glOrtho(0.0f, originalWindowWidth, originalWindowHeight, 0.0f, -1.0f, 1.0f);
}


void OPI_Renderer::movedScreen()
{
	int wind_x, wind_y;
	SDL_GetWindowPosition(window, &wind_x, &wind_y);
	this->windowX = wind_x;
	this->windowY = wind_y;
}

void OPI_Renderer::resizeScreen()
{
	glViewport(0, 0, windowWidth, windowHeight); // reset the viewport
	glMatrixMode(GL_PROJECTION); // modify the projection matrix
	glLoadIdentity();            // load an identity matrix into the projection matrix
	glOrtho(0, windowWidth, windowHeight, 0, -1.0, 1.0); // create new projection matrix

	/// Important!!! You need to switch back to the model-view matrix
	/// or else your OpenGL calls are modifying the projection matrix!
	glMatrixMode(GL_MODELVIEW); // return to the model matrix
	glLoadIdentity();           // load an identity matrix into the model-view matrix
}


void OPI_Renderer::drawImage(int x, int y, const OPI_Image *img)
{
		SDL_Rect selection;
		selection.x = 0;
		selection.y = 0;
		selection.w = img->width;
		selection.h = img->height;
		drawImage(x, y, img, selection, this->defaultBlendTolerance);
}

void OPI_Renderer::drawImage(int x, int y, const OPI_Image* img, const SDL_Rect selection)
{
	if (x >= (int)(0 - img->width) &&
		x < this->originalWindowWidth &&
		y < this->originalWindowHeight &&
		y >= (int)(0 - img->height))
	{
		drawImage(x, y, img, selection, this->defaultBlendTolerance);
	}
}

void OPI_Renderer::drawImage(int x, int y, const OPI_Image *img, const SDL_Rect selection, float blendTolerance)
{

	// Do not attempt to render null images, just continue.
	if (img == nullptr)
		return;

	glEnable(GL_TEXTURE_2D);
	glColor4f(maskR, maskG, maskB, maskA);

	if (this->bound != img->texture)
	{
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glBindTexture(GL_TEXTURE_2D, img->texture);
		this->bound = img->texture;

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	
	float offset_x = selection.x;
	float offset_y = selection.y;
	float end_x = x + selection.w;
	float end_y = y + selection.h;

	float right = (offset_x + selection.w) / (float)img->width;
	float bottom = (offset_y + selection.h) / (float)img->height;
	float left = (offset_x) / (float)img->width;
	float top = (offset_y) / (float)img->height;

	glBegin(GL_QUADS);
	//Top-left vertex (corner)
	glTexCoord2f(left, top);
	glVertex2f(x, y);

	//Bottom-left vertex (corner)
	glTexCoord2f(left, bottom);
	glVertex2f(x, end_y);

	//Bottom-right vertex (corner)
	glTexCoord2f(right, bottom);
	glVertex2f(end_x, end_y);

	//Top-right vertex (corner)
	glTexCoord2f(right, top);
	glVertex2f(end_x, y);
	glEnd();
}

void OPI_Renderer::drawText(OPI_Text::TextComponent *text)
{
	//tint
	glColor3f(text->R, text->G, text->B);

	float x = text->x;
	float y = text->y;
	SDL_Rect selection;
	selection.w = text->contentRender->width;
	selection.h = text->contentRender->height;
	selection.x = 0;
	selection.y = 0;

	drawImage(x, y, text->contentRender, selection, 0.01); // TODO test if this is still needed

	//reset tint
	glColor3f(1.0f, 1.0f, 1.0f);
}

void OPI_Renderer::drawRect(SDL_Rect rect, unsigned char r, unsigned char g, unsigned char b)
{
	float fR = r == 0? 0.0f : r / 255.0;
	float fG = g == 0? 0.0f : g / 255.0;
	float fB = b == 0? 0.0f : b / 255.0;

	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_TEXTURE_2D);
	glLoadIdentity();

	//Draw rect
	glBegin(GL_LINE_LOOP);
	glColor4f(fR, fG, fB, 0.7f);
	glVertex2f(rect.x, rect.y);
	glVertex2f(rect.x + rect.w, rect.y);
	glVertex2f(rect.x + rect.w, rect.y + rect.h);
	glVertex2f(rect.x, rect.y + rect.h);
	glEnd();

	//Draw outside border
	glBegin(GL_LINE_LOOP);
	glColor4f(fR/2, fG/2, fB/2, 0.35f);
	glVertex2f(rect.x - 1, rect.y - 1);
	glVertex2f(rect.x + rect.w + 1, rect.y - 1);
	glVertex2f(rect.x + rect.w + 1, rect.y + rect.h + 1);
	glVertex2f(rect.x - 1, rect.y + rect.h + 1);
	glEnd();

	//Draw inside border
	glBegin(GL_LINE_LOOP);
	glColor3f(0, 0, 0);
	glColor4f(fR, fG, fB, 0.4f);
	glVertex2f(rect.x + 1, rect.y+ 1);
	glVertex2f(rect.x + rect.w - 1, rect.y + 1);
	glVertex2f(rect.x + rect.w - 1, rect.y + rect.h - 1);
	glVertex2f(rect.x + 1, rect.y + rect.h - 1);
	glEnd();

	//reset color mask
	resetColorMask();
}

void OPI_Renderer::setColorMask(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	maskR = r == 0 ? 0.0f : (r / 255.0);
	maskG = g == 0 ? 0.0f : (g / 255.0);
	maskB = b == 0 ? 0.0f : (b / 255.0);
	maskA = a == 0 ? 0.0f : (a / 255.0);
}

void OPI_Renderer::resetColorMask()
{
	maskR = 1.0f;
	maskG = 1.0f;
	maskB = 1.0f;
	maskA = 1.0f;
}

void OPI_Renderer::drawFilledRect(SDL_Rect rect, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{

	drawRect(rect, r, g, b);

	float fR = r == 0 ? 0.0f : (r / 255.0 + 1) / 3;
	float fG = g == 0 ? 0.0f : (g / 255.0 + 1) / 3;
	float fB = b == 0 ? 0.0f : (b / 255.0 + 1) / 3;
	float fA = a == 0 ? 0.0f : (a / 255.0);

	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_TEXTURE_2D);
	glLoadIdentity();

	glBegin(GL_POLYGON);
	glColor4f(fR, fG, fB, fA);
	glVertex2f(rect.x, rect.y);
	glVertex2f(rect.x + rect.w - 1, rect.y + 1);
	glVertex2f(rect.x + rect.w - 1, rect.y + rect.h - 1);
	glVertex2f(rect.x + 1, rect.y + rect.h - 1);
	glEnd();

	resetColorMask();
}

void OPI_Renderer::startDraw()
{
	glClearColor(0.00f, 0.00f, 0.00f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void OPI_Renderer::updateScreen()
{
	SDL_GL_SwapWindow(this->window);
}

int OPI_Renderer::getScaledMouseX(int mouse_x)
{
	return mouse_x * (float)this->originalWindowWidth / this->windowWidth;
}

int OPI_Renderer::getScaledMouseY(int mouse_y)
{
	return mouse_y * (float)this->originalWindowHeight / this->windowHeight;
}
