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

	template<typename C>
	class WatchedAddressIndexedContainer : public WatchedAddress {
	public:
		const WatchedAddress* owner;
		int index;
		char* calculateAddress() const { return (char*)&(*((C*)owner->calculateAddress()))[index]; }
		WatchedAddressIndexedContainer(const WatchedAddress* owner, int index) : owner(owner), index(index) {}
	};

	class StructMember {
	public:
		std::string name;
		Type type;
		unsigned int offset;
		// options?

		StructMember();
		StructMember(const char* n, Type t, unsigned int o);
	};

	class StructDescBase {
	public:
		std::string name;

		StructDescBase(const char* n = "struct has no name");

		class Iterator {
		public:
			virtual void next() = 0;
			virtual bool isAtEnd() const = 0;
			virtual WatchedAddress* generateWatchedAddress() = 0;
			virtual std::string getName() const = 0;
			virtual Type getType() const = 0;
		};

		virtual Iterator* generateIterator(WatchedAddress* from) = 0;
	};

	//class StructComplete {
	//public:
	//	std::string name;
	//	StructInfo desc;
	//
	//	StructComplete();
	//	StructComplete(const char* n, const StructInfo& d);
	//};

	typedef std::vector<StructMember> StructInfo;

	class StructDesc : public StructDescBase {
	public:
		StructInfo desc;

		StructDesc(const char* n, const StructInfo& d);

		class Iterator_ : public Iterator {
		public:
			const StructInfo& desc;
			WatchedAddress * watchedStruct;
			int index;

			Iterator_(const StructInfo& desc, WatchedAddress * watchedStruct);

			void next();
			bool isAtEnd() const;
			WatchedAddress* generateWatchedAddress();
			std::string getName() const;
			Type getType() const;
		};

		Iterator* generateIterator(WatchedAddress* from);
	};

	template<typename C>
	class StructDescIndexedContainer : public StructDescBase {
	public:
		StructDescIndexedContainer(const char* n) : StructDescBase(n) {}

		class Iterator_ : public Iterator {
		public:
			WatchedAddress * watchedContainer;
			int index;

			Iterator_(WatchedAddress* from) : index(0), watchedContainer(from) {}

			inline C& container() const { return *((C*)watchedContainer->calculateAddress()); }

			void next(){
				++index;
			};
			bool isAtEnd() const {
				return index >= (int)container().size();
			}
			WatchedAddress* generateWatchedAddress() {
				return new WatchedAddressIndexedContainer<C>(watchedContainer,index);
			}
			std::string getName() const {
				return std::string("[") + std::to_string(index) + std::string("]");
			}
			Type getType() const {
				return Exposing::getType<std::remove_reference<decltype(container()[0])>::type>();
			}
		};

		Iterator* generateIterator(WatchedAddress* from) {
			return new Iterator_(from);
		}
	};

	template<class T>
	Type getIndexedContainerType() {
		static Type thisContainerType = registerNewType("IndexedContainer", new StructDescIndexedContainer<T>("IndexedContainer"));
		return thisContainerType;
	}

	extern std::map<Exposing::Type, StructDescBase*> registeredTypes;

	Type registerNewType(const char* name, StructDescBase* desc);
	Type defineStruct(const char* name, const StructInfo& members);

	class Watcher : public virtual jmg::Base {
	public:
		int calculatedHeight;

		StructDescBase* mWatchedStruct;
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

		Watcher(StructDescBase* sc, WatchedAddress* wa, int y = 0);
		~Watcher();

		int getHeight() const;
	};

	class WatcherWindow : public Watcher, public jmg::Window {
	public:
		WatcherWindow(StructDescBase* sc, WatchedAddress* wa);

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

	StructDescBase* sc = registeredTypes[typeToWatch];

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

#define EXPOSE_IC(var, ...) __EXPOSE(Exposing::getIndexedContainerType<decltype(var)>(), var, __VA_ARGS__)

#define EXPOSE_END \
	type = Exposing::defineStruct(STR(EXPOSE_TYPE), vec); \
	return type; \
}

#endif

/*
Notes pour continuer

1) définir un Exposing::getType alternatif pour les containers indexés
2) polymorphiser StructComplete pour avoir les types différents
     - le type "StructComplete" qui va simplement encapsuler le vecteur
	 - un autre type genre "StructIndexedContainer" cqfd
3) créér un itérateur polymorphe simpliste pour parcourir tout ça
4) adapter le constructeur de Watcher : le WatchedAddressOffset devra être remplacé par un genre
de "generateWatchAddress" depuis l'itérateur

attention au mind fuck "polymorphiser StructInfo ou StructComplete???"
  -> StructComplete est simplement un StructInfo avec un nom collé avec

*/