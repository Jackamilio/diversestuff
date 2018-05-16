#include "Model.h"
#include "GraphicContext.h"
#include "LevelData.h"
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

void Transform(
	const LevelData::Vertex* vert,
	const LevelData::Coordinate& c,
	const glm::mat4& mat,
	const glm::vec3 nor,
	float uo, float uf, float vo, float vf,
	Model::MinVertex& out) {
	glm::vec4 f = mat * glm::vec4(vert->xyz, 1.0f);
	glm::vec4 n = mat * glm::vec4(nor, 0.0f);
	
	out.pos = c.asVec3() + f.xyz();
	out.nor = n.xyz();
	out.uv.x = uo + vert->u * uf;
	out.uv.y = vo + vert->v * vf;
}

Model::Model(const LevelData& level, GraphicContext& g) : graphics(g)
{
	std::map<const LevelData::TilesetData*, unsigned int> meshmap;

	glm::mat4 mat;

	for (LevelData::const_iterator it = level.begin(); it != level.end(); ++it) {
		const LevelData::Coordinate& c = it->first;

		for (unsigned int b = 0; b < it->second.size(); ++b) {
			const LevelData::Brick& brick = it->second[b];

			brick.matrix.CalcMatrix(mat);

			if (brick.brickdata) {
				const std::vector<int>& tris = brick.brickdata->GetTriangleList();
				int nbTris = tris.size() / 3;
				for (int i = 0; i < nbTris; ++i) {
					const LevelData::Vertex* v1 = brick.brickdata->GetVertex(tris[i * 3]);
					const LevelData::Vertex* v2 = brick.brickdata->GetVertex(tris[i * 3 + 1]);
					const LevelData::Vertex* v3 = brick.brickdata->GetVertex(tris[i * 3 + 2]);
					if (v1 && v2 && v3) {
						float uo = 0.0f;
						float uf = 1.0f;
						float vo = 0.0f;
						float vf = 1.0f;
						const LevelData::TilesetData* tsdt = brick.tilesetdata;
						if (tsdt) {
							float tw = (float)graphics.textures.GetTexWidth(tsdt->file);
							float th = (float)graphics.textures.GetTexHeight(tsdt->file);
							uo = (float)(tsdt->ox + brick.tilex * (tsdt->tw + tsdt->px)) / tw;
							vo = (float)(tsdt->oy + brick.tiley * (tsdt->th + tsdt->py)) / th;
							uf = (float)tsdt->tw / tw;
							vf = (float)tsdt->th / th;
							//if (flipTexV)
							{
								vo = 1.0f - vo;
								vf = -vf;
							}
						}

						unsigned int& meshid = meshmap[tsdt];
						if (meshid == 0) {
							meshid = meshes.size() + 1;
							meshes.resize(meshes.size() + 1);
							meshes[meshid - 1].tex = tsdt ? graphics.textures.GetGlId(tsdt->file) : 0;
						}
						Mesh& mesh = meshes[meshid - 1]; // id shifted to have 0 as invalid value

						glm::vec3 v1tov2 = v2->xyz - v1->xyz;
						glm::vec3 v1tov3 = v3->xyz - v1->xyz;
						glm::vec3 normal = glm::normalize(glm::cross(v1tov2, v1tov3));

						unsigned int pid1 = mesh.minverts.size();
						unsigned int pid2 = pid1 + 1;
						unsigned int pid3 = pid2 + 1;
						mesh.tris.push_back(pid1);
						mesh.tris.push_back(pid2);
						mesh.tris.push_back(pid3);

						mesh.minverts.resize(pid1 + 3);
						Transform(v1, c, mat, normal, uo, uf, vo, vf, mesh.minverts[pid1]);
						Transform(v2, c, mat, normal, uo, uf, vo, vf, mesh.minverts[pid2]);
						Transform(v3, c, mat, normal, uo, uf, vo, vf, mesh.minverts[pid3]);
					}
				}
			}
		}
	}

	GenerateVBOS();
}

