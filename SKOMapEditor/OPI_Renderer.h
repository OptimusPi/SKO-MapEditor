#ifndef	__OPI_RENDERER_H_
#define __OPI_RENDERER_H_

#include "OPI_Image.h"
#include "TextComponent.h"
#include <string>


class OPI_Renderer
{
public:
	OPI_Renderer(std::string title, int windowWidth, int windowHeight);
	virtual ~OPI_Renderer();
	
	// Drawing functions
	void drawImage(int x, int y, const OPI_Image *img);
	void drawImage(int x, int y, const OPI_Image* img, SDL_Rect selection);
	void drawImage(int x, int y, const OPI_Image *img, SDL_Rect selection, float blendTolerance);
	void drawText(OPI_Text::TextComponent *text);
	void drawRect(SDL_Rect rect, unsigned char r, unsigned char g, unsigned char b);
	void drawFilledRect(SDL_Rect rect, unsigned char r, unsigned char g, unsigned char b, unsigned char a);

	//Color mask to apply to textures (temporarily) and reset
	void setColorMask(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
	void resetColorMask();

	// Screen functions
	void initScreen();
	void sizeScreen();
	void resizeScreen();
	void movedScreen();
	void startDraw();
	void updateScreen();


	// Convertibng mouse from screen space to window render coordinates
	int getScaledMouseX(int mouse_x);
	int getScaledMouseY(int mouse_y);
	
	// SDL2 window
	SDL_Window *window = nullptr;

	// Window dimensions, helps with scaling
	int originalWindowWidth = 1920;
	int originalWindowHeight = 1080;

	int windowWidth = 1920;
	int windowHeight = 1080;

	int windowX = 0;
	int windowY = 0;
	// Window title
	std::string title = "";

	// OpenGL Context
	SDL_GLContext glContext = nullptr;
	GLuint bound = 0; 

private:
	float defaultBlendTolerance = 0.75f;
	float maskR = 1.0f;
	float maskG = 1.0f;
	float maskB = 1.0f;
	float maskA = 1.0f;

};

#endif

