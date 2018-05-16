#include <Fl/Fl_Shared_Image.H>
#include "TextureManager.h"

void FltkTexture::Load(const std::string & file)
{
	needsToLoad = false;

	img = Fl_Shared_Image::get(file.c_str());
	if (img) {
		glGenTextures(1, &value);
		glBindTexture(GL_TEXTURE_2D, value);

		GLint format;
		switch (img->d()) {
		case 3:
			format = GL_RGB;
			break;
		case 4:
			format = GL_RGBA;
			break;
		default:
			format = -1;
			break;
		}

		if (format != -1) {
			glTexImage2D(GL_TEXTURE_2D, 0, format, img->w(), img->h(), 0, format, GL_UNSIGNED_BYTE, img->data()[0]);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
	}
}
