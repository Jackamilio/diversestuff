#ifndef ___GAME_DATA_H___
#define ___GAME_DATA_H___

#include <string>
#include <iostream>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <typeinfo>
#include <sstream>
#include <vector>

class GameData {
private:
	class ValueBase {
	public:
		virtual void* GetValue() const = 0;

		virtual ValueBase* Clone() const = 0;
		virtual void Save(std::ostream& f) const = 0;
		virtual void Load(std::istream& i) = 0;
		virtual const type_info& GetType() const = 0;
	};

public:
	GameData(); // just an empty game data waiting to be filled
	GameData(const std::string& description); // generates a GameData that will be used as a descriptor and its default values
	GameData(const std::string& descriptorname, const std::string& values); // generates a GameData useful for actual data handling
	GameData(GameData* descriptor, const std::string& values); // generates a GameData useful for actual data handling
	GameData(std::istream& f); // generates an anonymous game data, used for loading instances in LevelData
	~GameData();
	void Set(std::istream& f);
	void Set(const std::string& values);



	class ValueContainer {
	private:
		const std::string* type;
		ValueBase* value;
	public:
		ValueContainer();
		ValueContainer(const std::string& type, ValueBase* value);
		ValueContainer(const ValueContainer& rhs);
		~ValueContainer();
		void operator = (const ValueContainer& rhs);
		void* GetValue() const;
		ValueBase* Clone() const;
		void Save(std::ostream& f) const;
		void Load(std::istream& i);
		const type_info& GetType() const;
		const std::string& Type() const;

		template<class T>
		T& GetVal() const;
	};

	template<class T>
	static ValueBase* InstanciateValue();
	static std::map<std::string, ValueBase* (*)()> instanciators;
	static void Init();

	static std::map<std::string, GameData*> descriptors;
	static GameData* GetDescriptor(const std::string& descname);
	static void LoadGameDataDescriptor(const std::string& file);

	static ValueContainer* GenerateContainer(const std::string& type, bool asDescriptor = false);

	typedef std::map<std::string, ValueContainer*> Values;
	Values values;

	template<class T>
	T& GetValue(const std::string& name);
	template<class T>
	const T& GetValue(const std::string& name) const;
	const std::string& GetType(const std::string& name) const;
	template<class T>
	bool HasValue(const std::string& name) const;
	bool IsComposite(const std::string& name) const;
	inline bool HasField(const std::string& name) const { Values::const_iterator cit; return FindFieldConst(name, cit); }
	ValueContainer& GetField(const std::string& name);


	void Save(std::ostream & f) const;
	void Clear();
private:
	void Init(GameData* desc, const std::string& values);
	bool anonymous;

	void AddValue(const std::string& type, const std::string& name, bool asDescriptor);
	bool FindField(const std::string& name, Values::iterator& it);
	bool FindFieldConst(const std::string& name, Values::const_iterator& it) const;

	template<class T>
	class Value : public ValueBase {
	public:
		T value;
		Value();
		void* GetValue() const;
		ValueBase* Clone() const;
		void Save(std::ostream& f) const;
		void Load(std::istream& i);
		const type_info& GetType() const;
	};
	template<class T>
	class ValuePtr : public ValueBase {
	public:
		ValuePtr(T* ptr);
		~ValuePtr();
		T* value;
		void* GetValue() const;
		ValueBase* Clone() const;
		void Save(std::ostream& f) const;
		void Load(std::istream& i);
		const type_info& GetType() const;
	};
};

template<class T>
inline GameData::ValueBase * GameData::InstanciateValue()
{
	return new Value<T>();
}

template<class T>
inline T & GameData::ValueContainer::GetVal() const
{
	return *(T*)GetValue();
}

template<class T>
inline T & GameData::GetValue(const std::string & name)
{
	Values::iterator it;
	assert(FindField(name, it) && it->second->GetType() == typeid(T) && "Value doesn't exists or has wrong type.\n");
	return it->second->GetVal<T>();
}

