#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include "dump.h"
#include <math.h>
#include <glm/gtc/matrix_transform.hpp>
#include "TextureManager.h"
#include <locale>

std::string tolower(const std::string& str)
{
	std::locale loc;
	std::string ret(str);
	for (auto &c : ret) {
		c = std::tolower(c, loc);
	}
	return ret;
}

std::string toupper(const std::string& str)
{
	std::locale loc;
	std::string ret(str);
	for (auto &c : ret) {
		c = std::toupper(c, loc);
	}
	return ret;
}

void swapmemorychunks(void* elementA, void* elementB, size_t size)
{
	unsigned char* swap = new unsigned char[size];

	memcpy(swap, elementA, size);
	memcpy(elementA, elementB, size);
	memcpy(elementB, swap, size);

	delete[] swap;
}

// found in https://thispointer.com/how-to-remove-substrings-from-a-string-in-c/
void eraseAllSubStr(std::string& mainStr, const std::string& toErase)
{
	size_t pos = std::string::npos;
	// Search for the substring in string in a loop untill nothing is found
	while ((pos = mainStr.find(toErase)) != std::string::npos)
	{
		// If found then erase it from string
		mainStr.erase(pos, toErase.length());
	}
}

std::string makefilelocal(const std::string& file)
{
	char* alcurdir = al_get_current_directory();
	std::string curdir(alcurdir);
	al_free(alcurdir);

	curdir.append("\\");
	std::string ret(file);
	eraseAllSubStr(ret, curdir);
	
	return ret;
}

void DrawGlWireCube(float neg, float pos)
{
	glBegin(GL_LINES);
	{
		glVertex3f(neg, neg, neg); glVertex3f(pos, neg, neg);
		glVertex3f(pos, neg, neg); glVertex3f(pos, pos, neg);
		glVertex3f(pos, pos, neg); glVertex3f(neg, pos, neg);
		glVertex3f(neg, pos, neg); glVertex3f(neg, neg, neg);

		glVertex3f(neg, neg, pos); glVertex3f(pos, neg, pos);
		glVertex3f(pos, neg, pos); glVertex3f(pos, pos, pos);
		glVertex3f(pos, pos, pos); glVertex3f(neg, pos, pos);
		glVertex3f(neg, pos, pos); glVertex3f(neg, neg, pos);

		glVertex3f(neg, neg, neg); glVertex3f(neg, neg, pos);
		glVertex3f(pos, neg, neg); glVertex3f(pos, neg, pos);
		glVertex3f(pos, pos, neg); glVertex3f(pos, pos, pos);
		glVertex3f(neg, pos, neg); glVertex3f(neg, pos, pos);
	} glEnd();
}

void DrawGlWireCapsule(float radius, float height, int turns)
{
	static glm::vec3 halfCircle[9];
	static bool initHalfCircle = true;
	if (initHalfCircle) {
		for (int i = 0; i <= 8; ++i) {
			float a = glm::pi<float>() * (float)i / 8.0f;
			halfCircle[i].x = glm::cos(a);
			halfCircle[i].y = glm::sin(a);
		}
		initHalfCircle = false;
	}

	float hh = height * 0.5f;
	for (int t = 0; t < turns; ++t) {
		glPushMatrix();

		glRotatef(180.0f * (float)t / (float)turns, 0.0f, 1.0f, 0.0f);

		glBegin(GL_LINE_LOOP);
		for (int i = 0; i <= 8; ++i) {
			const glm::vec3& v = halfCircle[i];
			glVertex3f(v.x*radius, hh + v.y*radius, 0.0f);
		}
		for (int i = 8; i >= 0; --i) {
			const glm::vec3& v = halfCircle[i];
			glVertex3f(v.x*radius, -hh - v.y*radius, 0.0f);
		}
		glEnd();

		glPopMatrix();
	}

	glBegin(GL_LINE_LOOP);
	for (int i = -8; i < 8; ++i) {
		const glm::vec3& v = halfCircle[i < 0 ? -i : i];
		glVertex3f(v.x*radius, hh, v.y*radius * (i < 0 ? -1.0f : 1.0f));
	}
	glEnd();
	glBegin(GL_LINE_LOOP);
	for (int i = -8; i < 8; ++i) {
		const glm::vec3& v = halfCircle[i < 0 ? -i : i];
		glVertex3f(v.x*radius, -hh, v.y*radius * (i < 0 ? -1.0f : 1.0f));
	}
	glEnd();
}

void DrawBrick(const MapData::BrickData* brick, float uo, float uf, float vo, float vf) {
	if (brick) {
		glBegin(GL_TRIANGLES);

		vo += vf;
		vf = -vf;

		const std::vector<int>& tris = brick->GetTriangleList();
		int nbTris = (int)tris.size() / 3;
		for (int i = 0; i < nbTris; ++i) {
			const MapData::Vertex* v1 = brick->GetVertex(tris[i * 3]);
			const MapData::Vertex* v2 = brick->GetVertex(tris[i * 3 + 1]);
			const MapData::Vertex* v3 = brick->GetVertex(tris[i * 3 + 2]);
			if (v1 && v2 && v3) {
				glm::vec3 v1tov2 = v2->xyz - v1->xyz;
				glm::vec3 v1tov3 = v3->xyz - v1->xyz;
				glm::vec3 normal = glm::normalize(glm::cross(v1tov2, v1tov3));

				glNormal3f(normal.x, normal.y, normal.z);

				glTexCoord2f(uo + v1->u * uf, vo + v1->v * vf); glVertex3f(v1->x, v1->y, v1->z);
				glTexCoord2f(uo + v2->u * uf, vo + v2->v * vf); glVertex3f(v2->x, v2->y, v2->z);
				glTexCoord2f(uo + v3->u * uf, vo + v3->v * vf); glVertex3f(v3->x, v3->y, v3->z);
			}
		}

		glEnd();
	}
}

