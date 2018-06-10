#include "Exposing.h"
#include <memory>

template<> Exposing::Type Exposing::getType<bool>() { return Exposing::BOOL; }
template<> Exposing::Type Exposing::getType<char>() { return Exposing::INT8; }
template<> Exposing::Type Exposing::getType<unsigned char>() { return Exposing::UINT8; }
template<> Exposing::Type Exposing::getType<short>() { return Exposing::INT16; }
template<> Exposing::Type Exposing::getType<unsigned short>() { return Exposing::UINT16; }
template<> Exposing::Type Exposing::getType<int>() { return Exposing::INT; }
template<> Exposing::Type Exposing::getType<unsigned int>() { return Exposing::UINT; }
template<> Exposing::Type Exposing::getType<float>() { return Exposing::FLOAT; }
template<> Exposing::Type Exposing::getType<double>() { return Exposing::DOUBLE; }
template<> Exposing::Type Exposing::getType<std::string>() { return Exposing::STRING; }

std::map<Exposing::Type, Exposing::StructDescBase*> Exposing::registeredTypes;
unsigned int typeCount = (unsigned int)Exposing::MAX;

Exposing::Type Exposing::registerNewType(const char* name, Exposing::StructDescBase* desc) {
	Exposing::Type newType = (Exposing::Type)++typeCount;
	registeredTypes[newType] = desc;
	return newType;
}

Exposing::Type Exposing::defineStruct(const char * name, const Exposing::StructInfo& members)
{
	return registerNewType(name, new StructDesc(name, members));
}

Exposing::StructMember::StructMember()
	: name("struct has no name")
	, type(Exposing::UNDEF)
	, offset(0)
{
}

Exposing::StructMember::StructMember(const char * n, Type t, unsigned int o)
	: name(n)
	, type(t)
	, offset(o)
{
}

void editValueCallback(void* a) {
	Exposing::Watcher::EditValueArgs* args = (Exposing::Watcher::EditValueArgs*)a;

	char* address = args->address->calculateAddress();

	jmg::Base* base = args->field;
	jmg::Text* text = dynamic_cast<jmg::Text*>(base);
	const Exposing::Type type = args->type;
	if (text) {
		switch (type) {
		case Exposing::INT8:	*(char*)address = text->getAsInt(); break;
		case Exposing::UINT8:	*(unsigned char*)address = text->getAsInt(); break;
		case Exposing::INT16:	*(short*)address = text->getAsInt(); break;
		case Exposing::UINT16:	*(unsigned short*)address = text->getAsInt(); break;
		case Exposing::INT:		*(int*)address = text->getAsInt(); break;
		case Exposing::UINT:	*(unsigned int*)address = text->getAsInt(); break;
		case Exposing::FLOAT:	*(float*)address = text->getAsFloat(); break;
		case Exposing::DOUBLE:	*(double*)address = text->getAsDouble(); break;
		case Exposing::STRING:	*(std::string*)address = text->getValue(); break;
		}
	}
	else if (type == Exposing::BOOL) {
		jmg::CheckBox* checkBox = dynamic_cast<jmg::CheckBox*>(base);
		if (checkBox) {
			*(bool*)address = checkBox->mChecked;
		}
	}
}

//const int yStep = 20;

