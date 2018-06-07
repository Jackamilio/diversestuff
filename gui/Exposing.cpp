#include "Exposing.h"

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

void Exposing::Watcher::refreshValueForLabels()
{
	for (std::vector<EditValueArgs*>::iterator it = mValueArgs.begin(); it != mValueArgs.end(); ++it) {
		char* address = (*it)->address->calculateAddress();
		if (address) {
			;//(char*)mWatchedAddress + (*it)->structMember->offset;

			jmg::Base* base = (*it)->field;
			const Type type = (*it)->type;

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
		win->mDeleteMe = true;
		win->close();
	}
}

void editValueCallback(void* a) {
	Exposing::Watcher::EditValueArgs* args = (Exposing::Watcher::EditValueArgs*)a;

	char* address = args->address->calculateAddress();//(char*)args->watcher->mWatchedAddress + args->structMember->offset;//args->watcher->mWatchedStruct.desc[args->id].offset;

	jmg::Base* base = args->field;// args->watcher->mValueFields[args->id];
	jmg::Text* text = dynamic_cast<jmg::Text*>(base);
	const Exposing::Type type = args->type;//args->watcher->mWatchedStruct.desc[args->id].type;
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

Exposing::Watcher::Watcher(StructDescBase* sc, WatchedAddress* wa, int y)
	: mWatchedStruct(sc)
	, mWatchedAddress(wa)
{
	calculatedHeight = 0;
	const int xl = 20;
	const int xr = 150;
	const int yStep = 20;
	StructDescBase::Iterator* it = sc->generateIterator(wa);
	for (; !it->isAtEnd(); it->next()) {
		jmg::Label* nameLabel = new jmg::Label(it->getName().c_str());
		nameLabel->mRelx = xl;
		nameLabel->mRely = y;
		mToDelete.push_back(nameLabel);
		addChild(nameLabel);

		WatchedAddress* address = it->generateWatchedAddress(mWatchedAddress);
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
		
		EditValueArgs* newValueArgs = new EditValueArgs({ address, it->getType(), nullptr });
		mValueArgs.push_back(newValueArgs);

		if (valueField) {
			valueField->mEditCallback = editValueCallback;
			valueField->mEditCallbackArgs = newValueArgs;
			mToDelete.push_back(valueField);
			newValueArgs->field = valueField;
			//addAndAdaptLabel(valueField, xr, y, 10);
			valueField->mWidth = 140;
			addChild(valueField, xr, y);
		}
		else if (checkbox) {
			checkbox->mEditCallback = editValueCallback;
			checkbox->mEditCallbackArgs = newValueArgs;
			mToDelete.push_back(checkbox);
			newValueArgs->field = checkbox;
			addChild(checkbox, xr, y);
		}
		else {
			nameLabel->mRelx += xl;
			StructDescBase* sc_ = registeredTypes[it->getType()];
			Base* value = new Watcher(sc_, newValueArgs->address);
			mToDelete.push_back(value);
			jmg::ShowHide* sh = new jmg::ShowHide();
			mToDelete.push_back(sh);
			addChild(sh, xl, y);
			sh->addChild(value, 0, yStep);
			sh->mOverrideDeltaExpand = value->getHeight();
			y += value->getHeight();
		}

		y += yStep;
	}
	calculatedHeight += y;
}

Exposing::Watcher::~Watcher()
{
	for (unsigned int i = 0; i < (unsigned int)mToDelete.size(); ++i) {
		delete mToDelete[i];
	}
	mToDelete.clear();
	for (unsigned int i = 0; i < (unsigned int)mValueArgs.size(); ++i) {
		if (mValueArgs[i]->address) {
			delete mValueArgs[i]->address;
		}
		delete mValueArgs[i];
	}
	mValueArgs.clear();
	if (dynamic_cast<WatchedAddressRoot*>(mWatchedAddress)) {
		delete mWatchedAddress;
	}
}

int Exposing::Watcher::getHeight() const
{
	return calculatedHeight;
}

Exposing::WatcherWindow::WatcherWindow(StructDescBase* sc, WatchedAddress * wa)
	: InteractiveRectangle(300,100)
	, Window(300,100,sc->name.c_str())
	, Watcher(sc, wa, 20)
{
	mHeight = calculatedHeight;

	mBtnClose.mCallback = closeWatcherCallback;
	mBtnClose.mCallbackArgs = (void*)this;
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
	return new Iterator_(desc);
}

Exposing::StructDesc::Iterator_::Iterator_(const StructInfo & desc)
	: desc(desc)
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

Exposing::WatchedAddress * Exposing::StructDesc::Iterator_::generateWatchedAddress(WatchedAddress* owner)
{
	return new WatchedAddressOffset(owner, desc[index].offset);
}

std::string Exposing::StructDesc::Iterator_::getName() const
{
	return desc[index].name;
}

Exposing::Type Exposing::StructDesc::Iterator_::getType() const
{
	return desc[index].type;
}