void DrawBrick(const MapData::Brick& brick, TextureManager& texMngr, bool flipTexV) {
	if (brick.brickdata) {
		glm::mat4 mat;
		glPushMatrix();
		brick.matrix.CalcMatrix(mat);
		glMultMatrixf(&mat[0][0]);
		float uo = 0.0f;
		float uf = 1.0f;
		float vo = 0.0f;
		float vf = 1.0f;
		const MapData::TilesetData* tsdt = brick.tilesetdata;
		if (tsdt) {
			texMngr.Bind(tsdt->file);
			float tw = (float)texMngr.GetTexWidth(tsdt->file);
			float th = (float)texMngr.GetTexHeight(tsdt->file);
			uo = (float)(tsdt->ox + brick.tilex * (tsdt->tw + tsdt->px)) / tw;
			vo = (float)(tsdt->oy + brick.tiley * (tsdt->th + tsdt->py)) / th;
			uf = (float)tsdt->tw / tw;
			vf = (float)tsdt->th / th;
			if (flipTexV) {
				vo = 1.0f - vo;
				vf = -vf;
			}
		}
		else {
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		DrawBrick(brick.brickdata, uo, uf, vo, vf);
		glPopMatrix();
	}
}

void DrawBrickWireframe(const MapData::Brick& brick)
{
	const MapData::BrickData* const bd = brick.brickdata;
	if (bd) {
		glm::mat4 mat;
		glPushMatrix();
		brick.matrix.CalcMatrix(mat);
		glMultMatrixf(&mat[0][0]);

		glBegin(GL_LINE_LOOP);

		const std::vector<int>& tris = bd->GetTriangleList();
		int nbTris = (int)tris.size() / 3;
		for (int i = 0; i < nbTris; ++i) {
			const MapData::Vertex* v1 = bd->GetVertex(tris[i * 3]);
			const MapData::Vertex* v2 = bd->GetVertex(tris[i * 3 + 1]);
			const MapData::Vertex* v3 = bd->GetVertex(tris[i * 3 + 2]);
			if (v1 && v2 && v3) {
				glVertex3f(v1->x, v1->y, v1->z);
				glVertex3f(v2->x, v2->y, v2->z);
				glVertex3f(v3->x, v3->y, v3->z);
			}
		}

		glEnd();

		glPopMatrix();
	}
}

void DrawBrickHeap(const MapData::BrickHeap& brickheap, TextureManager& texMngr, bool flipTexV) {
	for (int i = 0; i < brickheap.size(); ++i) {
		DrawBrick(brickheap[i], texMngr, flipTexV);
	}
}

void DrawLevelData(const MapData& level, TextureManager& txmgr, bool flipTexV) {
	for (MapData::const_iterator it = level.begin(); it != level.end(); ++it) {
		glPushMatrix();

		const MapData::Coordinate& c = it->first;
		glTranslatef((float)c.x, (float)c.y, (float)c.z);
		DrawBrickHeap(it->second, txmgr, flipTexV);

		glPopMatrix();
	}
}

inline btVector3 Transform(const MapData::Vertex* vert, const MapData::Coordinate& c, const glm::mat4& mat) {
	glm::vec4 f = mat * glm::vec4(vert->xyz, 1.0f);
	return btVector3((float) c.x + f.x, (float)c.y + f.y, (float)c.z + f.z);
}

btTriangleMesh * ConstructLevelCollision(const MapData & level)
{
	btTriangleMesh* data = new btTriangleMesh();
	glm::mat4 mat;

		
	for (MapData::const_iterator it = level.begin(); it != level.end(); ++it) {
		const MapData::Coordinate& c = it->first;

		for (unsigned int i = 0; i < it->second.size(); ++i) {
			const MapData::Brick& brick = it->second[i];

			brick.matrix.CalcMatrix(mat);

			if (brick.brickdata) {
				const std::vector<int>& tris = brick.brickdata->GetTriangleList();
				int nbTris = tris.size() / 3;
				for (int i = 0; i < nbTris; ++i) {
					const MapData::Vertex* v1 = brick.brickdata->GetVertex(tris[i * 3]);
					const MapData::Vertex* v2 = brick.brickdata->GetVertex(tris[i * 3 + 1]);
					const MapData::Vertex* v3 = brick.brickdata->GetVertex(tris[i * 3 + 2]);
					if (v1 && v2 && v3) {
						data->addTriangle(Transform(v1, c, mat), Transform(v2, c, mat), Transform(v3, c, mat), true);
					}
				}
			}
		}
	}

	return data;
}
