#include "GameData.h"
#include <fstream>

std::map<std::string, GameData::ValueBase* (*)()> GameData::instanciators;
std::map<std::string, GameData*> GameData::descriptors;

void GameData::Init()
{
	instanciators["int"] = InstanciateValue<int>;
	instanciators["uint"] = InstanciateValue<unsigned int>;
	instanciators["float"] = InstanciateValue<float>;
	instanciators["double"] = InstanciateValue<double>;
	instanciators["string"] = InstanciateValue<std::string>;
	instanciators["vec3"] = InstanciateValue<glm::vec3>;
	instanciators["vec4"] = InstanciateValue<glm::vec4>;
	instanciators["quat"] = InstanciateValue<glm::quat>;

	descriptors["locrotscale"] = new GameData("vec3 loc quat rot vec3 scale");
}

GameData * GameData::GetDescriptor(const std::string& descname)
{
	std::map<std::string, GameData*>::iterator it = descriptors.find(descname);
	assert(it != descriptors.end() && "Descriptor is not defined\n");
	return it->second;
}

void GameData::LoadGameDataDescriptor(const std::string & file)
{
	std::ifstream f;
	f.open(file, std::ios::in);
	if (f.is_open()) {
		while (!f.eof()) {
			std::string name;
			f >> name;
			std::ostringstream ss;
			std::string str;
			f >> std::ws;
			char c;
			f.read(&c, 1);
			assert(c == '{');
			bool cont = true;
			while (cont) {
				f >> str;
				if (str[str.size() - 1] == '}') {
					cont = false;
					if (str.size() > 1) {
						str = str.substr(0, str.size() - 1);
					}
					else {
						str.clear();
					}
				}
				ss << str << ' ';
			}
			descriptors[name] = new GameData(ss.str());
		}
		f.close();
	}
}

GameData::ValueContainer * GameData::GenerateContainer(const std::string & type_, bool asDescriptor)
{
	static std::map<std::string, std::string> types;
	const std::string& type = types[type_] = type_; // I wanted to use set, but nothing beats the convenience of the operator []

	std::map<std::string, GameData::ValueBase* (*)()>::iterator it = instanciators.find(type);
	if (it == instanciators.end()) {
		if (asDescriptor) {
			Value<GameData*>* val = new Value<GameData*>();
			val->value = GetDescriptor(type);
			return new ValueContainer(type, val);
		}
		else {
			return new ValueContainer(type, new ValuePtr<GameData>(new GameData(type, "")));
		}
	}
	else {
		return new ValueContainer(type, it->second());
	}
}

const std::string & GameData::GetType(const std::string & name) const
{
	Values::const_iterator it;
	assert(FindFieldConst(name, it));
	return it->second->Type();
}

bool GameData::IsComposite(const std::string & name) const
{
	Values::const_iterator it;
	assert(FindFieldConst(name, it));
	return it->second->GetType() == typeid(GameData);
}

bool GameData::FindField(const std::string & name, Values::iterator& it)
{
	size_t pos = name.find_first_of('.', 0);
	if (pos == std::string::npos) {
		it = values.find(name);
		return it != values.end();
	}
	else {
		std::string myField = name.substr(0, pos);
		it = values.find(myField);
		if (it != values.end()) {
			GameData& gd = it->second->GetVal<GameData>();
			return gd.FindField(name.substr(pos + 1, name.size() - pos - 1), it);
		}
	}
	return false;
}

bool GameData:: GameData::FindFieldConst(const std::string & name, Values::const_iterator& it) const
{
	size_t pos = name.find_first_of('.', 0);
	if (pos == std::string::npos) {
		it = values.find(name);
		return it != values.end();
	}
	else {
		std::string myField = name.substr(0, pos);
		it = values.find(myField);
		if (it != values.end()) {
			const GameData& gd = it->second->GetVal<GameData>();
			return gd.FindFieldConst(name.substr(pos + 1, name.size() - pos - 1), it);
		}
	}
	return false;
}

GameData::ValueContainer & GameData::GetField(const std::string & name)
{
	Values::iterator it;
	assert(FindField(name, it));
	return *it->second;
}

void GameData::Save(std::ostream & f) const
{
	for (Values::const_iterator it = values.begin(); it != values.end(); ++it) {
		if (anonymous) {
			f << it->second->Type() << ' ';
		}
		f << it->first << " = { ";
		it->second->Save(f);
		f << " } ";
		if (anonymous) {
			f << '\n';
		}
	}
}

void GameData::Clear()
{
	for (Values::iterator it = values.begin(); it != values.end(); ++it) {
		delete it->second;
	}
	values.clear();
}