const aiNode* FindNodeInSceneRecursive(const aiScene* scene, const aiNode* node, std::string nodeName) {

	std::string myName(node->mName.C_Str());
	if (myName == nodeName) {
		return node;
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		const aiNode* res = FindNodeInSceneRecursive(scene, node->mChildren[i], nodeName);
		if (res) {
			return res;
		}
	}

	return 0;
}

void FillBoneChildren(const aiNode* node, Model::Skeleton& skeleton, std::vector<unsigned int>& children) {
	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		std::string childName(node->mChildren[i]->mName.C_Str());

		int id = -1;
		for (unsigned int j = 0; j < skeleton.bones.size(); ++j) {
			if (skeleton.bones[j].name == childName) {
				id = j;
				break;
			}
		}
		if (id >= 0) {
			children.push_back(id);
		}
	}
}

Model::Model(const std::string& file, GraphicContext& g) : graphics(g)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(file, aiProcessPreset_TargetRealtime_MaxQuality);

	if (!scene) {
		fprintf(stdout, "%s\n", importer.GetErrorString());
	}
	else {
		if (scene->HasMeshes()) {
			for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
				meshes.resize(meshes.size() + 1);
				const aiMesh* omesh = scene->mMeshes[i];

				// "texture" material
				aiString texPath;
				if (AI_SUCCESS == scene->mMaterials[omesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &texPath))
				{
					meshes[i].tex = graphics.textures.GetGlId(texPath.C_Str());
				}
				else {
					meshes[i].tex = 0;
				}

				// fill in vertices
				std::vector<Vertex>& verts = meshes[i].verts;
				verts.resize(omesh->mNumVertices);
				for (unsigned int j = 0; j < omesh->mNumVertices; ++j) {
					verts[j].pos.x = omesh->mVertices[j].x;
					verts[j].pos.y = omesh->mVertices[j].y;
					verts[j].pos.z = omesh->mVertices[j].z;

					if (omesh->HasNormals()) {
						verts[j].nor.x = omesh->mNormals[j].x;
						verts[j].nor.y = omesh->mNormals[j].y;
						verts[j].nor.z = omesh->mNormals[j].z;
					}

					if (omesh->HasTextureCoords(0)) {
						verts[j].uv.x = omesh->mTextureCoords[0][j].x;
						verts[j].uv.y = omesh->mTextureCoords[0][j].y;
					}

					for (unsigned int k = 0; k < MAX_BONE_INFLUENCE; ++k) {
						verts[j].boneIds[k] = 0;
						verts[j].weights[k] = 0.0f;
					}
				}

				// fill in triangles
				std::vector<unsigned int>& tris = meshes[i].tris;
				tris.reserve(omesh->mNumFaces*3);
				int notTriangles = 0;
				for (unsigned int j = 0; j < omesh->mNumFaces; ++j) {
					const aiFace& face = omesh->mFaces[j];
					if (face.mNumIndices == 3) {
						tris.push_back(face.mIndices[0]);
						tris.push_back(face.mIndices[1]);
						tris.push_back(face.mIndices[2]);
					}
					else {
						++notTriangles;
					}
				}

				if (notTriangles > 0) {
					fprintf(stdout, "Warning, found %i faces which are not triangles in one of the meshes of %s\n", notTriangles, file.c_str());
				}

				// fill in skeletons and skin
				if (omesh->HasBones()) {
					bool foundTooManyBones = false;
					for (unsigned int j = 0; j < omesh->mNumBones; ++j) {
						const aiBone* obone = omesh->mBones[j];
						std::string boneName(obone->mName.C_Str());
						unsigned int boneId;
						Bone& bone = FindOrAddBone(scene, boneName, boneId);

						memcpy(&bone.offset[0].x, &obone->mOffsetMatrix.a1, sizeof(float) * 16);
						bone.offset = glm::transpose(bone.offset);

						for (unsigned int k = 0; k < obone->mNumWeights; ++k) {
							//std::vector<BoneInfluence>& inf = verts[obone->mWeights[k].mVertexId].inf;
							//unsigned int infId = inf.size();
							//inf.resize(infId + 1);
							int* boneIds = verts[obone->mWeights[k].mVertexId].boneIds;
							float* weights = verts[obone->mWeights[k].mVertexId].weights;
							unsigned int infId = 0;
							while (weights[infId] != 0.0f && infId < MAX_BONE_INFLUENCE) {
								++infId;
							}
							if (infId < MAX_BONE_INFLUENCE) {
								boneIds[infId] = boneId;
								weights[infId] = obone->mWeights[k].mWeight;
							}
							else {
								foundTooManyBones = true;
							}
						}
					}
					if (foundTooManyBones) {
						fprintf(stdout, "Warning, the mesh %i from %s contains vertices that references too many bones (maximum is %i)\n", i, file.c_str(), MAX_BONE_INFLUENCE);
					}
					//now normalize weights troughout the vertices
					for (unsigned int j = 0; j < verts.size(); ++j) {
						//std::vector<BoneInfluence>& inf = verts[j].inf;
						//unsigned int nbInf = inf.size();
						int* boneIds = verts[j].boneIds;
						float* weights = verts[j].weights;

						unsigned int nbInf = 0;
						float totalWeight = 0.0f;

						while (weights[nbInf] != 0.0f && nbInf < MAX_BONE_INFLUENCE) {
							totalWeight += weights[nbInf];
							++nbInf;
						}
						for (unsigned int k = 0; k < nbInf; ++k) {
							weights[k] /= totalWeight;
						}
					}
				}
			}

			//fill bone children
			for (unsigned int i = 0; i < skeletons.size(); ++i) {
				Skeleton& skel = skeletons[i];
				const aiNode* node = FindNodeInSceneRecursive(scene, scene->mRootNode, skel.name);
				FillBoneChildren(node, skel, skel.children);
				for (unsigned int j = 0; j < skel.bones.size(); ++j) {
					node = FindNodeInSceneRecursive(scene, scene->mRootNode, skel.bones[j].name);
					FillBoneChildren(node, skel, skel.bones[j].children);
				}
			}

			// animations
			if (scene->HasAnimations()) {
				for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {
					std::string animName(scene->mAnimations[i]->mName.C_Str());
					unsigned int t = animName.find('|');
					std::string skelName = animName.substr(0, t);
					animName = animName.substr(t + 1, animName.size() - t - 1);

					Skeleton* skel = FindSkeleton(skelName);
					if (skel) {
						skel->animations.resize(skel->animations.size() + 1);
						Animation& anim = skel->animations[skel->animations.size() - 1];
						anim.name = animName;
						float tps = scene->mAnimations[i]->mTicksPerSecond;
						if (tps == 0.0f) {
							tps = 1.0f;
						}
						anim.duration = scene->mAnimations[i]->mDuration / tps;
						// copy channels
						for (unsigned int j = 0; j < scene->mAnimations[i]->mNumChannels; ++j) {
							const aiNodeAnim* ochannel = scene->mAnimations[i]->mChannels[j];
							std::string channelName(ochannel->mNodeName.C_Str());
							unsigned int boneId;
							if (FindBone(channelName, boneId)) {
								anim.channels.resize(anim.channels.size() + 1);
								Channel& channel = anim.channels[anim.channels.size() - 1];
								channel.boneId = boneId;
								channel.positions.resize(ochannel->mNumPositionKeys);
								channel.rotations.resize(ochannel->mNumRotationKeys);
								channel.scalings.resize(ochannel->mNumScalingKeys);
								for (unsigned int k = 0; k < ochannel->mNumPositionKeys; ++k) {
									channel.positions[k].time = ochannel->mPositionKeys[k].mTime / tps;
									channel.positions[k].loc.x = ochannel->mPositionKeys[k].mValue.x;
									channel.positions[k].loc.y = ochannel->mPositionKeys[k].mValue.y;
									channel.positions[k].loc.z = ochannel->mPositionKeys[k].mValue.z;
								}
								for (unsigned int k = 0; k < ochannel->mNumRotationKeys; ++k) {
									channel.rotations[k].time = ochannel->mRotationKeys[k].mTime / tps;
									channel.rotations[k].rot.x = ochannel->mRotationKeys[k].mValue.x;
									channel.rotations[k].rot.y = ochannel->mRotationKeys[k].mValue.y;
									channel.rotations[k].rot.z = ochannel->mRotationKeys[k].mValue.z;
									channel.rotations[k].rot.w = ochannel->mRotationKeys[k].mValue.w;
								}
								for (unsigned int k = 0; k < ochannel->mNumScalingKeys; ++k) {
									channel.scalings[k].time = ochannel->mScalingKeys[k].mTime / tps;
									channel.scalings[k].scale.x = ochannel->mScalingKeys[k].mValue.x;
									channel.scalings[k].scale.y = ochannel->mScalingKeys[k].mValue.y;
									channel.scalings[k].scale.z = ochannel->mScalingKeys[k].mValue.z;
								}
							}
						}
					}
				}
			}

			GenerateVBOS();
		}
		else {
			fprintf(stdout, "Warning loading %s, no meshes found!\n", file.c_str());
		}
	}
}

