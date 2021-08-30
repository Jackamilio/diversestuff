#ifndef ___TEXTURE_MANAGER_H___
#define ___TEXTURE_MANAGER_H___

#include "ResourceManager.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_opengl.h>
#include <FL/Fl_Image.H>

class AllegroTexture {
private:
	ALLEGRO_BITMAP* bitmap;
	bool needsToLoad;
	
public:
	AllegroTexture() : bitmap(0), needsToLoad(true) {}
	~AllegroTexture() { if (bitmap) { al_destroy_bitmap(bitmap); bitmap = 0; } };
	inline GLuint GetValue() const { return al_get_opengl_texture(bitmap); }
	void Load(const std::string& file) { needsToLoad = false; bitmap = al_load_bitmap(file.c_str()); }
	inline bool NeedsToLoad() const { return needsToLoad; }
	inline float GetWidth() const { return al_get_bitmap_width(bitmap); }
	inline float GetHeight() const { return al_get_bitmap_height(bitmap); }
};

class FltkTexture {
private:
	Fl_Image* img;
	GLuint value;
	bool needsToLoad;

public:
	FltkTexture() : img(0), value(0), needsToLoad(true) {}
	~FltkTexture() { if (value != 0) { glDeleteTextures(1, &value); value = 0; } };
	inline GLuint GetValue() const { return value; }
	void Load(const std::string& file);
	inline bool NeedsToLoad() const { return needsToLoad; }
	inline float GetWidth() const { return img->w(); }
	inline float GetHeight() const { return img->h(); }
};

class TextureManagerBase {
public:
	virtual void Bind(const std::string& file) = 0;
	virtual float GetTexWidth(const std::string& file) = 0;
	virtual float GetTexHeight(const std::string& file) = 0;
	virtual GLuint GetGlId(const std::string& file) = 0;
};

template<class Texture>
class TextureManager : public TextureManagerBase, public ResourceManager<Texture, GLuint> {
public:
	inline const Texture& Get(const std::string& file) { return ResourceManager<Texture, GLuint>::GetHandler(file); }
	inline void Bind(const std::string& file) { glBindTexture(GL_TEXTURE_2D, ResourceManager<Texture, GLuint>::GetValue(file)); }
	inline float GetTexWidth(const std::string& file) { return Get(file).GetWidth(); }
	inline float GetTexHeight(const std::string& file) { return Get(file).GetHeight(); }
	inline GLuint GetGlId(const std::string& file) { return ResourceManager<Texture, GLuint>::GetValue(file); }
};


#endif