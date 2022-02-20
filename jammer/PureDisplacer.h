#ifndef __PURE_DISPLACER_H__
#define __PURE_DISPLACER_H__

#include "Draggable.h"

class PureDisplacer : public Draggable {
public:
	virtual bool hitCheck(const glm::ivec2& pos) const;
};

#endif //__PURE_DISPLACER_H__