Model::Bone& Model::FindOrAddBone(const aiScene* scene, std::string boneName, unsigned int& boneId) {
	// first look for us
	Bone* b = FindBone(boneName, boneId);
	if (b) { return *b; }

	// then look for the bone in the scene hierarchy
	const aiNode* boneNode = FindNodeInSceneRecursive(scene, scene->mRootNode, boneName);

	// find its skeleton
	const aiNode* skelNode = boneNode;
	const aiNode* parent = boneNode->mParent;
	while (parent != scene->mRootNode) {
		skelNode = parent;
		parent = parent->mParent;
	}

	// check if we already made this skeleton
	std::string skelName(skelNode->mName.C_Str());
	Skeleton* skeleton = 0;
	for (unsigned int i = 0; i < skeletons.size(); ++i) {
		if (skeletons[i].name == skelName) {
			skeleton = &skeletons[i];
			break;
		}
	}

	// create it if not
	if (skeleton == 0) {
		skeletons.resize(skeletons.size() + 1);
		skeleton = &skeletons[skeletons.size() - 1];
		skeleton->name = skelName;
		memcpy(&skeleton->local[0].x, &skelNode->mTransformation.a1, sizeof(float) * 16);
		skeleton->local = glm::transpose(skeleton->local);
	}

	// create the bone
	boneId = skeleton->bones.size();
	skeleton->bones.resize(boneId + 1);
	Bone& bone = skeleton->bones[boneId];
	bone.name = boneName;
	memcpy(&bone.local[0].x, &boneNode->mTransformation.a1, sizeof(float) * 16);
	bone.local = glm::transpose(bone.local);
	return bone;
}

