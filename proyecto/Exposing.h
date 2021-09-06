#ifndef __EXPOSING_H__
#define __EXPOSING_H__

#include <string>
#include <allegro5/allegro5.h>

namespace Expose {
	void Value(bool& b, const char* name);
	void Value(int& integer, const char* name);
	void Value(float& flt, const char* name);
	void Value(std::string& str, const char* name);
	void Value(ALLEGRO_BITMAP* img, const char* name);
}

#define IMPLEMENT_EXPOSE void ExposeFunction()
#define IMPLEMENT_EXPOSE_MEMBER(classused) void classused::ExposeFunction()
#define EXPOSE_VALUE(val) Expose::Value(val, #val)

#endif // __EXPOSING_H__
