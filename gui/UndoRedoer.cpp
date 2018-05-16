#include "UndoRedoer.h"

UndoRedoer::UndoRedoer()
{
	fromRedo = false;
	grouping = false;
}

UndoRedoer::~UndoRedoer()
{
	Clear();
}

bool UndoRedoer::Undo()
{
	if (!undopile.empty()) {
		int last = (int)undopile.size();

		do {
			--last;
			DoMemBase* mem = undopile.at(last);
			mem->DoCall();
			grouping = mem->donext;
			delete mem;
		} while (grouping);

		undopile.resize(last);
		return true;
	}
	else {
		return false;
	}
}

bool UndoRedoer::Redo()
{
	if (!redopile.empty()) {
		int last = (int)redopile.size();

		fromRedo = true;
		do {
			--last;
			DoMemBase* mem = redopile.at(last);
			mem->DoCall();
			grouping = mem->donext;
			delete mem;
		} while (grouping);
		fromRedo = false;

		redopile.resize(last);
		return true;
	}
	else {
		return false;
	}
}

void UndoRedoer::Clear()
{
	for (unsigned int i = 0; i < redopile.size(); ++i) {
		DoMemBase* mem = redopile[i];
		ReleaseSPtr(mem->args);
		delete mem;
	}
	redopile.clear();
	for (unsigned int i = 0; i < undopile.size(); ++i) {
		DoMemBase* mem = undopile[i];
		ReleaseSPtr(mem->args);
		delete mem;
	}
	undopile.clear();

	/*// First pass to avoid deleting twice because of destructors that could still call ReleaseSPtr
	for (std::map<void*, SPtrBase*>::iterator it = sptrs.begin(); it != sptrs.end(); ++it) {
	(*it).second->count = 9999999;
	}
	for (std::map<void*, SPtrBase*>::iterator it = sptrs.begin(); it != sptrs.end(); ++it) {
	(*it).second->Del();
	delete (*it).second;
	}*/
	std::map<void*, SPtrBase*>::iterator it = sptrs.begin();
	while (it != sptrs.end()) {
		(*it).second->count = 1;
		ReleaseSPtr((*it).first);
		it = sptrs.begin();
	}
	sptrs.clear();
}

void UndoRedoer::StartGroup()
{
	grouping = true;
	groupIndexStart = (int)undopile.size();
}

void UndoRedoer::EndGroup()
{
	grouping = false;
	undopile[groupIndexStart]->donext = false;
}

void UndoRedoer::AddUndo(DoMemBase *dm)
{
	dm->donext = grouping;
	undopile.push_back(dm);
	if (!fromRedo) {
		for (unsigned int i = 0; i < redopile.size(); ++i) {
			DoMemBase* mem = redopile[i];
			ReleaseSPtr(mem->args);
			delete mem;
		}
		redopile.clear();
	}
}

void UndoRedoer::AddRedo(DoMemBase *dm)
{
	dm->donext = grouping;
	redopile.push_back(dm);
}