Model::Bone* Model::FindBone(std::string boneName, unsigned int& boneId) {
	for (unsigned int i = 0; i < skeletons.size(); ++i) {
		for (unsigned int j = 0; j < skeletons[i].bones.size(); ++j) {
			if (skeletons[i].bones[j].name == boneName) {
				boneId = j;
				return &skeletons[i].bones[j];
			}
		}
	}
	return 0;
}

Model::Skeleton * Model::FindSkeleton(std::string skelName)
{
	for (unsigned int i = 0; i < skeletons.size(); ++i) {
		if (skeletons[i].name == skelName) {
			return &skeletons[i];
		}
	}
	return 0;
}

void Model::GenerateVBOS()
{
	for (unsigned int i = 0; i < meshes.size(); ++i) {
		glGenBuffers(1, &meshes[i].verticesVBO);
		glBindBuffer(GL_ARRAY_BUFFER, meshes[i].verticesVBO);

		if (!meshes[i].verts.empty()) {
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*meshes[i].verts.size(), &meshes[i].verts[0], GL_STATIC_DRAW);
		}
		else if (!meshes[i].minverts.empty()) {
			glBufferData(GL_ARRAY_BUFFER, sizeof(MinVertex)*meshes[i].minverts.size(), &meshes[i].minverts[0], GL_STATIC_DRAW);
		}

		glGenBuffers(1, &meshes[i].indicesVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshes[i].indicesVBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*meshes[i].tris.size(), &meshes[i].tris[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

Model::~Model()
{
	for (unsigned int i = 0; i < meshes.size(); ++i) {
		if (meshes[i].verticesVBO) {
			glDeleteBuffers(1, &meshes[i].verticesVBO);
			meshes[i].verticesVBO = 0;
		}
		if (meshes[i].indicesVBO) {
			glDeleteBuffers(1, &meshes[i].indicesVBO);
			meshes[i].indicesVBO = 0;
		}
	}
}

void Color4f(const aiColor4D *color)
{
	glColor4f(color->r, color->g, color->b, color->a);
}

void Model::Draw(FinalPose* pose) const
{
	Program* program = graphics.programs.GetCurrent();
	if (program) {
		for (unsigned int i = 0; i < meshes.size(); ++i) {
			glBindTexture(GL_TEXTURE_2D, meshes[i].tex);

			glBindBuffer(GL_ARRAY_BUFFER, meshes[i].verticesVBO);
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			int hasBones = meshes[i].verts.empty() ? 0 : 1;
			program->SetUniform("hasBones", hasBones);
			if (hasBones) {
				glEnableVertexAttribArray(3);
				glEnableVertexAttribArray(4);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, nor));
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
				glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, boneIds));
				glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));
				if (pose) { program->SetUniform("bones", *pose); }
			}
			else
			{
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MinVertex), (void*)offsetof(MinVertex, pos));
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MinVertex), (void*)offsetof(MinVertex, nor));
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MinVertex), (void*)offsetof(MinVertex, uv));
			}

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshes[i].indicesVBO);

			

			glDrawElements(GL_TRIANGLES, meshes[i].tris.size(), GL_UNSIGNED_INT, BUFFER_OFFSET(0));

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);
			glDisableVertexAttribArray(3);
			glDisableVertexAttribArray(4);
		}
	}

	/*for (unsigned int i = 0; i < meshes.size(); ++i) {
		const Vertex* verts = &meshes[i].verts[0];
		const unsigned int* tris = &meshes[i].tris[0];
		unsigned int numTris = meshes[i].tris.size();
		unsigned int numVerts = meshes[i].verts.size();

		if (meshes[i].skin.size() != numVerts) {
			meshes[i].skin.resize(numVerts);
		}

		MinVertex* skin = &meshes[i].skin[0];
		glm::mat4* transforms = &finalizedPose.transforms[0];

		glm::vec4 pos, nor;

		// skinning
		for (unsigned int j = 0; j < numVerts; ++j) {
			//unsigned int nbInf = verts[j].inf.size();
			unsigned int nbInf = 0;
			while (verts[j].weights[nbInf] != 0.0f && nbInf < MAX_BONE_INFLUENCE) {
				++nbInf;
			}
			if (nbInf > 0) {
				//const BoneInfluence* inf = &verts[j].inf[0];
				const int* boneIds = verts[j].boneIds;
				const float* weights = verts[j].weights;
				skin[j].pos.x = 0.0f;
				skin[j].pos.y = 0.0f;
				skin[j].pos.z = 0.0f;
				skin[j].nor.x = 0.0f;
				skin[j].nor.y = 0.0f;
				skin[j].nor.z = 0.0f;

				for (unsigned int k = 0; k < nbInf; ++k) {
					pos.x = verts[j].pos.x;
					pos.y = verts[j].pos.y;
					pos.z = verts[j].pos.z;
					pos.w = 1.0f;
					nor.x = verts[j].nor.x;
					nor.y = verts[j].nor.y;
					nor.z = verts[j].nor.z;
					nor.w = 0.0f;

					pos = (transforms[boneIds[k]] * pos) * weights[k];
					nor = (transforms[boneIds[k]] * nor) * weights[k];

					skin[j].pos.x += pos.x;
					skin[j].pos.y += pos.y;
					skin[j].pos.z += pos.z;
					skin[j].nor.x += nor.x;
					skin[j].nor.y += nor.y;
					skin[j].nor.z += nor.z;
				}
			}
			else {
				skin[j].pos = verts[j].pos;
				skin[j].nor = verts[j].nor;
			}
		}

		// draw
		glBindTexture(GL_TEXTURE_2D, meshes[i].tex);

		glBegin(GL_TRIANGLES);
		for (unsigned int j = 0; j < numTris; ++j) {
			glTexCoord2fv(&verts[tris[j]].uv.x);
			glNormal3fv(&skin[tris[j]].nor.x);
			glVertex3fv(&skin[tris[j]].pos.x);
		}
		glEnd();
	}*/
}

