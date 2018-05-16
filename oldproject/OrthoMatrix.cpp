#include "OrthoMatrix.h"
#include <glm/vec4.hpp>

glm::vec4 vectors[] = {
	glm::vec4( 1, 0, 0, 0),
	glm::vec4(-1, 0, 0, 0),
	glm::vec4( 0, 1, 0, 0),
	glm::vec4( 0,-1, 0, 0),
	glm::vec4( 0, 0, 1, 0),
	glm::vec4( 0, 0,-1, 0),
	glm::vec4( 0, 0, 0, 1)
};

int xcor[] = { 2,4,3,5 };
int ycor[] = { 0,5,1,4 };
int zcor[] = { 0,2,1,3 };

OrthoMatrix::OrthoMatrix()
	: refx(0)
	, refy(2)
	, refz(4)
{
}

OrthoMatrix::OrthoMatrix(const OrthoMatrix & om)
	: refx(om.refx)
	, refy(om.refy)
	, refz(om.refz)
{
}

OrthoMatrix::OrthoMatrix(int fromSingleInt)
	: refx(fromSingleInt & 0x0007)
	, refy((fromSingleInt >> 3) & 0x0007)
	, refz((fromSingleInt >> 6) & 0x0007)
{
}

void OrthoMatrix::operator=(const OrthoMatrix & om)
{
	refx = om.refx;
	refy = om.refy;
	refz = om.refz;
}

bool OrthoMatrix::operator==(const OrthoMatrix & comp) const
{
	return refx == comp.refx && refy == comp.refy && comp.refz == comp.refz;
}

bool OrthoMatrix::operator!=(const OrthoMatrix & comp) const
{
	return refx != comp.refx || refy != comp.refy || comp.refz != comp.refz;
}

void OrthoMatrix::RotXPos()
{
	ApplyRotOnAll(xcor, 1);
}

void OrthoMatrix::RotXNeg()
{
	ApplyRotOnAll(xcor, -1);
}

void OrthoMatrix::RotYPos()
{
	ApplyRotOnAll(ycor, 1);
}

void OrthoMatrix::RotYNeg()
{
	ApplyRotOnAll(ycor, -1);
}

void OrthoMatrix::RotZPos()
{
	ApplyRotOnAll(zcor, 1);
}

void OrthoMatrix::RotZNeg()
{
	ApplyRotOnAll(zcor, -1);
}

void OrthoMatrix::CalcMatrix(glm::mat4 & mat) const
{
	mat[0] = vectors[refx];
	mat[1] = vectors[refy];
	mat[2] = vectors[refz];
	/*mat[3] = vectors[6];

	glm::mat4 tr1;
	tr1[3] = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

	glm::mat4 tr2;
	tr2[3] = glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f);

	mat = tr1 * mat * tr2;*/

	float sumRefX = vectors[refx].x + vectors[refy].x + vectors[refz].x;
	float sumRefY = vectors[refx].y + vectors[refy].y + vectors[refz].y;
	float sumRefZ = vectors[refx].z + vectors[refy].z + vectors[refz].z;

	mat[3] = glm::vec4(sumRefX < 0.0f ? 1.0f : 0.0f, sumRefY < 0.0f ? 1.0f : 0.0f, sumRefZ < 0.0f ? 1.0f : 0.0f, 1.0f);
}

void ApplyRotOnOne(const int* cor, int step, int& v) {
	for (unsigned int i = 0; i < 4; ++i) {
		if (v == cor[i]) {
			v = cor[(i + 4 + step) % 4];
			return;
		}
	}
}

void OrthoMatrix::ApplyRotOnAll(const int * cor, int step)
{
	ApplyRotOnOne(cor, step, refx);
	ApplyRotOnOne(cor, step, refy);
	ApplyRotOnOne(cor, step, refz);
}