template<class T>
inline const T & GameData::GetValue(const std::string & name) const
{
	Values::const_iterator it;
	assert(FindFieldConst(name, it) && it->second->GetType() == typeid(T) && "Value doesn't exists or has wrong type.\n");
	return it->second->GetVal<T>();
}

template<class T>
inline bool GameData::HasValue(const std::string & name) const
{
	Values::const_iterator it;
	if (FindFieldConst(name, it)) {
		return it->second->GetType() == typeid(T);
	}
	return false;
}

template<class T>
inline GameData::Value<T>::Value()
{
}

template<class T>
inline void* GameData::Value<T>::GetValue() const
{
	return (void*)&value;
}

template<class T>
inline GameData::ValueBase * GameData::Value<T>::Clone() const
{
	Value<T>* ret = new Value<T>();
	ret->value = value;
	return ret;
}

template<class T>
inline const type_info & GameData::Value<T>::GetType() const
{
	return typeid(T);
}

template<class T>
inline GameData::ValuePtr<T>::ValuePtr(T * ptr) : value(ptr)
{
}

template<class T>
inline GameData::ValuePtr<T>::~ValuePtr()
{
	delete value;
}

template<class T>
inline void* GameData::ValuePtr<T>::GetValue() const
{
	return (void*)value;
}

template<class T>
inline GameData::ValueBase * GameData::ValuePtr<T>::Clone() const
{
	return new ValuePtr<T>(new T(*value));
}

template<class T>
inline const type_info & GameData::ValuePtr<T>::GetType() const
{
	return typeid(T);
}

template<class T>
inline void GameData::Value<T>::Save(std::ostream & f) const
{
	f << value << ' ';
}

template<class T>
inline void GameData::Value<T>::Load(std::istream & f)
{
	f >> value;
}

template<>
inline void GameData::Value<GameData*>::Load(std::istream & f)
{
	assert(0);
}

template<>
inline void GameData::Value<std::string>::Save(std::ostream & f) const
{
	std::string save(value);
	static std::vector<size_t> places;
	places.clear();
	size_t place = save.find('"');
	while (place != std::string::npos) {
		places.push_back(place);
		save.erase(place, 1);
		place = save.find('"');
	}
	for (int i = (int)places.size() - 1; i >= 0 ; --i) {
		save.insert(places[i], "\\\"");
	}
	f << '"' << save << "\" ";
}

template<>
inline void GameData::Value<std::string>::Load(std::istream & f)
{
	value.clear();
	f >> std::ws;
	char c;
	f.read(&c, 1);
	assert(c == '"' && "A string should start with \"\n");
	f.read(&c, 1);
	while (c != '"') {
		if (c == '\\') {
			f.read(&c, 1);
		}
		value.append(1, c);
		f.read(&c, 1);
	}
}

template<>
inline void GameData::Value<glm::vec3>::Save(std::ostream & f) const
{
	f << value.x << ' ' << value.y << ' ' << value.z << ' ';
}

template<>
inline void GameData::Value<glm::vec3>::Load(std::istream & f)
{
	f >> value.x >> value.y >> value.z;
}

template<>
inline void GameData::Value<glm::vec4>::Save(std::ostream & f) const
{
	f << value.x << ' ' << value.y << ' ' << value.z << ' ' << value.w << ' ';
}

template<>
inline void GameData::Value<glm::vec4>::Load(std::istream & f)
{
	f >> value.x >> value.y >> value.z >> value.w;
}

template<>
inline void GameData::Value<glm::quat>::Save(std::ostream & f) const
{
	f << value.x << ' ' << value.y << ' ' << value.z << ' ' << value.w << ' ';
}

template<>
inline void GameData::Value<glm::quat>::Load(std::istream & f)
{
	f >> value.x >> value.y >> value.z >> value.w;
}

template<>
inline void GameData::ValuePtr<GameData>::Save(std::ostream & f) const
{
	value->Save(f);
}

template<>
inline void GameData::ValuePtr<GameData>::Load(std::istream & f)
{
	value->Set(f);
}

#endif//___GAME_DATA_H___
