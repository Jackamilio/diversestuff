#ifndef __EXPOSING_H__
#define __EXPOSING_H__

#include <string>
#include <vector>
#include <map>
#include "JackamikazGUI.h"

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

	typedef std::vector<StructMember> StructInfo;

	class StructComplete {
	public:
		std::string name;
		StructInfo desc;

		StructComplete();
		StructComplete(const char* n, const StructInfo& d);
	};

	extern std::map<Exposing::Type, StructComplete> registeredTypes;

	Type defineStruct(const char* name, const StructInfo& members);

	class WatchedAddress {
	public:
		virtual char* calculateAddress() const = 0;
	};

	class WatchedAddressRoot : public WatchedAddress {
	public:
		char* address;
		char* calculateAddress() const { return address; }
		WatchedAddressRoot(char* address) : address(address) {}
	};

	class WatchedAddressOffset : public WatchedAddress {
	public:
		const WatchedAddress * owner;
		unsigned int offset;
		char* calculateAddress() const { return owner->calculateAddress() + offset; }
		WatchedAddressOffset(const WatchedAddress* owner, unsigned int offset) : owner(owner), offset(offset) {}
	};

	template<typename container>
	class WatchedAddressIndexedContainer : public WatchedAddress {
	public:
		const WatchedAddress* owner;
		int index;
		char* calculateAddress() const { return &((container*)owner->calculateAddress())[index]; }
	};

	class Watcher : public virtual jmg::Base {
	public:
		int calculatedHeight;

		const StructInfo& mWatchedStruct;
		WatchedAddress* mWatchedAddress;
		std::vector<jmg::Base*> mToDelete;

		struct EditValueArgs {
			WatchedAddress* address;
			Type type;
			jmg::Base* field;
		};

		std::vector<EditValueArgs*> mValueArgs;

		void refreshValueForLabels();

		void draw(int origx, int origy);

		Watcher(const StructInfo& sc, WatchedAddress* wa, int y = 0);
		~Watcher();

		int getHeight() const;
	};

	class WatcherWindow : public Watcher, public jmg::Window {
	public:
		WatcherWindow(const StructComplete& sc, WatchedAddress* wa);

		void draw(int origx, int origy);

		int getHeight() const;
	};

	template<class T>
	jmg::Window* createWatcherFor(T& obj);
};


template<class T>
jmg::Window * Exposing::createWatcherFor(T& obj)
{
	Exposing::Type typeToWatch = Exposing::getType<T>();

	const StructComplete& sc = registeredTypes[typeToWatch];

	jmg::Window* window = new WatcherWindow(sc, new WatchedAddressRoot((char*)&obj));

	return window;
}


#define IM_AN_EXPOSER static Exposing::Type __getType();

// must define EXPOSE_TYPE first
#define STR_(s) #s
#define STR(s) STR_(s)
#define EXPOSE_START \
Exposing::Type EXPOSE_TYPE::__getType() { \
	static Exposing::Type type = Exposing::UNDEF; \
	if (type != Exposing::UNDEF) return type; \
	std::vector<Exposing::StructMember> vec; \
	Exposing::StructMember tmp;

//tmp = { #var, type, offsetof(EXPOSE_TYPE, var), "" ## __VA_ARGS__};
#define __EXPOSE(type, var, ...) \
	tmp = Exposing::StructMember(#var, type, offsetof(EXPOSE_TYPE, var)); \
	vec.push_back(tmp);

#define EXPOSE(var, ...) __EXPOSE(Exposing::getType<decltype(var)>(), var, __VA_ARGS__)

#define EXPOSE_AS(type, ...) __EXPOSE(TW_TYPE_ ## type, __VA_ARGS__)

#define EXPOSE_PARENT(p, ...) \
	tmp = { "parent" ## "(" ## #p ## ")", Exposing::getType<p>(), 0, "" ## __VA_ARGS__}; \
	vec.push_back(tmp);

#define EXPOSE_END \
	type = Exposing::defineStruct(STR(EXPOSE_TYPE), vec); \
	return type; \
}

#endif

