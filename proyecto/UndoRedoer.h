#ifndef ___UNDO_REDOER_H___
#define ___UNDO_REDOER_H___

#include <vector>
#include <map>
#include <type_traits>

class UndoRedoer {
public:

	UndoRedoer();
	~UndoRedoer();

	bool Undo();
	bool Redo();

	void Clear();

	void StartGroup();
	void EndGroup();

	template<class T>
	bool RemoveThing(std::vector<T>& vec, const int index, bool managePtr = false);

	template<class K, class T>
	bool RemoveThing(std::map<K, T>& map, const K& key, bool managePtr = false);

	template<class T>
	void AddThing(std::vector<T>& vec, T elem, bool managePtr = false);

	template<class K, class T>
	void AddThing(std::map<K, T>& map, const K& key, T elem, bool managePtr = false);

	template<class T>
	void UpdateThing(T& thing, const T& val);

	template<class T>
	T* UseSPtr(T* ptr);

	template<class T>
	void ReleaseSPtr(T* ptr);

private:
	// undo redo stuff
	class DoMemBase {
	public:
		DoMemBase(void* a) : args(a), donext(false) {}
		virtual void DoCall() = 0;
		void* args;
		bool donext;
	};

	template<class T>
	class DoMem : public DoMemBase {
	public:
		DoMem() : caller(0), fonc(0), args(0) {};
		DoMem(T* c, void(T::*f)(void*), void*a) : caller(c), fonc(f), DoMemBase(a) {}
		T* caller;
		void (T::*fonc)(void*);
		void DoCall();
	};

	std::vector<DoMemBase*> undopile;
	std::vector<DoMemBase*> redopile;

	void AddUndo(DoMemBase*);
	void AddRedo(DoMemBase*);
	bool fromRedo;
	bool grouping;
	unsigned int groupIndexStart;

	class SPtrBase {
	public:
		int count{};
		virtual void Del() = 0;
	};

	template<class T>
	class SPtr : public SPtrBase {
	public:
		SPtr();
		SPtr(T*);
		T* ptr;
		void Del();
	};

	std::map<void*, SPtrBase*> sptrs;

	template<class T>
	class Indexed {
	public:
		std::vector<T>& vector;
		int index;
		T mem;
		bool managePtr;
		UndoRedoer& parent;
		//T object;
		//Indexed(std::vector<T*>& v, int i, const T& o) : vector(v), index(i), object(o) {}
		Indexed(std::vector<T>& v, int i, bool managePtr, UndoRedoer& ur);
		~Indexed();
	};

	template<class K, class T>
	class Mapped {
	public:
		std::map<K, T>& map;
		K key;
		T mem;
		bool managePtr;
		UndoRedoer& parent;
		Mapped(std::map<K, T>& m, K k, bool managePtr, UndoRedoer& ur);
		~Mapped();
	};

	template<class T>
	class MemThing {
	public:
		T& thing;
		T val;
		MemThing(T& t, const T& v) : thing(t), val(v) {}
		void swap() {
			T mem(thing);
			thing = val;
			val = mem;
		}
	};

	// templated remove
	template<class  T>
	void UndoRemoveThing(void*);
	template<class  T>
	void RedoRemoveThing(void*);
	template<class K, class T>
	void UndoRemoveMappedThing(void *);
	template<class K, class T>
	void RedoRemoveMappedThing(void *);

	// templated add
	template<class  T>
	void UndoAddThing(void*);
	template<class  T>
	void RedoAddThing(void*);
	template<class K, class T>
	void UndoAddMappedThing(void *);
	template<class K, class T>
	void RedoAddMappedThing(void *);

	// templated update
	template<class T>
	void UndoUpdateThing(void*);
	template<class T>
	void RedoUpdateThing(void*);
	// bis
	/*template<class T>
	bool UpdateThingInVector(std::vector<T>& vec, const int index, const T& val);
	template<class T>
	void UndoUpdateThingInVector(void*);
	template<class T>
	void RedoUpdateThingInVector(void*);*/
};

template<class T>
inline T* UndoRedoer::UseSPtr(T * ptr)
{
	std::map<void*, SPtrBase*>::iterator it = sptrs.find((void*)ptr);
	if (it == sptrs.end()) {
		sptrs[(void*)ptr] = new SPtr<T>(ptr);
	}
	++sptrs[(void*)ptr]->count;
	return ptr;
}

