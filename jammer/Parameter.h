#ifndef __PARAMETER_H__
#define __PARAMETER_H__

#include <vector>
#include "Jammer.h"

union Parameter {
	float fvalue;
	Jammer::Sprite* sprite;
	void* pointer;
};
typedef std::vector<Parameter> ParameterList;

#endif //__PARAMETER_H__