void RecursiveDrawBones(const std::vector<Model::Bone>& bones, const std::vector<unsigned int>& boneIds, const glm::mat4& mat) {
	glm::vec4 pos1(0, 0, 0, 1);
	pos1 = mat * pos1;

	for (unsigned int i = 0; i < boneIds.size(); ++i) {
		const Model::Bone& bone = bones[boneIds[i]];
		
		glm::mat4 mat2 = mat * bone.local;
		glm::vec4 pos2(0, 0, 0, 1);
		pos2 = mat2 * pos2;

		glVertex3f(pos1.x, pos1.y, pos1.z);
		glVertex3f(pos2.x, pos2.y, pos2.z);

		RecursiveDrawBones(bones, bone.children, mat2);
	}
}

void Model::DrawSkeleton() const
{
	const Skeleton& skeleton = skeletons[0];
	glm::mat4 mat;

	glBegin(GL_LINES);

	RecursiveDrawBones(skeleton.bones, skeleton.children, mat);

	glEnd();
}

void RecursiveDrawBonePoses(const Model::WorkingPose& pose, const std::vector<unsigned int>& boneIds, const glm::mat4& mat) {
	glm::vec4 pos1(0, 0, 0, 1);
	pos1 = mat * pos1;

	if (boneIds.empty()) {
		glm::vec4 pos2(0, 0.2f, 0, 1);
		pos2 = mat * pos2;

		glColor3ub(255, 0, 0);
		glVertex3f(pos1.x, pos1.y, pos1.z);
		glVertex3f(pos2.x, pos2.y, pos2.z);
	}

	glColor3ub(255, 255, 255);

	for (unsigned int i = 0; i < boneIds.size(); ++i) {
		const glm::mat4& mat2 = mat * LocRotScaleToMat4(pose.transforms[boneIds[i]]);
		glm::vec4 pos2(0, 0, 0, 1);
		pos2 = mat2 * pos2;

		glVertex3f(pos1.x, pos1.y, pos1.z);
		glVertex3f(pos2.x, pos2.y, pos2.z);

		RecursiveDrawBonePoses(pose, pose.skeleton->bones[boneIds[i]].children, mat2);
	}
}