GameData::GameData()
{
	anonymous = true;
}

GameData::GameData(const std::string& descriptor)
{
	anonymous = false;
	std::istringstream ss(descriptor);

	std::string valtype;
	std::string valname;

	while (!ss.eof()) {
		ss >> valtype;
		if (ss.eof()) {
			return;
		}
		ss >> valname;
		AddValue(valtype, valname, true);
		ss >> std::ws;
		int c = ss.peek();
		if (c != EOF) {
			if ((char)c == '=') {
				ss.ignore(1);
				values[valname]->Load(ss);
			}
		}
	}
}

GameData::GameData(const std::string & descriptorname, const std::string & values)
{
	anonymous = false;
	Init(GetDescriptor(descriptorname), values);
}

GameData::GameData(GameData * descriptor, const std::string & values)
{
	anonymous = false;
	Init(descriptor, values);
}

GameData::GameData(std::istream & f)
{
	anonymous = false;
	Set(f);
}

GameData::~GameData()
{
	Clear();
}

void GameData::Init(GameData * desc, const std::string & vals)
{
	for (Values::iterator it = desc->values.begin(); it != desc->values.end(); ++it) {
		if (it->second->GetType() == typeid(GameData*)) {
			values[it->first] = new ValueContainer(it->second->Type(), new ValuePtr<GameData>(new GameData(it->second->GetVal<GameData*>(), "")));
		}
		else {
			values[it->first] = new ValueContainer(it->second->Type(), it->second->Clone());
		}
	}

	Set(vals);
}

void GameData::AddValue(const std::string & type, const std::string & name, bool asDescriptor)
{
	assert(values.find(name) == values.end() && "The new value must not already exist");

	values[name] = GenerateContainer(type, asDescriptor);
}

void GameData::Set(std::istream& ifs)
{
	std::stringstream f;
	f.clear();
	while (!ifs.eof()) {
		char c;
		ifs.read(&c, 1);
		f << c;
	}
	std::string valname, valtype;
	while (f >> std::ws) {
		if (anonymous) {
			f >> valtype;
		}
		f >> valname;
		if (f.eof()) {
			return; // bad standard C++ design makes code uglyyyy ... and 3 hours in a row debugging for this crap
		}
		if (anonymous && values.find(valname) == values.end()) {
			AddValue(valtype, valname, false);
		}
		assert(values.find(valname) != values.end() && "Value must exist to be set");

		std::stringstream ss;
		f >> std::ws;
		char c;
		f.read(&c, 1);
		assert(c == '=');
		f >> std::ws;
		unsigned int bracketCount = 0;
		bool stringStarted = false;
		do {
			char lastc = c;
			f.read(&c, 1);
			if (c == '"' && lastc != '\\') {
				stringStarted = !stringStarted;
			}
			if (c == '{') {
				if (bracketCount > 0) {
					ss << c;
				}
				++bracketCount;
			}
			else if (c == '}') {
				--bracketCount;
				if (bracketCount > 0) {
					ss << c;
				}
			}
			else {
				ss << c;
			}
		} while (bracketCount || stringStarted);
		values[valname]->Load(ss);
	}
}

void GameData::Set(const std::string & svals)
{
	if (!svals.empty()) {
		std::istringstream ss(svals);
		Set(ss);
	}
}

GameData::ValueContainer::ValueContainer() : type(0), value(0)
{
}

GameData::ValueContainer::ValueContainer(const std::string & type, ValueBase * value) : type(&type), value(value)
{
}

GameData::ValueContainer::ValueContainer(const ValueContainer & rhs) : type(rhs.type), value(rhs.Clone())
{
	*this = rhs;
}

GameData::ValueContainer::~ValueContainer()
{
	assert(type && value);
	delete value;
}

void GameData::ValueContainer::operator=(const ValueContainer & rhs)
{
	assert(type && value);
	assert(GetType() == rhs.GetType());
	std::stringstream ss;
	rhs.Save(ss);
	Load(ss);
}

void * GameData::ValueContainer::GetValue() const
{
	assert(type && value);
	return value->GetValue();
}

GameData::ValueBase * GameData::ValueContainer::Clone() const
{
	assert(type && value);
	return value->Clone();
}

void GameData::ValueContainer::Save(std::ostream & f) const
{
	assert(type && value);
	value->Save(f);
}

void GameData::ValueContainer::Load(std::istream & i)
{
	assert(type && value);
	value->Load(i);
}

const type_info & GameData::ValueContainer::GetType() const
{
	assert(type && value);
	return value->GetType();
}

const std::string & GameData::ValueContainer::Type() const
{
	return *type;
}
