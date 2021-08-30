#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include "dump.h"
#include "MathUtils.h"
#include <math.h>
//#include <FL/Fl.H>
//#include <FL/glu.h>
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

bool linePlaneIntersection(const glm::vec3& plane_center, const glm::vec3& plane_normal, const glm::vec3& line_origin, const glm::vec3& line_direction, bool lineasray, glm::vec3* res)
{
	//plane_normal = glm::normalize(plane_normal);
	//line_direction = glm::normalize(line_direction);

	float denom = glm::dot(plane_normal, line_direction);
	if (abs(denom) > 0.0001f)
	{
		float t = glm::dot((plane_center - line_origin), plane_normal) / denom;
		glm::vec3 intersect = line_origin + line_direction*t;
		if (lineasray && glm::dot(intersect-line_origin,line_direction) < 0.0f) {
			return false;
		}
		if (res) {
			*res = intersect;
		}
		return true;
	}
	return false;
}

// thank you here! http://www.blackpawn.com/texts/pointinpoly/
// are P1 and P2 in the same side compared to [AB] ?
bool sameSide(const glm::vec3 p1, const glm::vec3 p2, const glm::vec3 A, const glm::vec3 B) {
	glm::vec3 cp1 = glm::cross(B - A, p1 - A);
	glm::vec3 cp2 = glm::cross(B - A, p2 - A);
	return glm::dot(cp1, cp2) >= 0.0f;
}

bool pointInTriangle(const glm::vec3& point, const glm::vec3& t1, const glm::vec3& t2, const glm::vec3& t3)
{
	return sameSide(point, t1, t2, t3) && sameSide(point, t2, t1, t3) && sameSide(point, t3, t1, t2);
	/* This code is totally irrelevant ... but I keep the record of this error
	glm::vec3 ab = t2 - t1;
	glm::vec3 bc = t3 - t2;
	glm::vec3 ca = t1 - t3;

	glm::vec3 ap = point - t1;
	glm::vec3 bp = point - t2;
	glm::vec3 cp = point - t3;

	float d1 = glm::dot(ab, ap);
	float d2 = glm::dot(bc, bp);
	float d3 = glm::dot(ca, cp);

	return (d1 >= 0 && d2 >= 0 && d3 >= 0);// || (d1 <= 0 && d2 <= 0 && d3 <= 0);*/
}

bool lineTriangleIntersection(const glm::vec3& t1, const glm::vec3& t2, const glm::vec3& t3, const glm::vec3& line_origin, const glm::vec3& line_direction, bool lineasray, glm::vec3* res)
{
	glm::vec3 plane_normal = glm::normalize(glm::cross(t2 - t1, t3 - t1));
	glm::vec3 point;
	if (linePlaneIntersection(t1, plane_normal, line_origin, line_direction, lineasray, &point)) {
		if (pointInTriangle(point, t1, t2, t3)) {
			if (res) {
				*res = point;
			}
			return true;
		}
	}
	return false;
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
		for (unsigned int i = 0; i <= 8; ++i) {
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

void DrawBrick(const LevelData::BrickData* brick, float uo, float uf, float vo, float vf) {
	if (brick) {
		glBegin(GL_TRIANGLES);

		vo += vf;
		vf = -vf;

		const std::vector<int>& tris = brick->GetTriangleList();
		int nbTris = (int)tris.size() / 3;
		for (int i = 0; i < nbTris; ++i) {
			const LevelData::Vertex* v1 = brick->GetVertex(tris[i * 3]);
			const LevelData::Vertex* v2 = brick->GetVertex(tris[i * 3 + 1]);
			const LevelData::Vertex* v3 = brick->GetVertex(tris[i * 3 + 2]);
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

void DrawBrickHeap(const LevelData::BrickHeap& brickheap, TextureManager& texMngr, bool flipTexV) {
	glm::mat4 mat;
	for (unsigned int i = 0; i < brickheap.size(); ++i) {
		glPushMatrix();
		const LevelData::Brick& brick = brickheap[i];
		brick.matrix.CalcMatrix(mat);
		glMultMatrixf(&mat[0][0]);
		float uo = 0.0f;
		float uf = 1.0f;
		float vo = 0.0f;
		float vf = 1.0f;
		const LevelData::TilesetData* tsdt = brick.tilesetdata;
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
		DrawBrick(brickheap[i].brickdata, uo, uf, vo, vf);
		glPopMatrix();
	}
}

void DrawLevelData(const LevelData& level, TextureManager& txmgr, bool flipTexV) {
	for (LevelData::const_iterator it = level.begin(); it != level.end(); ++it) {
		glPushMatrix();

		const LevelData::Coordinate& c = it->first;
		glTranslatef((float)c.x, (float)c.y, (float)c.z);
		DrawBrickHeap(it->second, txmgr, flipTexV);

		glPopMatrix();
	}
}

/*inline btVector3 Transform(const LevelData::Vertex* vert, const LevelData::Coordinate& c, const glm::mat4& mat) {
	glm::vec4 f = mat * glm::vec4(vert->xyz, 1.0f);
	return btVector3((float) c.x + f.x, (float)c.y + f.y, (float)c.z + f.z);
}

btTriangleMesh * ConstructLevelCollision(const LevelData & level)
{
	btTriangleMesh* data = new btTriangleMesh();
	glm::mat4 mat;

		
	for (LevelData::const_iterator it = level.begin(); it != level.end(); ++it) {
		const LevelData::Coordinate& c = it->first;

		for (unsigned int i = 0; i < it->second.size(); ++i) {
			const LevelData::Brick& brick = it->second[i];

			brick.matrix.CalcMatrix(mat);

			if (brick.brickdata) {
				const std::vector<int>& tris = brick.brickdata->GetTriangleList();
				int nbTris = tris.size() / 3;
				for (int i = 0; i < nbTris; ++i) {
					const LevelData::Vertex* v1 = brick.brickdata->GetVertex(tris[i * 3]);
					const LevelData::Vertex* v2 = brick.brickdata->GetVertex(tris[i * 3 + 1]);
					const LevelData::Vertex* v3 = brick.brickdata->GetVertex(tris[i * 3 + 2]);
					if (v1 && v2 && v3) {
						data->addTriangle(Transform(v1, c, mat), Transform(v2, c, mat), Transform(v3, c, mat), true);
					}
				}
			}
		}
	}

	return data;
}*/

glm::mat4 LocRotScaleToMat4(const LocRotScale& poseBone)
{
	glm::mat4 tr = glm::mat4(1.0f);
	tr = glm::translate(tr, poseBone.loc);

	glm::mat4 rot = glm::mat4_cast(poseBone.rot);
	tr = tr * rot;

	tr = glm::scale(tr, poseBone.scale);
	return tr;
}