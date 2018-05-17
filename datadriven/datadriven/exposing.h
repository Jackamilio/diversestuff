#ifndef __EXPOSING_H__
#define __EXPOSING_H__

#include <AntTweakBar.h>
#include <vector>

namespace Exposer {

	template<class T>
	TwType getTwType() {
		return T::__getTwType();
	}

	template<> TwType getTwType<bool>();
	template<> TwType getTwType<char>();
	template<> TwType getTwType<unsigned char>();
	template<> TwType getTwType<short>();
	template<> TwType getTwType<unsigned short>();
	template<> TwType getTwType<int>();
	template<> TwType getTwType<unsigned int>();
	template<> TwType getTwType<float>();
	template<> TwType getTwType<double>();
	template<> TwType getTwType<std::string>();

	// Types that can't be automatic
	//TW_TYPE_BOOL8
	//TW_TYPE_BOOL16
	//TW_TYPE_BOOL32
	//TW_TYPE_CHAR
	//TW_TYPE_COLOR32
	//TW_TYPE_COLOR3F
	//TW_TYPE_COLOR4F
	//TW_TYPE_CDSTRING
	//TW_TYPE_QUAT4F
	//TW_TYPE_QUAT4D
	//TW_TYPE_DIR3F
	//TW_TYPE_DIR3D
}

extern TwBar *exposureBar;

#define IM_AN_EXPOSER static TwType __getTwType();

// must define EXPOSE_TYPE first
#define STR_(s) #s
#define STR(s) STR_(s)
#define EXPOSE_START \
TwType EXPOSE_TYPE::__getTwType() { \
	static TwType type = TW_TYPE_UNDEF; \
	if (type != TW_TYPE_UNDEF) return type; \
	std::vector<TwStructMember> vec; \
	TwStructMember tmp;

#define __EXPOSE(type, var, ...) \
	tmp = { #var, type, offsetof(EXPOSE_TYPE, var), "" ## __VA_ARGS__}; \
	vec.push_back(tmp);

#define EXPOSE(var, ...) __EXPOSE(Exposer::getTwType<decltype(var)>(), var, __VA_ARGS__)

#define EXPOSE_AS(type, ...) __EXPOSE(TW_TYPE_ ## type, __VA_ARGS__)

#define EXPOSE_PARENT(p, ...) \
	tmp = { "parent" ## "(" ## #p ## ")", Exposer::getTwType<p>(), 0, "" ## __VA_ARGS__}; \
	vec.push_back(tmp);

#define EXPOSE_END \
	type = TwDefineStruct(STR(EXPOSE_TYPE), &vec[0], vec.size(), sizeof(EXPOSE_TYPE), NULL, NULL); \
	return type; \
}

#define EXPOSE_GLOBAL(var) TwAddVarRW(exposureBar, #var, Exposer::getTwType<decltype(var)>(), &var, "")

#endif
