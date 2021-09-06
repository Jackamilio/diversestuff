#ifndef ___ENGINE_H___
#define ___ENGINE_H___

#include <allegro5/allegro.h>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <bullet/btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include "GraphicContext.h"
#include "Shader.h"
#include "Exposing.h"

#define OTN(name) const char* ObjectTypeName() const { return #name; }

class Engine {
private:
	// must be initialized first
	class Object;
	std::set<Object*> objectsTracker;
public:
	// default uber mother class
	friend class Object;
	class Object {
	public:
		Engine& engine;

		Object();
		~Object();

		virtual const char* ObjectTypeName() const = 0;

		void AddChild(Object* c, bool onTop = false);
		void RemoveChild(Object* c);

		inline int ChildrenSize() const { return children.size(); }
		inline Object* GetChild(int i) { return children[i]; }
		virtual IMPLEMENT_EXPOSE;

	private:
		std::vector<Object*> children;
	};

private:
	class RootObject : public Object {
		OTN(RootObject);
	};
	RootObject rootObject;

public:
	static Engine& Get();

	Engine();
	~Engine();

	bool Init();
	bool OneLoop();

	// mother node classes
	template<class T>
	class Node : virtual public Object {
	private:
		using Object::AddChild;
		using Object::RemoveChild;
		using Object::ChildrenSize;
		using Object::GetChild;

		std::vector<T*> children;
	public:
		void AddChild(T* c, bool onTop = false);
		void RemoveChild(T* c);

		inline int ChildrenSize() const { return children.size(); }
		inline T* GetChild(int i) { return children[i]; }
	};
	class Input : public Node<Input> {
	public:
		static ALLEGRO_MOUSE_STATE mouseState;
		static ALLEGRO_KEYBOARD_STATE keyboardState;
		virtual bool Event(ALLEGRO_EVENT& event) = 0;
	};
	class Dynamic : public Node<Dynamic> {
	public:
		virtual void Tick() = 0;
		virtual void Collision(Dynamic* other, btPersistentManifold& manifold);
		void ReactToCollisionFrom(btRigidBody& body);
	};
	class Update : public Node<Update> {
	public:
		virtual void Step() = 0;
	};
	class Graphic : public Node<Graphic> {
	public:
		virtual void Draw() = 0;
	};

	// root node classes
	class InputRoot : public Input {
	public:
		OTN(InputRoot)
		bool Event(ALLEGRO_EVENT& event);
	};
	class DynamicRoot : public Dynamic {
	public:
		OTN(DynamicRoot)
		void Tick();
	};
	class UpdateRoot : public Update {
	public:
		OTN(UpdateRoot)
		void Step();
	};
	class GraphicRoot : public Graphic {
	public:
		OTN(GraphicRoot)
		void Draw();
	};
	class GraphicTarget : public Graphic {
	private:
		bool ownsBitmap;
	public:
		OTN(GraphicTarget);
		IMPLEMENT_EXPOSE;

		ALLEGRO_BITMAP* bitmap;
		glm::vec4 clearColor;

		GraphicTarget(ALLEGRO_BITMAP* bitmap = nullptr);
		GraphicTarget(int width, int height, bool depth = false);
		~GraphicTarget();
		void Draw();
	};

	// graphic node classes
	class ShaderGraphic : public Graphic {
	public:
		std::map<Program*, GraphicRoot> programChildren;
		ShaderGraphic();

		void Draw();
		void AddChildToProgram(Graphic* child, const std::string& progFile);
		void RemoveChildFromProgram(Graphic* child, const std::string& progFile);
	};
	class MainGraphic : public ShaderGraphic {
	public:
		OTN(MainGraphic)
		MainGraphic();
		void Draw();
	};
	class DebugGraphic : public ShaderGraphic {
	public:
		OTN(DebugGraphic)
		DebugGraphic();

		void Draw();
	};
	class OverlayGraphic : public ShaderGraphic {
	public:
		OTN(OverlayGraphic)
		OverlayGraphic();
		void Draw();
	};

	// helper classes
	class DoubleGraphic : public Engine::Graphic {
	private:
		class SecondGraphic : public Engine::Graphic {
		public:
			OTN(SecondGraphic)
			DoubleGraphic& dg;
			SecondGraphic(DoubleGraphic& dg) : dg(dg) {}
			void Draw() { dg.SecondDraw(); }
		};
		SecondGraphic sg;
	public:
		DoubleGraphic() : sg(*this) {}
		inline Engine::Graphic* GetSecondGraphic() { return (Engine::Graphic*)&sg; }
		virtual void SecondDraw() = 0;
	};

	class TripleGraphic : public DoubleGraphic {
	private:
		class ThirdGraphic : public Engine::Graphic {
		public:
			OTN(ThirdGraphic)
			TripleGraphic& trg;
			ThirdGraphic(TripleGraphic& trg) : trg(trg) {}
			void Draw() { trg.ThirdDraw(); }
		};
		ThirdGraphic thg;
	public:
		TripleGraphic() : thg(*this) {}
		inline Engine::Graphic* GetThirdGraphic() { return (Engine::Graphic*)&thg; }
		virtual void ThirdDraw() = 0;
	};

	// engine node instances
	InputRoot inputRoot;
	DynamicRoot dynamicRoot;
	UpdateRoot updateRoot;
	GraphicRoot graphicTargets;
	GraphicTarget defaultGraphicTarget;
	MainGraphic mainGraphic;
	DebugGraphic debugGraphic;
	OverlayGraphic overlayGraphic;

	// engine variables
	double lastTime;
	double time;
	double dt;
	double dtTarget;

	ALLEGRO_DISPLAY* display;
	GraphicContext graphics;
	std::string currentDirectory;

	btDiscreteDynamicsWorld* physics;

	template<class T>
	T& Access();
private:
	btDefaultCollisionConfiguration* collisionConfig;
	btCollisionDispatcher* dispatcher;
	btBroadphaseInterface* overlappingPairCache;
	btSequentialImpulseConstraintSolver* solver;
	ALLEGRO_EVENT_QUEUE* eventQueue;
	bool initSuccess;

	bool guiEnabled;
	void ShowGui();

	class Data {
	public:
		virtual ~Data() {};
	};
	template<class T>
	class TData : public Data {
	public:
		T* data;
		TData();
		~TData();
	};
	std::map<const type_info*, Data*> userData;
};

template<class T>
inline void Engine::Node<T>::AddChild(T * c, bool onTop)
{
	if (onTop) {
		children.insert(children.begin(), c);
	}
	else {
		children.push_back(c);
	}
	Object::AddChild((Object*)c);
}

template<class T>
inline void Engine::Node<T>::RemoveChild(T* c)
{
	for (auto it = children.begin(); it != children.end(); ) {
		if (*it  == c) {
			it = children.erase(it);
		}
		else {
			++it;
		}
	}
	Object::RemoveChild((Object*)c);
}

template<class T>
inline T& Engine::Access()
{
	TData<T>*& usr = (TData<T>*&)userData[&typeid(T)];
	if (!usr) {
		usr = new TData<T>();
	}
	return *usr->data;
}

template<class T>
inline Engine::TData<T>::TData()
{
	data = new T();
}

template<class T>
inline Engine::TData<T>::~TData()
{
	delete data;
}

#endif//___ENGINE_H___
