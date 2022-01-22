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
#include "Arborescent.h"

// forcing the second line to use the static version
// this way the compiler will complain if the user forgets to update this when he renames his class
#define OTN(name) static inline const char* StaticObjectTypeName() { return #name; } \
const char* ObjectTypeName() const { return name::StaticObjectTypeName(); }

class Engine {
public:
	class Object;
private:
	// must be initialized first
	std::set<Object*> objectsTracker;
public:
	// default uber mother class
	friend class Object;
	class Object : public Arborescent<Object> {
	public:
		Engine& engine;

		Object();
		~Object();

		virtual const char* ObjectTypeName() const = 0;
		virtual IMPLEMENT_EXPOSE;
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
	class Node : virtual public Object, public Arborescent<T> {
	private:
		using Object::AddChild;
		using Object::AddChildBefore;
		using Object::AddChildAfter;
		using Object::RemoveChild;
		using Object::HasChild;
		using Object::ChildrenSize;
		using Object::GetChild;
		using Arborescent<T>::AddChild;
		using Arborescent<T>::AddChildBefore;
		using Arborescent<T>::AddChildAfter;
		using Arborescent<T>::RemoveChild;
	public:
		void AddChild(T* c, bool onTop = false);
		void AddChildBefore(T* c, T* target);
		void AddChildAfter(T* c, T* target);
		void RemoveChild(T* c);

		inline bool HasChild(T* c) const { return Arborescent<T>::HasChild(c); }
		inline int ChildrenSize() const { return Arborescent<T>::ChildrenSize(); }
		inline T* GetChild(int i) { return Arborescent<T>::GetChild(i); }
	};
	enum class InputStatus {
		grabbed,
		ignored,
		notforchildren
	};
	class Input : public Node<Input> {
	public:
		static ALLEGRO_MOUSE_STATE mouseState;
		static ALLEGRO_KEYBOARD_STATE keyboardState;
		virtual InputStatus Event(ALLEGRO_EVENT& event) = 0;
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
		virtual void PostStep() {}
	};
	class Graphic : public Node<Graphic> {
	public:
		virtual void Draw() = 0;
		virtual void PostDraw() {}
	};

	// root node classes
	class InputRoot : public Input {
	public:
		OTN(InputRoot);
		InputStatus Event(ALLEGRO_EVENT& event);
	};
	class DynamicRoot : public Dynamic {
	public:
		OTN(DynamicRoot);
		void Tick();
	};
	class UpdateRoot : public Update {
	public:
		OTN(UpdateRoot);
		void Step();
	};
	class GraphicRoot : public Graphic {
	public:
		OTN(GraphicRoot);
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

	// Recursive functions made public if the user wants to bypass them then reuse it
	static bool RecursiveInput(Engine::Input* input, ALLEGRO_EVENT& event, bool doroot = true);
	static void RecursiveUpdate(Engine::Update* update);
	static void RecursiveGraphic(Engine::Graphic* graphic);

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
inline void Engine::Node<T>::AddChild(T* c, bool onTop)
{
	Arborescent<T>::AddChild(c, onTop);
	Object::AddChild((Object*)c);
}

template<class T>
inline void Engine::Node<T>::AddChildBefore(T* c, T* target)
{
	Arborescent<T>::AddChildBefore(c, target);
	Object::AddChildBefore((Object*)c, (Object*)target);
}

template<class T>
inline void Engine::Node<T>::AddChildAfter(T* c, T* target)
{
	Arborescent<T>::AddChildAfter(c, target);
	Object::AddChildAfter((Object*)c, (Object*)target);
}

template<class T>
inline void Engine::Node<T>::RemoveChild(T* c)
{
	Arborescent<T>::RemoveChild(c);
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