template<class T>
inline void UndoRedoer::ReleaseSPtr(T * ptr)
{
	std::map<void*, SPtrBase*>::iterator it = sptrs.find((void*)ptr);
	if (it != sptrs.end()) {
		SPtrBase* sptr = sptrs[(void*)ptr];
		--sptr->count;
		if (sptr->count <= 0) {
			sptr->Del();
			sptrs.erase(it);
			delete sptr;
		}
	}
}

template<class T>
inline bool UndoRedoer::RemoveThing(std::vector<T>& vec, const int index, bool managePtr)
{
	if (index >= 0 && index < (int)vec.size()) {
		AddUndo(new DoMem<UndoRedoer>(this, &UndoRedoer::UndoRemoveThing<T>, (void*)UseSPtr(new Indexed<T>(vec, index, managePtr, *this))));
		vec.erase(vec.begin() + index);
		return true;
	}
	else {
		return false;
	}
}

template<class K, class T>
inline bool UndoRedoer::RemoveThing(std::map<K, T>& map, const K & key, bool managePtr)
{
	typename std::map<K, T>::iterator it = map.find(key);
	if (it != map.end()) {
		AddUndo(new DoMem<UndoRedoer>(this, &UndoRedoer::UndoRemoveMappedThing<K,T>, (void*)UseSPtr(new Mapped<K,T>(map, key, managePtr, *this))));
		map.erase(it);
		return true;
	}
	else {
		return false;
	}
}

template<class T>
inline void UndoRedoer::UndoRemoveThing(void * arg)
{
	Indexed<T>* idxd = (Indexed<T>*)arg;
	idxd->vector.insert(idxd->vector.begin() + idxd->index, idxd->mem);
	AddRedo(new DoMem<UndoRedoer>(this, &UndoRedoer::RedoRemoveThing<T>, (void*)idxd));
}

template<class T>
inline void UndoRedoer::RedoRemoveThing(void * arg)
{
	Indexed<T>* idxd = (Indexed<T>*)arg;
	RemoveThing<T>(idxd->vector, (int)idxd->index, idxd->managePtr);
	ReleaseSPtr(idxd);
}

template<class K, class T>
inline void UndoRedoer::UndoRemoveMappedThing(void * arg)
{
	Mapped<K,T>* mpd = (Mapped<K, T>*)arg;
	mpd->map[mpd->key] = mpd->mem;
	AddRedo(new DoMem<UndoRedoer>(this, &UndoRedoer::RedoRemoveMappedThing<K,T>, (void*)mpd));
}

template<class K, class T>
inline void UndoRedoer::RedoRemoveMappedThing(void * arg)
{
	Mapped<K, T>* mpd = (Mapped<K, T>*)arg;
	RemoveThing<K, T>(mpd->map, mpd->key, mpd->managePtr);
	ReleaseSPtr(mpd);
}

template<class T>
inline void UndoRedoer::AddThing(std::vector<T>& vec, T elem, bool managePtr)
{
	vec.push_back(elem);
	AddUndo(new DoMem<UndoRedoer>(this, &UndoRedoer::UndoAddThing<T>, UseSPtr(new Indexed<T>(vec, (int)vec.size() - 1, managePtr, *this))));
}

template<class K, class T>
inline void UndoRedoer::AddThing(std::map<K, T>& map, const K & key, T elem, bool managePtr)
{
	map[key] = elem;
	AddUndo(new DoMem<UndoRedoer>(this, &UndoRedoer::UndoAddMappedThing<K,T>, UseSPtr(new Mapped<K,T>(map, key, managePtr, *this))));
}

template<class T>
inline void UndoRedoer::UndoAddThing(void *arg)
{
	Indexed<T>* idxd = (Indexed<T>*)arg;
	idxd->vector.resize(idxd->vector.size() - 1);
	AddRedo(new DoMem<UndoRedoer>(this, &UndoRedoer::RedoAddThing<T>, arg));
}

template<class T>
inline void UndoRedoer::RedoAddThing(void *arg)
{
	Indexed<T>* idxd = (Indexed<T>*)arg;
	AddThing<T>(idxd->vector, idxd->mem, idxd->managePtr);
	ReleaseSPtr(idxd);
}

