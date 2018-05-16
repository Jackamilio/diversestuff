#ifndef ___ENGINE_H___
#define ___ENGINE_H___

#include <allegro5/allegro.h>
#include <vector>
#include <map>
#include <string>
#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <typeinfo>
#include "GraphicContext.h"
#include "Shader.h"

class Engine {
public:
	Engine();
	~Engine();

	bool Init();
	bool OneLoop();

	// mother node classes
	template<class T>
	class Node {
	public:
		std::vector<T*> children;
		void AddChild(T* c);
	};
	class Input : public Node<Input> {
	public:
		virtual void Event(ALLEGRO_EVENT& event) = 0;
		bool stopChild;
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
		void Event(ALLEGRO_EVENT& event);
		inline bool ContinueLoop() { return !stopChild; }
	};
	class DynamicRoot : public Dynamic {
	public:
		void Tick();
	};
	class UpdateRoot : public Update {
	public:
		void Step();
	};
	class GraphicRoot : public Graphic {
	public:
		void Draw();
	};

	// graphic node classes
	class ShaderGraphic : public Graphic {
	public:
		std::map<Program*, GraphicRoot> programChildren;
		GraphicContext& graphics;
		ShaderGraphic(GraphicContext& g);

		void Draw();
		void AddChildForProgram(Graphic* child, const std::string& progFile);
	};
	class MainGraphic : public ShaderGraphic {
	public:
		Engine& engine;

		MainGraphic(Engine& e);
		void Draw();
	};
	class DebugGraphic : public ShaderGraphic {
	public:
		DebugGraphic(GraphicContext& g);

		void Draw();
	};
	class OverlayGraphic : public ShaderGraphic {
	public:
		Engine& engine;
		OverlayGraphic(Engine& e);
		void Draw();
	};

	// helper classes
	class DoubleGraphic : public Engine::Graphic {
	private:
		class SecondGraphic : public Engine::Graphic {
		public:
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
			TripleGraphic& trg;
			ThirdGraphic(DoubleGraphic& tg) : trg(trg) {}
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
	GraphicRoot graphicRoot;
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

	btDiscreteDynamicsWorld* physics;;

	template<class T>
	T& Get();
private:
	btDefaultCollisionConfiguration* collisionConfig;
	btCollisionDispatcher* dispatcher;
	btBroadphaseInterface* overlappingPairCache;
	btSequentialImpulseConstraintSolver* solver;
	ALLEGRO_EVENT_QUEUE* eventQueue;
	bool initSuccess;

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
inline void Engine::Node<T>::AddChild(T * c)
{
	children.push_back(c);
}

template<class T>
inline T& Engine::Get()
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
