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

	template<typename T> class StructDescIndexedContainer;

	// what a mindfuck. Plz C++ gods, make metaprogramming simpler!!
	// thanks to :
	// https://stackoverflow.com/questions/12015195/how-to-call-member-function-only-if-object-happens-to-have-it
	template< typename T>
	struct has_Type_getType
	{
		struct type_ok {};
		struct type_fail {};
		
		// SFINAE __getType-exists :)
		template <typename A>
		static type_ok test(decltype(&A::__getType), void *) {
			return type_ok();
		}

		// SFINAE game over :(
		template<typename A>
		static type_fail test(...) {
			return type_fail();
		}

		// This will be either `std::true_type` or `std::false_type`
		typedef decltype(test<T>(0, 0)) type;

		static Type _eval(type_ok) {
			return T::__getType();
		}

		// `eval(...)` is a no-op for otherwise unmatched arguments
		static Type _eval(...) {
			return UNDEF;
		}

		// `eval()` delegates to :-
		//- `eval(type())` when `type` == `std::true_type`
		//- `eval(...)` otherwise
		static Type eval() {
			return _eval(type());
		}
	};

	template<class T>
	Type getType() {
		return has_Type_getType<T>::eval();
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

	std::string getBasicTypeAsString(Exposing::Type type, char* address);
	bool setBasicTypeFromString(Exposing::Type type, char* address, const std::string& str);

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

	template<typename C, typename I = int>
	class WatchedAddressIndexedContainer : public WatchedAddress {
	public:
		const WatchedAddress* owner;
		I index;
		char* calculateAddress() const { return (char*)&(*((C*)owner->calculateAddress()))[index]; }
		WatchedAddressIndexedContainer(const WatchedAddress* owner, I& index) : owner(owner), index(index) {}
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
			virtual ~Iterator() {};
			virtual void next() = 0;
			virtual bool isAtEnd() const = 0;
			virtual WatchedAddress* generateWatchedAddress() = 0;
			virtual std::string getName() const = 0;
			virtual Type getType() const = 0;
		};

		virtual Iterator* generateIterator(WatchedAddress* from) = 0;
		virtual bool isContainer() const { return true; }
	};

	typedef std::vector<StructMember> StructInfo; // use map instead of vector?

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
		bool isContainer() const { return false; }
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
			}
			bool isAtEnd() const {
				return index >= (int)container().size();
			}
			WatchedAddress* generateWatchedAddress() {
				return new WatchedAddressIndexedContainer<C>(watchedContainer,index);
			}
			std::string getName() const {
				return std::to_string(index);
			}
			Type getType() const {
				return Exposing::getType<std::remove_reference<decltype(container()[0])>::type>();
			}
		};

		Iterator* generateIterator(WatchedAddress* from) {
			return new Iterator_(from);
		}
	};

	template<typename C>
	class StructDescMapContainer : public StructDescBase {
	public:
		StructDescMapContainer(const char* n) : StructDescBase(n) {}

		class Iterator_ : public Iterator {
		public:
			WatchedAddress * watchedContainer;
			typename C::iterator it;

			inline C& container() const { return *((C*)watchedContainer->calculateAddress()); }
			Iterator_(WatchedAddress* from) : watchedContainer(from){
				it = container().begin();
			}

			virtual void next(){
				++it;
			}
			virtual bool isAtEnd() const {
				return (it == container().end());
			}
			virtual WatchedAddress* generateWatchedAddress() {
				return new WatchedAddressIndexedContainer<C, std::remove_reference_t<decltype(it->first)>>(watchedContainer, it->first);
			}
			virtual std::string getName() const {
				return getBasicTypeAsString(Exposing::getType<std::remove_const_t<std::remove_reference_t<decltype(it->first)>>>(), (char*)&it->first);
			}
			virtual Type getType() const {
				return Exposing::getType<std::remove_reference_t<decltype(it->second)>>();
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

	template<class T>
	Type getMapContainerType() {
		static Type thisContainerType = registerNewType("MapContainer", new StructDescMapContainer<T>("MapContainer"));
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

		class EditValueArgs {
		public:
			WatchedAddress* address;
			Type type;
			jmg::Base* field;
			jmg::Label* label;
			jmg::ShowHide* sh;

			EditValueArgs(WatchedAddress* address, Type type);
			~EditValueArgs();
		};

		std::vector<EditValueArgs*> mValueArgs;

		int pushNewField(StructDescBase::Iterator* it, int y, bool nameIsIndex);

		void refreshValueForLabels();

		void draw(int origx, int origy);

		Watcher(StructDescBase* sc, WatchedAddress* wa, int y = 0);
		virtual ~Watcher();

		int getHeight() const;
	};

	class WatcherWindow : public Watcher, public jmg::Window {
	public:
		WatcherWindow(StructDescBase* sc, WatchedAddress* wa);
		~WatcherWindow();

		void draw(int origx, int origy);

		int getHeight() const;
	};

	template<class T>
	jmg::Window* createWatcherFor(T& obj);

	template<class T>
	void saveToFile(T& obj, const char* filename);
	void saveToFile(StructDescBase* sc, WatchedAddress* wa, const char* f);

	template<class T>
	bool loadFromFile(T& obj, const char* filename);
	bool loadFromFile(StructDescBase* sc, WatchedAddress* wa, const char* f);

	// type detection to have correct behaviour
	struct regular_type {};
	struct indexed_container {};
	struct map_container {};
	template<typename T> struct detect_type : public regular_type {};

	template<typename T, typename A>
	struct detect_type<std::vector<T, A>> : public indexed_container {};

	template<typename K, typename T, typename C, typename A>
	struct detect_type<std::map<K, T, C, A>> : public map_container {};

	template <typename T>
	class CorrectGetType {
	private:
		static Type getType(map_container) {
			return Exposing::getMapContainerType<T>();
		}
		static Type getType(indexed_container) {
			return Exposing::getIndexedContainerType<T>();
		}
		static Type getType(regular_type) {
			return Exposing::getType<T>();
		}
	public:
		static Type getType() {
			return getType(detect_type<T>{});
		}
	};
};


template<class T>
jmg::Window * Exposing::createWatcherFor(T& obj)
{
	Exposing::Type typeToWatch = Exposing::CorrectGetType<T>::getType();

	StructDescBase* sc = registeredTypes[typeToWatch];

	jmg::Window* window = new WatcherWindow(sc, new WatchedAddressRoot((char*)&obj));

	return window;
}

template<class T>
void Exposing::saveToFile(T& obj, const char* filename)
{
	Exposing::Type typeToWatch = Exposing::CorrectGetType<T>::getType();
	StructDescBase* sc = registeredTypes[typeToWatch];

	saveToFile(sc, &WatchedAddressRoot((char*)&obj), filename);
}

template<class T>
bool Exposing::loadFromFile(T& obj, const char* filename)
{
	Exposing::Type typeToWatch = Exposing::CorrectGetType<T>::getType();
	StructDescBase* sc = registeredTypes[typeToWatch];

	return loadFromFile(sc, &WatchedAddressRoot((char*)&obj), filename);
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

#define __EXPOSE(type, var, ...) \
	tmp = Exposing::StructMember(#var, type, offsetof(EXPOSE_TYPE, var)); \
	vec.push_back(tmp);

#define EXPOSE(var, ...) __EXPOSE(Exposing::CorrectGetType<decltype(var)>::getType(), var, __VA_ARGS__)

#define EXPOSE_END \
	type = Exposing::defineStruct(STR(EXPOSE_TYPE), vec); \
	return type; \
}

#endif
