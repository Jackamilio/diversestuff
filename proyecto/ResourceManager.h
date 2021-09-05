#ifndef ___RESOURCE_MANAGER_H___
#define ___RESOURCE_MANAGER_H___

#include <map>
#include <string>

/*
ResHandler must allow default constructor and define :
	const ResValue& GetValue() const;
	void Load(const std::string& file);
	bool NeedsToLoad() const;
*/
template<class ResHandler, class ResValue, class Key = std::string>
class ResourceManager {
private:
	std::map<Key, ResHandler> resources;

public:
	// use this to verify if files exist for instance, and return a default one for instance
	virtual const Key& ValidateKey(const Key& key);

	template<class ... Args>
	ResHandler& GetHandler(const Key& key, Args& ... args);

	template<class ... Args>
	ResValue GetValue(const Key& key, Args& ... arg);

	template<class ... Args>
	const ResValue& GetRefValue(const Key& key, Args& ... arg);

	const Key GetKey(const ResHandler& value, const Key& def) const;

	void RemoveValue(const Key& key);
	void Clear();
};

template<class ResHandler, class ResValue, class Key>
inline const Key& ResourceManager<ResHandler, ResValue, Key>::ValidateKey(const Key& key)
{
	return key;
}

template<class ResHandler, class ResValue, class Key>
template<class ...Args>
inline ResHandler & ResourceManager<ResHandler, ResValue, Key>::GetHandler(const Key & key, Args& ...args)
{
	const Key& vkey = ValidateKey(key);
	ResHandler& handler = resources[vkey];

	if (handler.NeedsToLoad()) {
		handler.Load(vkey, args...);
	}

	return handler;
}

template<class ResHandler, class ResValue, class Key>
template<class ...Args>
inline ResValue ResourceManager<ResHandler, ResValue, Key>::GetValue(const Key& key, Args& ... arg)
{
	return GetHandler(key, arg...).GetValue();
}

template<class ResHandler, class ResValue, class Key>
template<class ...Args>
inline const ResValue & ResourceManager<ResHandler, ResValue, Key>::GetRefValue(const Key& key, Args& ... arg)
{
	return GetHandler(key, arg...).GetValue();
}

template<class ResHandler, class ResValue, class Key>
inline const Key ResourceManager<ResHandler, ResValue, Key>::GetKey(const ResHandler& value, const Key& def) const
{
	// THIS, FREAKING THIS :
	// changed auto to auto&&, solution found in https://stackoverflow.com/questions/20709342/unexpected-copies-with-foreach-over-a-map
	// when GetKey was called for the Programs (in Shader.h) it created copies of programs
	// resulting on their destructor to be called at the end,
	// invalidating their glProgram, messing up the drawings.
	// Hello headache, and hours of my life lost...
	for (auto&& it : resources) {
		if (it.second == value) {
			return it.first;
		}
	}
	return def;
}

template<class ResHandler, class ResValue, class Key>
inline void ResourceManager<ResHandler, ResValue, Key>::RemoveValue(const Key& key)
{
	auto it = resources.find(key);
	if (it != resources.end()) {
		resources.erase(it);
	}
}

template<class ResHandler, class ResValue, class Key>
inline void ResourceManager<ResHandler, ResValue, Key>::Clear()
{
	resources.clear();
}

#endif//___RESOURCE_MANAGER_H___
