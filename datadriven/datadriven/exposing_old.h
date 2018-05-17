#ifndef __EXPOSING_H__
#define __EXPOSING_H__

#include <iostream>
#include <string>
#include <map>
#include <type_traits>
#include <AntTweakBar.h>

class Exposer;

extern Exposer globalexposer;
extern TwBar *exposureBar;

class Exposed
{
public:
	Exposer* parent;

	Exposed(Exposer* p) : parent(p) {}
	virtual std::string value() = 0;
};

template<class T>
class ExposedT : public Exposed
{
public:
	T* target;

	ExposedT(T* t, Exposer* p) : Exposed(p), target(t) {}
	std::string value() { return "No value"; }
};

template <> std::string ExposedT<int>::value() { return std::to_string(*target); }
template <> std::string ExposedT<unsigned int>::value() { return std::to_string(*target); }
template <> std::string ExposedT<long>::value() { return std::to_string(*target); }
template <> std::string ExposedT<unsigned long>::value() { return std::to_string(*target); }
template <> std::string ExposedT<long long>::value() { return std::to_string(*target); }
template <> std::string ExposedT<unsigned long long>::value() { return std::to_string(*target); }
template <> std::string ExposedT<float>::value() { return std::to_string(*target); }
template <> std::string ExposedT<double>::value() { return std::to_string(*target); }
template <> std::string ExposedT<long double>::value() { return std::to_string(*target); }
template <> std::string ExposedT<char *>::value() { return *target; }

class Exposer
{
private:
	typedef std::map<std::string, Exposed*> expmap;

	expmap exposedvars;

public:

	virtual ~Exposer()
	{
		for (auto& pair : exposedvars) {
			delete pair.second;
		}
	}

	void print(int level = 1);

	template<class T>
	void addExposure(T* t, const char* name) {
		if (std::is_base_of<Exposer, T>::value) {
			Exposer* exp = reinterpret_cast<Exposer*>(t);
			exposedvars[name] = new ExposedT<Exposer>(exp, this);
		}
		else {
			exposedvars[name] = new ExposedT<T>(t, this);
			giveToBar(t, name);
		}
	}

	template<class T>
	static void giveToBar(T* t, const char* name) {
		TwAddButton(exposureBar, name, nullptr, nullptr, "");
	}

	static void giveToBar(int* t, const char* name) {
		TwAddVarRW(exposureBar, name, TW_TYPE_INT32, t, "");
	}
};

#define EXPOSE_GLOBAL(x) globalexposer.addExposure(&x,#x)
#define EXPOSE(x) addExposure(&x,#x)

#endif
