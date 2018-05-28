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

std::map<Exposing::Type, Exposing::StructComplete> Exposing::registeredTypes;
unsigned int typeCount = (unsigned int)Exposing::MAX;

Exposing::Type Exposing::defineStruct(const char * name, const Exposing::StructInfo& members, unsigned int structSize)
{
	Exposing::Type newType = (Exposing::Type)++typeCount;
	registeredTypes[newType] = StructComplete(name, members);
	return newType;
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
	for (unsigned int i = 0; i < (unsigned int)mValueFields.size(); ++i) {
		char* address = (char*)mWatchedAddress + mWatchedStruct.desc[i].offset;

		jmg::Base* base = mValueFields[i];

		if (mWatchedStruct.desc[i].type == Exposing::BOOL) {
			dynamic_cast<jmg::CheckBox*>(base)->mChecked = *(bool*)address;
		}
		else {
			jmg::Text* text = dynamic_cast<jmg::Text*>(base);
			if (text && !text->isEditing()) {
				if (mWatchedStruct.desc[i].type == Exposing::INT8) {
					text->setFrom(*(char*)address);
				}
				else if (mWatchedStruct.desc[i].type == Exposing::UINT8) {
					text->setFrom(*(unsigned char*)address);
				}
				else if (mWatchedStruct.desc[i].type == Exposing::INT16) {
					text->setFrom(*(short*)address);
				}
				else if (mWatchedStruct.desc[i].type == Exposing::UINT16) {
					text->setFrom(*(unsigned short*)address);
				}
				else if (mWatchedStruct.desc[i].type == Exposing::INT) {
					text->setFrom(*(int*)address);
				}
				else if (mWatchedStruct.desc[i].type == Exposing::UINT) {
					text->setFrom(*(unsigned int*)address);
				}
				else if (mWatchedStruct.desc[i].type == Exposing::FLOAT) {
					text->setFrom(*(float*)address);
				}
				else if (mWatchedStruct.desc[i].type == Exposing::DOUBLE) {
					text->setFrom(*(double*)address);
				}
				else if (mWatchedStruct.desc[i].type == Exposing::STRING) {
					text->setValue(((std::string*)address)->c_str());
				}
				else {
					text->setValue("undef conv");
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

	char* address = (char*)args->watcher->mWatchedAddress + args->watcher->mWatchedStruct.desc[args->id].offset;

	jmg::Base* base = args->watcher->mValueFields[args->id];
	jmg::Text* text = dynamic_cast<jmg::Text*>(base);
	const Exposing::Type type = args->watcher->mWatchedStruct.desc[args->id].type;
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

Exposing::Watcher::Watcher(const StructComplete & sc, void* wa, int y)
	: mWatchedStruct(sc)
	, mWatchedAddress(wa)
{
	calculatedHeight = 0;
	const int xl = 20;
	const int xr = 150;
	const int yStep = 20;
	for (unsigned int i = 0; i < (unsigned int)sc.desc.size(); ++i) {
		jmg::Label* nameLabel = new jmg::Label(sc.desc[i].name.c_str());
		nameLabel->mRelx = xl;
		nameLabel->mRely = y;
		mToDelete.push_back(nameLabel);
		addChild(nameLabel);

		char* address = (char*)mWatchedAddress + mWatchedStruct.desc[i].offset;
		jmg::Text* valueField = nullptr;
		jmg::CheckBox* checkbox = nullptr;
		switch (sc.desc[i].type) {
		case BOOL:	checkbox = new jmg::CheckBox(*(bool*)address); break;
		case INT8:	valueField = new jmg::Numeric(*(char*)address); break;
		case UINT8:	valueField = new jmg::Numeric(*(unsigned char*)address); break;
		case INT16:	valueField = new jmg::Numeric(*(short*)address); break;
		case UINT16:valueField = new jmg::Numeric(*(unsigned short*)address); break;
		case INT:	valueField = new jmg::Numeric(*(int*)address); break;
		case UINT:	valueField = new jmg::Numeric(*(unsigned int*)address); break;
		case FLOAT:	valueField = new jmg::Numeric(*(float*)address); break;
		case DOUBLE:valueField = new jmg::Numeric(*(double*)address); break;
		case STRING:valueField = new jmg::Text(((std::string*)address)->c_str()); break;
		}
		
		mValueArgs.push_back(new EditValueArgs({ this,i }));

		if (valueField) {
			valueField->mEditCallback = editValueCallback;
			valueField->mEditCallbackArgs = mValueArgs[i];
			mValueFields.push_back(valueField);
			//addAndAdaptLabel(valueField, xr, y, 10);
			valueField->mWidth = 140;
			addChild(valueField, xr, y);
		}
		else if (checkbox) {
			checkbox->mEditCallback = editValueCallback;
			checkbox->mEditCallbackArgs = mValueArgs[i];
			mValueFields.push_back(checkbox);
			addChild(checkbox, xr, y);
		}
		else {
			nameLabel->mRelx += xl;
			const StructComplete& sc_ = registeredTypes[sc.desc[i].type];
			Base* value = new Watcher(sc_, address);
			mValueFields.push_back(value);
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
	for (unsigned int i = 0; i < (unsigned int)mValueFields.size(); ++i) {
		delete mValueFields[i];
	}
	mValueFields.clear();
	for (unsigned int i = 0; i < (unsigned int)mValueArgs.size(); ++i) {
		delete mValueArgs[i];
	}
	mValueArgs.clear();
}

int Exposing::Watcher::getHeight() const
{
	return calculatedHeight;
}

Exposing::WatcherWindow::WatcherWindow(const StructComplete & sc, void * wa)
	: InteractiveRectangle(300,100)
	, Window(300,100,sc.name.c_str())
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

Exposing::StructComplete::StructComplete() : name("Struct incomplete!!")
{
}

Exposing::StructComplete::StructComplete(const char * n, const Exposing::StructInfo & d) : name(n), desc(d)
{
}