void Model::DrawPose(const WorkingPose& pose) const
{
	glm::mat4 mat(1.0f);

	glBegin(GL_LINES);

	RecursiveDrawBonePoses(pose, pose.skeleton->children, mat);

	glEnd();
}

void GetLerp(const std::vector<Model::KeyFrame>& values, float t, glm::vec4& out, bool slerp) {
	if (!values.empty()) {
		if (t <= values[0].time) {
			out = values[0].val;
		}
		else if (t >= values[values.size() - 1].time) {
			out = values[values.size() - 1].val;
		}
		else {
			for (unsigned int j = 1; j < values.size(); ++j) {
				if (t <= values[j].time) {
					const Model::KeyFrame& before = values[j - 1];
					const Model::KeyFrame& after = values[j];
					float dt = after.time - before.time;
					float tt = (t - before.time) / dt;
					if (slerp) {
						//glm::quat b(before.val.w, before.val.x, before.val.y, before.val.z);
						//glm::quat a(after.val.w, after.val.x, after.val.y, after.val.z);
						//glm::quat o = glm::slerp(b, a, tt);
						glm::quat o = glm::slerp(before.rot, after.rot, tt);
						out.x = o.x;
						out.y = o.y;
						out.z = o.z;
						out.w = o.w;
					}
					else {
						out = before.val + (after.val - before.val) * tt;
					}
					break;
				}
			}
		}
	}
}

