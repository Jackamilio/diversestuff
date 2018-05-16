#ifndef ___ORTHO_MATRIX_H___
#define ___ORTHO_MATRIX_H___

#include <glm/mat4x4.hpp>

class OrthoMatrix {
public:

	OrthoMatrix();
	OrthoMatrix(const OrthoMatrix& om);
	OrthoMatrix(int fromSingleInt);
	void operator=(const OrthoMatrix& om);
	bool operator==(const OrthoMatrix& comp) const;
	bool operator!=(const OrthoMatrix& comp) const;

	void RotXPos();
	void RotXNeg();
	void RotYPos();
	void RotYNeg();
	void RotZPos();
	void RotZNeg();

	void CalcMatrix(glm::mat4& mat) const;

	inline int AsSingleInt() const { return refx | refy << 3 | refz << 6; }
	inline void SetFromSingleInt(int fromSingleInt) {
		refx = fromSingleInt & 0x0007;
		refy = (fromSingleInt >> 3) & 0x0007;
		refz = (fromSingleInt >> 6) & 0x0007; }
private:
	int refx, refy, refz;

	void ApplyRotOnAll(const int* cor, int step);
};

#endif//___ORTHO_MATRIX_H___
