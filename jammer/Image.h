#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "TextureManager.h"
#include "GuiElement.h"

class Rect;

class Image : public GuiElement {
private:
	const Texture* texture;

public:
	glm::ivec2 pos;
	Rect* rect;
	bool stretch;

	Image();
	Image(const Texture& tex);
	Image(const char* file);

	void ChangeTexture(const Texture& tex);
	void ChangeTexture(const char* file);
	void RemoveTexture();

	void Draw();
};

#endif