template<class K, class T>
inline void UndoRedoer::UndoAddMappedThing(void * arg)
{
	Mapped<K, T>* mpd = (Mapped<K, T>*)arg;
	mpd->map.erase(mpd->map.find(mpd->key));
	AddRedo(new DoMem<UndoRedoer>(this, &UndoRedoer::RedoAddMappedThing<K,T>, arg));
}

template<class K, class T>
inline void UndoRedoer::RedoAddMappedThing(void * arg)
{
	Mapped<K, T>* mpd = (Mapped<K, T>*)arg;
	AddThing<K, T>(mpd->map, mpd->key, mpd->mem, mpd->managePtr);
	ReleaseSPtr(mpd);
}

template<class T>
inline void UndoRedoer::UpdateThing(T& thing, const T & val)
{
	MemThing<T>* mt = UseSPtr(new MemThing<T>(thing, val));
	mt->swap();
	AddUndo(new DoMem<UndoRedoer>(this, &UndoRedoer::UndoUpdateThing<T>, (void*)mt));
}

template<class T>
inline void UndoRedoer::UndoUpdateThing(void *arg)
{
	MemThing<T>* mt = (MemThing<T>*)arg;
	mt->swap();
	AddRedo(new DoMem<UndoRedoer>(this, &UndoRedoer::RedoUpdateThing<T>, (void*)mt));
}

template<class T>
inline void UndoRedoer::RedoUpdateThing(void *arg)
{
	MemThing<T>* mt = (MemThing<T>*)arg;
	UpdateThing(mt->thing, mt->val);
	ReleaseSPtr(mt);
}

/*template<class T>
inline bool UndoRedoer::UpdateThingInVector(std::vector<T>& vec, const int index, const T & val)
{
if (index >= 0 && index < (int)vec.size()) {
AddUndo(new DoMem<UndoRedoer>(this, &UndoRedoer::UndoUpdateThing<T>, UseSPtr(new Indexed<T>(vec, index, vec[index]))));

tilesets[index] = val;

return true;
}
else {
return false;
}
}

template<class T>
inline void UndoRedoer::UndoUpdateThingInVector(void *arg)
{
Indexed<T>* idxd = (Indexed<T>*)arg;
T swap(idxd->vector[idxd->index]);
idxd->vector[idxd->index] = idxd->object;
idxd->object = swap;
AddRedo(new DoMem<UndoRedoer>(this, &UndoRedoer::RedoUpdateThing<T>, arg));
}

template<class T>
inline void UndoRedoer::RedoUpdateThingInVector(void *arg)
{
Indexed<T>* idxd = (Indexed<T>*)arg;
UpdateThing<T>(idxd->vector, idxd->index, idxd->object);
ReleaseSPtr(idxd);
}*/

template<class T>
inline UndoRedoer::SPtr<T>::SPtr() : ptr(0) {}

template<class T>
inline UndoRedoer::SPtr<T>::SPtr(T *p) : ptr(p) {}

template<class T>
inline void UndoRedoer::SPtr<T>::Del()
{
	if (ptr) {
		delete ptr;
		ptr = 0;
	}
}

template<class T>
inline void UndoRedoer::DoMem<T>::DoCall()
{
	if (caller && fonc) {
		(caller->*fonc)(args);
	}
}

template<class T>
inline UndoRedoer::Indexed<T>::Indexed(std::vector<T>& v, int i, bool mp, UndoRedoer & ur) : vector(v), index(i), mem(v[i]), managePtr(mp), parent(ur)
{
	if (managePtr && std::is_pointer<T>::value) {
		parent.UseSPtr(mem);
	}
}


template<class T>
inline UndoRedoer::Indexed<T>::~Indexed()
{
	if (managePtr && std::is_pointer<T>::value) {
		parent.ReleaseSPtr(mem);
	}
}

template<class K, class T>
inline UndoRedoer::Mapped<K, T>::Mapped(std::map<K, T>& m, K k, bool mp, UndoRedoer & ur) : map(m), key(k), mem(m[k]), managePtr(mp), parent(ur)
{
	if (managePtr && std::is_pointer<T>::value) {
		parent.UseSPtr(mem);
	}
}

template<class K, class T>
inline UndoRedoer::Mapped<K, T>::~Mapped()
{
	if (managePtr && std::is_pointer<T>::value) {
		parent.ReleaseSPtr(mem);
	}
}

#endif//___UNDO_REDOER_H___
