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

void Exposing::WatcherWindow::refreshValueForLabels()
{
	for (unsigned int i = 0; i < (unsigned int)mValueFields.size(); ++i) {
		char* address = (char*)mWatchedAddress + mWatchedStruct.desc[i].offset;

		// expand later, testing int only now
		if (mWatchedStruct.desc[i].type == Exposing::INT) {
			mValueFields[i]->setFrom(*(int*)address);
		}
		else {
			mValueFields[i]->setValue("undef conv");
		}
	}
}

void Exposing::WatcherWindow::draw(int origx, int origy)
{
	refreshValueForLabels();
	jmg::Window::draw(origx, origy);
}

void closeWatcherCallback(void* arg) {
	if (arg) {
		Exposing::WatcherWindow* win = (Exposing::WatcherWindow*)arg;
		win->mDeleteMe = true;
		win->close();
	}
}

void editValueCallback(void* a) {
	Exposing::WatcherWindow::EditValueArgs* args = (Exposing::WatcherWindow::EditValueArgs*)a;

	char* address = (char*)args->window->mWatchedAddress + args->window->mWatchedStruct.desc[args->id].offset;

	//testing int only now
	if (args->window->mWatchedStruct.desc[args->id].type == Exposing::INT) {
		*(int*)address = args->window->mValueFields[args->id]->getAsInt();
	}
}

Exposing::WatcherWindow::WatcherWindow(const StructComplete & sc, void* wa) : jmg::Window(300, 100, sc.name.c_str()), mWatchedStruct(sc), mWatchedAddress(wa)
{
	int y = 20;
	const int xl = 20;
	const int xr = 200;
	const int yStep = 25;
	for (unsigned int i = 0; i < (unsigned int)sc.desc.size(); ++i) {
		jmg::Label* nameLabel = new jmg::Label(sc.desc[i].name.c_str());
		nameLabel->mRelx = xl;
		nameLabel->mRely = y;

		jmg::Numeric* valueField = new jmg::Numeric(9,9,false);
		valueField->mRelx = xr;
		valueField->mRely = y;
		mValueArgs.push_back(new EditValueArgs{this,i});
		valueField->mEditCallback = editValueCallback;
		valueField->mEditCallbackArgs = mValueArgs[i];

		mNameLabels.push_back(nameLabel);
		mValueFields.push_back(valueField);

		addChild(nameLabel);
		addChild(valueField);

		y += yStep;
	}

	mHeight = y + yStep;
	mWidth = 300; //wtf this doesn't work?

	mBtnClose.mCallback = closeWatcherCallback;
	mBtnClose.mCallbackArgs = (void*)this;
}

Exposing::WatcherWindow::~WatcherWindow()
{
	for (unsigned int i = 0; i < (unsigned int)mNameLabels.size(); ++i) {
		delete mNameLabels[i];
	}
	mNameLabels.clear();
	for (unsigned int i = 0; i < (unsigned int)mValueFields.size(); ++i) {
		delete mValueFields[i];
	}
	mValueFields.clear();
	for (unsigned int i = 0; i < (unsigned int)mValueArgs.size(); ++i) {
		delete mValueArgs[i];
	}
	mValueArgs.clear();
}

Exposing::StructComplete::StructComplete() : name("Struct incomplete!!")
{
}

Exposing::StructComplete::StructComplete(const char * n, const Exposing::StructInfo & d) : name(n), desc(d)
{
}
