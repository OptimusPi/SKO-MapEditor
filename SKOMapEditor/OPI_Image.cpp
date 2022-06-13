#include "OPI_Image.h"

OPI_Image::OPI_Image()
{
	width = 0;
	height = 0;
	texture = 0;
}

OPI_Image::~OPI_Image()
{
	if (texture)
		glDeleteTextures(1, &texture);
}

OPI_Image::OPI_Image(std::string filename)
{
	fileLocation = filename;
	setImage(filename);
}

OPI_Image::OPI_Image(OPI_Image *source)
{
	fileLocation = source->fileLocation;
	setImage(source);
}

OPI_Image::OPI_Image(SDL_Surface *surface)
{
	setImage(surface);
}

SDL_Surface* OPI_Image::copySurface(SDL_Surface *surface)
{
	Uint32 rmask, gmask, bmask, amask;
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0xff000000;
		gmask = 0x00ff0000;
		bmask = 0x0000ff00;
		amask = 0x000000ff;
	#else // little endian, like x86
		rmask = 0x000000ff;
		gmask = 0x0000ff00;
		bmask = 0x00ff0000;
		amask = 0xff000000;
	#endif
	return SDL_CreateRGBSurfaceFrom(surface->pixels, surface->w, surface->h, 32, surface->pitch, rmask, gmask, bmask, amask);
}

SDL_Surface* OPI_Image::copyOntoSurface(SDL_Surface* src, SDL_Surface* dst)
{
	SDL_BlitSurface(src, NULL, dst, NULL);
	return dst;
}

SDL_Surface* OPI_Image::clipSurface(SDL_Surface* surface, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
	Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else // little endian, like x86
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif
	SDL_Rect clipArea;
	clipArea.x = x;
	clipArea.y = y;
	clipArea.w = w;
	clipArea.h = h;
	
	// Copy the surface and then set the target clip area.
	auto copy = OPI_Image::copySurface(surface);
	SDL_SetClipRect(copy, &clipArea);
	
	// Create blank canvas, then blit the clip area onto it.
	auto canvas = OPI_Image::createBlankSurface(w, h);
	SDL_BlitSurface(copy, &clipArea, canvas, NULL);
	return canvas;
}

SDL_Surface* OPI_Image::tintSurface(SDL_Surface *surface, unsigned int r, unsigned int g, unsigned int b, unsigned int a)
{
	auto copy = OPI_Image::copySurface(surface);
	SDL_SetSurfaceColorMod(copy, r, g, b);
	SDL_SetSurfaceAlphaMod(copy, a);
	auto canvas = OPI_Image::createBlankSurface(surface->w, surface->h);
	SDL_BlitSurface(copy, NULL, canvas, NULL);
	return canvas;
}

int pot(unsigned int x)
{
	return ((x != 0) && !(x & (x - 1)));
}

void OPI_Image::setImage(SDL_Surface * surface)
{
	// TODO do I really want to do this? 
	// it could interfere with other OPI_Image that are running.
	//Clean up old texture memory from OpenGL
	if (texture)
	{
		glDeleteTextures(1, &texture);
	}

	this->width = surface->w;
	this->height = surface->h;

	if (!pot(width))
		printf("ERROR: OPI_Image width not power of two!\n");
	if (!pot(height))
		printf("ERROR: OPI_Image width not power of two!\n");

	texture = OPI_Image::generateTexture(surface);
	this->surface = surface;
}

void OPI_Image::setImage(std::string path)
{
	this->fileLocation = path;
	SDL_Surface *surface = OPI_Image::getSurface(path);
	this->surface = surface;
	setImage(surface);
}

void OPI_Image::setImage(OPI_Image *source)
{
	this->fileLocation = source->fileLocation;
	texture = source->texture;
	surface = source->surface;
	width = source->width;
	height = source->height;
}


SDL_Surface* OPI_Image::getSurface(std::string filePath)
{
	SDL_Surface *surface = IMG_Load(filePath.c_str());
	return surface;
}

