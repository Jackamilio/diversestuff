#ifndef __JAMMER_H__
#define __JAMMER_H__

#include <list>
#include <map>
#include <string>
#include <glm/glm.hpp>

class Jammer {
public:
	struct Sprite {
		std::string image{};
		glm::vec4 color{ 1.0f,1.0f,1.0f,1.0f };
		glm::vec2 position{};
		float direction{};
		float scale{1.0f};
	};
	struct Scene : Sprite {
		std::list<Sprite> instances;
	};

	typedef std::map<std::string, std::string> ResourceMapper;
	ResourceMapper images;
	ResourceMapper sounds;

	typedef std::map<std::string, Sprite> SpriteList;
	SpriteList sprites;

	typedef std::map<std::string, Scene> SceneList;
	SceneList scenes;

	Scene liveScene;
};

#endif //__JAMMER_H__