int Exposing::Watcher::pushNewField(StructDescBase::Iterator * it, int y)
{
	const int xl = 20;
	const int xr = 150;

	jmg::Label* nameLabel = new jmg::Label(it->getName().c_str());
	addChild(nameLabel, BOTTOM, xl);
	y = nameLabel->mRely;

	WatchedAddress* address = it->generateWatchedAddress();
	char* calculatedAddress = address->calculateAddress();
	jmg::Text* valueField = nullptr;
	jmg::CheckBox* checkbox = nullptr;
	switch (it->getType()) {
	case BOOL:	checkbox = new jmg::CheckBox(*(bool*)calculatedAddress); break;
	case INT8:	valueField = new jmg::Numeric(*(char*)calculatedAddress); break;
	case UINT8:	valueField = new jmg::Numeric(*(unsigned char*)calculatedAddress); break;
	case INT16:	valueField = new jmg::Numeric(*(short*)calculatedAddress); break;
	case UINT16:valueField = new jmg::Numeric(*(unsigned short*)calculatedAddress); break;
	case INT:	valueField = new jmg::Numeric(*(int*)calculatedAddress); break;
	case UINT:	valueField = new jmg::Numeric(*(unsigned int*)calculatedAddress); break;
	case FLOAT:	valueField = new jmg::Numeric(*(float*)calculatedAddress); break;
	case DOUBLE:valueField = new jmg::Numeric(*(double*)calculatedAddress); break;
	case STRING:valueField = new jmg::Text(((std::string*)calculatedAddress)->c_str()); break;
	}

	EditValueArgs* newValueArgs = new EditValueArgs(address, it->getType());
	newValueArgs->label = nameLabel;
	mValueArgs.push_back(newValueArgs);

	if (valueField) {
		valueField->mEditCallback = editValueCallback;
		valueField->mEditCallbackArgs = newValueArgs;
		newValueArgs->field = valueField;
		valueField->mWidth = 140;
		addChild(valueField, xr, y);
		//addChild(valueField, Base::BOTTOM, xr);
	}
	else if (checkbox) {
		checkbox->mEditCallback = editValueCallback;
		checkbox->mEditCallbackArgs = newValueArgs;
		newValueArgs->field = checkbox;
		addChild(checkbox, xr, y);
		//addChild(checkbox, Base::BOTTOM, xr);
	}
	else {
		nameLabel->mRelx += xl;
		StructDescBase* sc_ = registeredTypes[it->getType()];
		Base* value = new Watcher(sc_, newValueArgs->address);
		newValueArgs->field = value;
		jmg::ShowHide* sh = new jmg::ShowHide();
		newValueArgs->sh = sh;
		//sh->addChild(value, 0, yStep);
		addChild(value, BOTTOM, xl);
		sh->mShowHideObject = value;
		//sh->addChild(value, Base::BOTTOM, 0);
		//sh->mOverrideDeltaExpand = value->getHeight();
		addChild(sh, xl, y);
		//addChild(sh, Base::BOTTOM, xl);
		//return value->getHeight() +yStep;
	}
	return 0;// yStep;
}

void Exposing::Watcher::refreshValueForLabels()
{
	std::unique_ptr<StructDescBase::Iterator> structIt(mWatchedStruct->generateIterator(mWatchedAddress));
	std::vector<EditValueArgs*>::iterator argsIt = mValueArgs.begin();
	while (!structIt->isAtEnd() && argsIt != mValueArgs.end()) {
		char* address = (*argsIt)->address->calculateAddress();

		// check if we're watching the same thing
		// update name label if not
		if (address != std::unique_ptr<WatchedAddress>(structIt->generateWatchedAddress())->calculateAddress()) {
			(*argsIt)->label->setValue(structIt->getName().c_str());
		}

		// update the field
		if (address) {
			jmg::Base* base = (*argsIt)->field;
			const Type type = (*argsIt)->type;

			if (type == Exposing::BOOL) {
				dynamic_cast<jmg::CheckBox*>(base)->mChecked = *(bool*)address;
			}
			else {
				jmg::Text* text = dynamic_cast<jmg::Text*>(base);
				if (text && !text->isEditing()) {
					if (type == Exposing::INT8) {
						text->setFrom(*(char*)address);
					}
					else if (type == Exposing::UINT8) {
						text->setFrom(*(unsigned char*)address);
					}
					else if (type == Exposing::INT16) {
						text->setFrom(*(short*)address);
					}
					else if (type == Exposing::UINT16) {
						text->setFrom(*(unsigned short*)address);
					}
					else if (type == Exposing::INT) {
						text->setFrom(*(int*)address);
					}
					else if (type == Exposing::UINT) {
						text->setFrom(*(unsigned int*)address);
					}
					else if (type == Exposing::FLOAT) {
						text->setFrom(*(float*)address);
					}
					else if (type == Exposing::DOUBLE) {
						text->setFrom(*(double*)address);
					}
					else if (type == Exposing::STRING) {
						text->setValue(((std::string*)address)->c_str());
					}
					else {
						text->setValue("undef conv");
					}
				}
			}
		}

		//iterate!
		if (argsIt != mValueArgs.end()) {
			++argsIt;
		}
		if (!structIt->isAtEnd()) {
			structIt->next();
		}
	}

	if (argsIt != mValueArgs.end()) {
		//delete the surplus of args
		int count = 0;
		for (; argsIt != mValueArgs.end();++argsIt) {
			calculatedHeight -= (*argsIt)->field->getHeight();
			(*argsIt)->field->remove(true);
			(*argsIt)->field = nullptr;
			(*argsIt)->label->remove(true);
			(*argsIt)->label = nullptr;
			if ((*argsIt)->sh) {
				(*argsIt)->sh->remove(true);
				(*argsIt)->sh = nullptr;
			}
			delete *argsIt;
			++count;
		}
		if (count > 0) {
			mValueArgs.erase(mValueArgs.begin() + (mValueArgs.size() - count), mValueArgs.end());
		}
	}
	else {
		//add more fields if necessary
		for (; !structIt->isAtEnd(); structIt->next()) {
			calculatedHeight += pushNewField(structIt.get(), calculatedHeight);
		}
	}
}

