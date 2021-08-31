#include "TextureManager.h"
#include <allegro5/allegro.h>

const std::string errortexfile("errortexture.png");

const std::string& TextureManager::ValidateKey(const std::string& key)
{
	auto mykey = processedKeys.find(key);
	if (mykey != processedKeys.end()) {
		return mykey->second ? mykey->first : errortexfile;
	}
	else {
		if (al_filename_exists(key.c_str())) {
			processedKeys[key] = true;
			return key;
		}
		else {
			assert(al_filename_exists(errortexfile.c_str()) && "An error texture must be present in the directory. Won't let you run around having crashes.");
			processedKeys[key] = false;
			return errortexfile;
		}
	}
}