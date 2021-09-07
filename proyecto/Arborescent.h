#ifndef __ARBORESCENT_H__
#define __ARBORESCENT_H__

#include <vector>

template<class T>
class Arborescent {
private:
	std::vector<T*> children;
public:
	void AddChild(T* c, bool onTop = false);
	void RemoveChild(T* c);

	inline int ChildrenSize() const { return (int)children.size(); }
	inline T* GetChild(int i) { return children[i]; }
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