GLuint OPI_Image::generateTexture(SDL_Surface * surface)
{
	GLuint tex = 0;			// This is a handle to our texture object
	GLenum texture_format = GL_RGBA;
	GLint  nOfColors = 0;

	if (surface)
	{
		// get the number of channels in the SDL surface
		nOfColors = surface->format->BytesPerPixel;
		if (nOfColors == 4)     // contains an alpha channel
		{

			if (surface->format->Rmask == 0x000000ff)
				texture_format = GL_RGBA;
			else
				texture_format = GL_BGRA;
		}
		else if (nOfColors == 3)     // no alpha channel
		{
			if (surface->format->Rmask == 0x000000ff)
				texture_format = GL_RGB;
			else
				texture_format = GL_BGR;
		}
		else {
			printf("WARNING: not truecolor OPI_Image! [from surface pointer]\n");
			texture_format = GL_RGBA;
		}


		// Have OpenGL generate a texture object handle for us
		glGenTextures(1, &tex);

		// Bind the texture object
		glBindTexture(GL_TEXTURE_2D, tex);


		// Set the texture's stretching properties
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Edit the texture object's OPI_Image data using the information SDL_Surface gives us
		glTexImage2D(GL_TEXTURE_2D, 0, nOfColors, surface->w, surface->h, 0,
			texture_format, GL_UNSIGNED_BYTE, surface->pixels);
	}

	return tex;
} 

 
SDL_Surface* OPI_Image::createColoredSurface(unsigned short width, unsigned short  height, unsigned char red, unsigned char green, unsigned char blue)
{
	/* Create a blank 32-bit SDL_Surface */
	SDL_Surface *s = OPI_Image::createBlankSurface(width, height);

	/* Filling the surface with red color. */
	SDL_FillRect(s, NULL, SDL_MapRGB(s->format, red, green, blue));

	return s;
}

SDL_Surface* OPI_Image::createColoredSurface(unsigned short width, unsigned short  height, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
	/* Create a blank 32-bit SDL_Surface */
	SDL_Surface* s = OPI_Image::createBlankSurface(width, height);

	/* Filling the surface with red color. */
	SDL_FillRect(s, NULL, SDL_MapRGBA(s->format, red, green, blue, alpha));

	return s;
}

SDL_Surface* OPI_Image::createBorderSurface(unsigned short width, unsigned short  height, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha, unsigned char thickness)
{
	SDL_Surface* surface = OPI_Image::createBlankSurface(width, height);

	auto borderColor = SDL_MapRGBA(surface->format, red, green, blue, alpha);

	// Apply very outer border
	SDL_Rect left{ 0 , 0, thickness,  height };
	SDL_Rect top{ 0 , 0, width, thickness };
	SDL_Rect right{ width - thickness, 0, thickness,  height };
	SDL_Rect bottom{ 0, height - thickness, width, thickness };

	SDL_FillRect(surface, &left, borderColor);
	SDL_FillRect(surface, &right, borderColor);
	SDL_FillRect(surface, &top, borderColor);
	SDL_FillRect(surface, &bottom, borderColor);

	return surface;
}

SDL_Surface* OPI_Image::createBlankSurface(unsigned short int width, unsigned short int height)
{
	// https://wiki.libsdl.org/SDL_CreateRGBSurfaceFrom
	// Set up the pixel format color masks for RGBA byte arrays.
	// Note: when blitting images, the destination SDL_Rect ignores width and height.
		Uint32 rmask, gmask, bmask, amask;
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0xff000000;
		gmask = 0x00ff0000;
		bmask = 0x0000ff00;
		amask = 0x000000ff;
	#else // little endian, like x86
		rmask = 0x000000ff;
		gmask = 0x0000ff00;
		bmask = 0x00ff0000;
		amask = 0xff000000;
	#endif

	return SDL_CreateRGBSurface(0, width, height,
		32,
		rmask, gmask, bmask, amask);
}