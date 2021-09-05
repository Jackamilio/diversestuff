#ifndef ___MODEL_H___
#define ___MODEL_H___

#include <allegro5/allegro.h>
#include <allegro5/allegro_opengl.h>
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <string>
#include "MathUtils.h"
class LevelData;
class GraphicContext;
struct aiScene;

#define MAX_BONE_INFLUENCE 4
#define BUFFER_OFFSET(i) ((void*)(i))

class Model {
public:
	Model(const LevelData& level, GraphicContext& graphics);
	Model(const std::string& file, GraphicContext& graphics);
	~Model();

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 nor;
		glm::vec2 uv;
		int boneIds[MAX_BONE_INFLUENCE];
		float weights[MAX_BONE_INFLUENCE];
	};

	struct MinVertex {
		glm::vec3 pos;
		glm::vec3 nor;
		glm::vec2 uv;
	};

	struct Mesh {
		GLuint tex;
		GLuint verticesVBO;
		GLuint indicesVBO;
		std::vector<Vertex> verts;
		std::vector<MinVertex> minverts;
		std::vector<int> tris;
	};

	struct KeyFrame {
		KeyFrame() : time(0.0f), val() {}
		float time;
		union {
			glm::vec4 val;
			glm::vec3 loc;
			glm::quat rot;
			glm::vec3 scale;
		};
	};

	struct Channel {
		int boneId;
		std::vector<KeyFrame> positions;
		std::vector<KeyFrame> rotations;
		std::vector<KeyFrame> scalings;
	};

	struct Animation {
		std::string name;
		float duration;
		std::vector<Channel> channels;
	};

	struct Bone {
		std::string name;
		glm::mat4 offset;
		glm::mat4 local;
		std::vector<int> children;
	};

	struct Skeleton : Bone {
		std::vector<Bone> bones;
		std::vector<Animation> animations;
	};

	struct WorkingPose {
		const Skeleton* skeleton;
		std::vector<LocRotScale> transforms;
	};

	typedef std::vector<glm::mat4> FinalPose;

	void Draw(FinalPose* pose = 0) const;
	inline void Draw(FinalPose& pose) const { Draw(&pose); }
	void DrawSkeleton() const;
	void DrawPose(const WorkingPose& debugPose) const;

	bool GetPose(std::string animName, float t, WorkingPose& pose, bool loop = true) const;
	static void FinalizePose(const WorkingPose& in, FinalPose& out);
	static void MixPoses(WorkingPose& inout, const WorkingPose& to, float f);

private:
	GraphicContext& graphics;

	std::vector<Mesh> meshes;
	std::vector<Skeleton> skeletons;

	Bone& FindOrAddBone(const aiScene* scene, std::string boneName, int& boneId);
	Bone* FindBone(std::string boneName, int& boneId);
	Skeleton* FindSkeleton(std::string skelName);

	void GenerateVBOS();
};

#endif//___MODEL_H___