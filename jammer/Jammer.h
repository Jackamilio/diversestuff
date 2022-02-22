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
		float rotation{};
		float scale{};
	};
	struct Scene : Sprite {
		std::list<Sprite> instances;
	};

	typedef ResourceMapper std::map<std::string, std::string>;
	ResourceMapper images;
	ResourceMapper sounds;

	typedef SpriteList std::map<std::string, Sprite>;
	SpriteList sprites;

	typedef SceneList std::map<std::string, Scene>;
	SceneList scenes;

	Scene liveScene;
};

#endif //__JAMMER_H__