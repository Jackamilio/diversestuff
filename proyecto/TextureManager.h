#ifndef ___TEXTURE_MANAGER_H___
#define ___TEXTURE_MANAGER_H___

#include "ResourceManager.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_opengl.h>
#include <map>

class Texture {
private:
	ALLEGRO_BITMAP* bitmap;
	bool needsToLoad;
	
public:
	Texture() : bitmap(0), needsToLoad(true) {}
	~Texture() { if (bitmap) { al_destroy_bitmap(bitmap); bitmap = 0; } };
	inline GLuint GetValue() const { return al_get_opengl_texture(bitmap); }
	inline ALLEGRO_BITMAP* GetAlValue() const { return bitmap; }
	void Load(const std::string& file) { needsToLoad = false; bitmap = al_load_bitmap(file.c_str()); }
	inline bool NeedsToLoad() const { return needsToLoad; }
	inline float GetWidth() const { return al_get_bitmap_width(bitmap); }
	inline float GetHeight() const { return al_get_bitmap_height(bitmap); }
};

class TextureManager : public ResourceManager<Texture, GLuint> {
private:
	std::map<std::string,bool> processedKeys;
public:
	const std::string& ValidateKey(const std::string& key);
	inline const Texture& Get(const std::string& file) { return GetHandler(file); }
	inline void Bind(const std::string& file) { glBindTexture(GL_TEXTURE_2D, GetValue(file)); }
	inline float GetTexWidth(const std::string& file) { return Get(file).GetWidth(); }
	inline float GetTexHeight(const std::string& file) { return Get(file).GetHeight(); }
	inline GLuint GetGlId(const std::string& file) { return GetValue(file); }
};


#endif