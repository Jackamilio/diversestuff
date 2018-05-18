#ifndef __EXPOSING_H__
#define __EXPOSING_H__

#include <string>
#include <vector>

namespace Exposing {
	enum Type {
		UNDEF,
		BOOL,
		INT8,
		UINT8,
		INT16,
		UINT16,
		INT,
		UINT,
		FLOAT,
		DOUBLE,
		STRING,
		
		MAX

		// Types that can't be automatic
		//BOOL8
		//BOOL16
		//BOOL32
		//CHAR
		//COLOR32
		//COLOR3F
		//COLOR4F
		//CDSTRING
		//QUAT4F
		//QUAT4D
		//DIR3F
		//DIR3D
	};

	template<class T>
	Type getType() {
		return T::__getType();
	}

	template<> Type getType<bool>();
	template<> Type getType<char>();
	template<> Type getType<unsigned char>();
	template<> Type getType<short>();
	template<> Type getType<unsigned short>();
	template<> Type getType<int>();
	template<> Type getType<unsigned int>();
	template<> Type getType<float>();
	template<> Type getType<double>();
	template<> Type getType<std::string>();

	class StructMember {
	public:
		std::string name;
		Type type;
		unsigned int offset;
		// options?

		StructMember();
		StructMember(const char* n, Type t, unsigned int o);
	};

	Type defineStruct(const char* name, const std::vector<StructMember>& members, unsigned int structSize);
};


#define IM_AN_EXPOSER static TwType __getTwType();

// must define EXPOSE_TYPE first
#define STR_(s) #s
#define STR(s) STR_(s)
#define EXPOSE_START \
TwType EXPOSE_TYPE::__getTwType() { \
	static Exposing type = Exposing::UNDEF; \
	if (type != Exposing::UNDEF) return type; \
	std::vector<Exposing::StructMember> vec; \
	Exposing::StructMember tmp;

#define __EXPOSE(type, var, ...) \
	tmp = Exposing::StructMember(#var, type, offsetof(EXPOSE_TYPE, var)); \//tmp = { #var, type, offsetof(EXPOSE_TYPE, var), "" ## __VA_ARGS__}; \
	vec.push_back(tmp);

#define EXPOSE(var, ...) __EXPOSE(Exposing::getType<decltype(var)>(), var, __VA_ARGS__)

#define EXPOSE_AS(type, ...) __EXPOSE(TW_TYPE_ ## type, __VA_ARGS__)

#define EXPOSE_PARENT(p, ...) \
	tmp = { "parent" ## "(" ## #p ## ")", Exposing::getType<p>(), 0, "" ## __VA_ARGS__}; \
	vec.push_back(tmp);

#define EXPOSE_END \
	type = Exposing::DefineStruct(STR(EXPOSE_TYPE), vec, sizeof(EXPOSE_TYPE)); \
	return type; \
}

#endif