void Exposing::Watcher::draw(int origx, int origy)
{
	refreshValueForLabels();
	jmg::Base::draw(origx, origy);
}

void closeWatcherCallback(void* arg) {
	if (arg) {
		Exposing::WatcherWindow* win = (Exposing::WatcherWindow*)arg;
		win->remove(true);
		win->close();
	}
}

Exposing::Watcher::Watcher(StructDescBase* sc, WatchedAddress* wa, int y)
	: mWatchedStruct(sc)
	, mWatchedAddress(wa)
{
	calculatedHeight = y;
	for (std::unique_ptr<StructDescBase::Iterator> it(sc->generateIterator(wa)); !it->isAtEnd(); it->next()) {
		calculatedHeight += pushNewField(it.get(), calculatedHeight);
	}
}

Exposing::Watcher::~Watcher()
{
	for (auto ptr : mValueArgs) {
		delete ptr;
	}
	mValueArgs.clear();
}

int Exposing::Watcher::getHeight() const
{
	//return calculatedHeight;
	return getEdge(BOTTOM);
}

Exposing::WatcherWindow::WatcherWindow(StructDescBase* sc, WatchedAddress * wa)
	: InteractiveRectangle(300, 100)
	, Window(300, 100, sc->name.c_str())
	, Watcher(sc, wa, 20)
{
	//mHeight = calculatedHeight;
	mHeight = getEdge(BOTTOM);

	mBtnClose.mCallback = closeWatcherCallback;
	mBtnClose.mCallbackArgs = (void*)this;
}

Exposing::WatcherWindow::~WatcherWindow()
{
	// the window is responsible for the memory management of the address, and this should be a root version
	delete mWatchedAddress;
}

void Exposing::WatcherWindow::draw(int origx, int origy)
{
	refreshValueForLabels();
	Window::draw(origx, origy);
}

int Exposing::WatcherWindow::getHeight() const
{
	return Window::getHeight();
}

Exposing::StructDescBase::StructDescBase(const char * n) : name(n)
{
}

Exposing::StructDesc::StructDesc(const char * n, const StructInfo & d)
	: StructDescBase(n)
	, desc(d)
{
}

Exposing::StructDescBase::Iterator * Exposing::StructDesc::generateIterator(WatchedAddress* from)
{
	return new Iterator_(desc, from);
}

Exposing::StructDesc::Iterator_::Iterator_(const StructInfo & desc, WatchedAddress * watchedStruct)
	: desc(desc)
	, watchedStruct(watchedStruct)
	, index(0)
{
}

void Exposing::StructDesc::Iterator_::next()
{
	++index;
}

bool Exposing::StructDesc::Iterator_::isAtEnd() const
{
	return index >= (int)desc.size();
}

Exposing::WatchedAddress * Exposing::StructDesc::Iterator_::generateWatchedAddress()
{
	return new WatchedAddressOffset(watchedStruct, desc[index].offset);
}

std::string Exposing::StructDesc::Iterator_::getName() const
{
	return desc[index].name;
}

Exposing::Type Exposing::StructDesc::Iterator_::getType() const
{
	return desc[index].type;
}

Exposing::Watcher::EditValueArgs::EditValueArgs(WatchedAddress* address, Type type)
	: address(address)
	, type(type)
	, field(nullptr)
	, label(nullptr)
	, sh(nullptr)
{
}

Exposing::Watcher::EditValueArgs::~EditValueArgs()
{
	// omg just learned that it was safe to delete null pointers!
	delete address;
	delete field;
	delete label;
	delete sh;
}
