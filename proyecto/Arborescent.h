#ifndef __ARBORESCENT_H__
#define __ARBORESCENT_H__

#include <vector>

template<class T>
class Arborescent {
private:
	std::vector<T*> children;
public:
	void AddChild(T* c, bool onTop = false);
	void AddChildBefore(T* c, T* target);
	void AddChildAfter(T* c, T* target);
	void RemoveChild(T* c);

	bool HasChild(T* c) const;

	inline int ChildrenSize() const { return (int)children.size(); }
	inline T* GetChild(int i) { return children[i]; }
	inline const T* GetChild(int i) const { return children[i]; }
};

template<class T>
inline void Arborescent<T>::AddChild(T* c, bool onTop)
{
	if (onTop) {
		children.insert(children.begin(), c);
	}
	else {
		children.push_back(c);
	}
}

template<class T>
inline void Arborescent<T>::AddChildBefore(T* c, T* target)
{
	for (auto it = children.begin(); it != children.end(); ) {
		if (*it == target) {
			children.insert(it, c);
			return;
		}
		else {
			++it;
		}
	}
	AddChild(c);
}

template<class T>
inline void Arborescent<T>::AddChildAfter(T* c, T* target)
{
	for (auto it = children.begin(); it != children.end(); ) {
		if (*it == target) {
			++it;
			if (it != children.end()) {
				children.insert(it, c);
				return;
			}
		}
		else {
			++it;
		}
	}
	AddChild(c);
}

template<class T>
inline bool Arborescent<T>::HasChild(T* c) const
{
	for (auto it = children.begin(); it != children.end(); ) {
		if (*it == c) {
			return true;
		}
		else {
			++it;
		}
	}
	return false;
}

template<class T>
inline void Arborescent<T>::RemoveChild(T* c)
{
	for (auto it = children.begin(); it != children.end(); ) {
		if (*it == c) {
			it = children.erase(it);
		}
		else {
			++it;
		}
	}
}

#endif
