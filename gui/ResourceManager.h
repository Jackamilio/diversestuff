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
	template<class ... Args>
	ResHandler& GetHandler(const Key& key, Args& ... args);

	template<class ... Args>
	ResValue GetValue(const Key& key, Args& ... arg);

	template<class ... Args>
	const ResValue& GetRefValue(const Key& key, Args& ... arg);

	void Clear();
};


template<class ResHandler, class ResValue, class Key>
template<class ...Args>
inline ResHandler & ResourceManager<ResHandler, ResValue, Key>::GetHandler(const Key & key, Args& ...args)
{
	ResHandler& handler = resources[key];

	if (handler.NeedsToLoad()) {
		handler.Load(key, args...);
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
inline void ResourceManager<ResHandler, ResValue, Key>::Clear()
{
	resources.clear();
}

#endif//___RESOURCE_MANAGER_H___