void GetLerp(const std::vector<Model::KeyFrame>& values, float t, glm::vec3& out) {
	glm::vec4 v;
	GetLerp(values, t, v, false);
	out = v.xyz();
}

void GetLerp(const std::vector<Model::KeyFrame>& values, float t, glm::quat& out) {
	glm::vec4 v;
	GetLerp(values, t, v, true);
	out.x = v.x;
	out.y = v.y;
	out.z = v.z;
	out.w = v.w;
}

void RecursiveGetGlobalPose(Model::FinalPose& fpose, const Model::WorkingPose& wpose, const Model::Skeleton& skeleton, const std::vector<unsigned int>& boneIds, const glm::mat4& mat) {
	
	for (unsigned int i = 0; i < boneIds.size(); ++i) {
		glm::mat4 tr = LocRotScaleToMat4(wpose.transforms[boneIds[i]]);
		tr = mat * tr;

		fpose[boneIds[i]] = tr;

		RecursiveGetGlobalPose(fpose, wpose, skeleton, skeleton.bones[boneIds[i]].children, tr);
	}
}

bool Model::GetPose(std::string animName, float t, Model::WorkingPose& pose, bool loop) const
{
	// first find anim
	const Animation* anim = 0;
	for (unsigned int i = 0; i < skeletons.size(); ++i) {
		for (unsigned int j = 0; j < skeletons[i].animations.size(); ++j) {
			if (skeletons[i].animations[j].name == animName) {
				anim = &skeletons[i].animations[j];
				pose.skeleton = &skeletons[i];
				break;
			}
		}
		if (anim) { break; }
	}

	if (anim) {
		if (loop) {
			t = fmodf(t, anim->duration);
		}

		if (pose.transforms.size() != pose.skeleton->bones.size()) {
			pose.transforms.resize(pose.skeleton->bones.size());
		}

		// local transforms first
		for (unsigned int i = 0; i < anim->channels.size(); ++i) {
			LocRotScale& tr = pose.transforms[anim->channels[i].boneId];

			GetLerp(anim->channels[i].positions, t, tr.loc);
			GetLerp(anim->channels[i].rotations, t, tr.rot);
			GetLerp(anim->channels[i].scalings, t, tr.scale);

			//glm::mat4& tr = pose.transforms[anim->channels[i].boneId];

			//glm::vec4 lpos, lrot, lscl;

			//GetLerp(anim->channels[i].positions, t, lpos, false);
			//GetLerp(anim->channels[i].rotations, t, lrot, true);
			//GetLerp(anim->channels[i].scalings, t, lscl, false);
			
			//tr = glm::mat4();
			//tr = glm::translate(tr, lpos.xyz());
			
			//glm::mat4 rot = glm::mat4_cast(glm::quat(lrot.w, lrot.x, lrot.y, lrot.z));
			//tr = tr * rot;

			//tr = glm::scale(tr, lscl.xyz());
		}


		return true;
	}

	return false;
}

void Model::FinalizePose(const WorkingPose& in, FinalPose& out)
{
	if (out.size() != in.transforms.size()) {
		out.resize(in.transforms.size());
	}
	RecursiveGetGlobalPose(out, in, *in.skeleton, in.skeleton->children, glm::mat4(1.0f));
	for (unsigned int i = 0; i < in.skeleton->bones.size(); ++i) {
		out[i] = out[i] * in.skeleton->bones[i].offset;
	}
}

void Model::MixPoses(WorkingPose & inout, const WorkingPose & target, float f)
{
	if (inout.skeleton && target.skeleton && inout.skeleton->bones.size() == target.skeleton->bones.size()) {
		for (unsigned int i = 0; i < target.transforms.size(); ++i) {
			LocRotScale& from = inout.transforms[i];
			const LocRotScale& to = target.transforms[i];
			from.loc += (to.loc - from.loc) * f;
			from.rot = glm::slerp(from.rot, to.rot, f);
			from.scale += (to.scale - from.scale) * f;
		}
	}
}
