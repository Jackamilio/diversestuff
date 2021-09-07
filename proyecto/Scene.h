#ifndef __SCENE_H__
#define __SCENE_H__

#include <map>
#include <string>
#include "Arborescent.h"
#include "Engine.h"

// This file has been sponsored by STAKCOVERFLOW (joking)
// I adapted it for myself obviously. There are no animal.
// https://stackoverflow.com/questions/10332725/how-to-automatically-register-a-class-on-creation

class RegisteredConstructor;

class ConstructorCollection {
private:
	ConstructorCollection();
	~ConstructorCollection();
public:
	
	// too lazy to restrict access to this
	// please don't remove, add, or alter the elements
	std::map<std::string, RegisteredConstructor*> themap;
	
	static ConstructorCollection& Get();
};

class SceneBase : public virtual Engine::Object, public Arborescent<SceneBase> {

};

class RegisteredConstructor {
public:
	virtual SceneBase* Construct() = 0;
};

template<class T>
class SceneObject : public SceneBase {
private:
	class NewConstructor : public RegisteredConstructor {
		SceneBase* Construct() {
			return new T;
		}
	};

	static bool reg;
	static bool AutomaticRegistering() {
		ConstructorCollection::Get().themap[T::StaticObjectTypeName()] = new NewConstructor;
		return true;
	}
public:
	SceneObject() {
		reg;
	}
};

template<class T>
bool SceneObject<T>::reg = SceneObject<T>::AutomaticRegistering();

#endif //__SCENE_H__