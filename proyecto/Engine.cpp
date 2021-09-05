#include "Engine.h"
#include <allegro5/allegro_opengl.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_native_dialog.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"
#include "imgui_impl_allegro5.h"

namespace DearImguiIntegration {
	void Init(ALLEGRO_DISPLAY* display) {
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// Setup Platform/Renderer backends
		ImGui_ImplAllegro5_Init(display);
	}

	bool Event(ALLEGRO_EVENT& event) {
		ImGui_ImplAllegro5_ProcessEvent(&event);

		if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE)
		{
			ImGui_ImplAllegro5_InvalidateDeviceObjects();
			//al_acknowledge_resize(display);
			ImGui_ImplAllegro5_CreateDeviceObjects();
		}

		ImGuiIO& io = ImGui::GetIO();
		return io.WantCaptureKeyboard || io.WantCaptureMouse;
	}

	void NewFrame() {
		// Start the Dear ImGui frame
		ImGui_ImplAllegro5_NewFrame();
		ImGui::NewFrame();
	}

	void Render() {
		ImGui::Render();
		ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());
	}

	void Destroy() {
		ImGui_ImplAllegro5_Shutdown();
		ImGui::DestroyContext();
	}
}


Engine::Engine()
	: inputRoot()
	, updateRoot()
	, graphicTargets()
	, defaultGraphicTarget()
	, mainGraphic(*this)
	, debugGraphic(graphics)
	, overlayGraphic(*this)
	, collisionConfig(nullptr)
	, dispatcher(nullptr)
	, overlappingPairCache(nullptr)
	, solver(nullptr)
	, physics(nullptr)
	, display(nullptr)
	, eventQueue(nullptr)
	, initSuccess(false)
{
	graphicTargets.AddChild(&defaultGraphicTarget);

	defaultGraphicTarget.AddChild(&mainGraphic);
	defaultGraphicTarget.AddChild(&debugGraphic);
	defaultGraphicTarget.AddChild(&overlayGraphic);

	lastTime = time = 0.0;
	dt = dtTarget = 1.0 / 60.0;
}

Engine::~Engine()
{
	for (std::map<const type_info*, Data*>::iterator it = userData.begin(); it != userData.end(); ++it) {
		delete it->second;
	}
	userData.clear();

	if (display) { al_destroy_display(display); }
	if (eventQueue) { al_destroy_event_queue(eventQueue); }

	DearImguiIntegration::Destroy();	
	
	if (collisionConfig) delete collisionConfig;
	if (dispatcher) delete dispatcher;
	if (overlappingPairCache) delete overlappingPairCache;
	if (solver) delete solver;
	if (physics) delete physics;
}

void RecursiveTick(Engine::Dynamic* dynamic) {
	dynamic->Tick();
	for (unsigned int i = 0; i < dynamic->children.size(); ++i) {
		RecursiveTick(dynamic->children[i]);
	}
}

void myTickCallback(btDynamicsWorld *world, btScalar timeStep) {
	int numManifolds = world->getDispatcher()->getNumManifolds();
	for (int i = 0; i<numManifolds; i++)
	{
		btPersistentManifold& contactManifold = *world->getDispatcher()->getManifoldByIndexInternal(i);
		if (contactManifold.getNumContacts() > 0) {
			const btCollisionObject* obA = static_cast<const btCollisionObject*>(contactManifold.getBody0());
			const btCollisionObject* obB = static_cast<const btCollisionObject*>(contactManifold.getBody1());
			Engine::Dynamic* dA = (Engine::Dynamic*)obA->getUserPointer();
			Engine::Dynamic* dB = (Engine::Dynamic*)obB->getUserPointer();

			if (dA) { dA->Collision(dB, contactManifold); }
			if (dB) { dB->Collision(dA, contactManifold); }
		}
	}

	Engine& engine = *((Engine*)world->getWorldUserInfo());
	RecursiveTick(&engine.dynamicRoot);
}

bool Engine::Init()
{
	if (initSuccess) {
		return true;
	}

	if (!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return false;
	}

	if (!al_install_keyboard()) {
		fprintf(stderr, "failed to install keyboard!\n");
		return false;
	}

	if (!al_install_mouse()) {
		fprintf(stderr, "failed to install mouse!\n");
		return false;
	}

	if (!al_init_image_addon()) {
		fprintf(stderr, "failed to load image addon!\n");
		return false;
	}

	if (!al_init_font_addon()) {
		fprintf(stderr, "failed to load font addon!\n");
		return false;
	}

	if (!al_init_native_dialog_addon()) {
		fprintf(stderr, "failed to load native dialog addon!\n");
		return false;
	}

	al_set_new_display_flags(ALLEGRO_OPENGL);
	al_set_new_display_option(ALLEGRO_DEPTH_SIZE, 16, ALLEGRO_SUGGEST);
	display = al_create_display(1280, 720);
	if (!display) {
		fprintf(stderr, "failed to create display!\n");
		return false;
	}

	defaultGraphicTarget.bitmap = al_get_backbuffer(display);

	eventQueue = al_create_event_queue();
	if (!eventQueue) {
		fprintf(stderr, "failed to create event queue!\n");
		return false;
	};

	al_register_event_source(eventQueue, al_get_display_event_source(display));
	al_register_event_source(eventQueue, al_get_keyboard_event_source());
	al_register_event_source(eventQueue, al_get_mouse_event_source());

	char* dir = al_get_current_directory();
	currentDirectory = std::string(dir);
	al_free(dir);

	collisionConfig = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfig);
	overlappingPairCache = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();
	physics = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfig);
	physics->setGravity(btVector3(0, 0, -10));
	physics->setInternalTickCallback(myTickCallback, (void*)this);

	initSuccess = true;

	int maj, min;
	glGetIntegerv(GL_MAJOR_VERSION, &maj);
	glGetIntegerv(GL_MINOR_VERSION, &min);

	fprintf(stdout, "opengl version is %i.%i\n", maj, min);

	DearImguiIntegration::Init(display);

	return true;
}

bool RecursiveInput(Engine::Input* input, ALLEGRO_EVENT& event) {
	if (!input->Event(event)) {
		for (int i = 0; i < input->children.size(); ++i) {
			if (RecursiveInput(input->children[i], event)) {
				return true;
			}
		}
		return false;
	}
	return true;
}

void RecursiveUpdate(Engine::Update* update) {
	update->Step();
	for (int i = 0; i < update->children.size(); ++i) {
		RecursiveUpdate(update->children[i]);
	}
}

void RecursiveGraphic(Engine::Graphic* graphic) {
	graphic->Draw();
	for (int i = 0; i < graphic->children.size(); ++i) {
		RecursiveGraphic(graphic->children[i]);
	}
}

bool Engine::OneLoop()
{
	// time management
	lastTime = time;
	time = al_get_time();
	dt = time - lastTime;

	bool stayOpen = true;

	al_get_keyboard_state(&Input::keyboardState);
	al_get_mouse_state(&Input::mouseState);

	ALLEGRO_EVENT event;
	while (al_get_next_event(eventQueue, &event)) {
		if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			stayOpen = false;
		}
		else if (!DearImguiIntegration::Event(event)) {
			RecursiveInput(&inputRoot, event);
		}
	}

	DearImguiIntegration::NewFrame();

	physics->stepSimulation(dt, 10);

	RecursiveUpdate(&updateRoot);
	RecursiveGraphic(&graphicTargets);

	DearImguiIntegration::Render();

	al_flip_display();

	double elapsed = al_get_time() - time;

	if (elapsed < dtTarget) {
		al_rest(dtTarget - elapsed);
	}

	return stayOpen;
}

void Engine::Dynamic::Collision(Engine::Dynamic* other, btPersistentManifold& manifold)
{
}

void Engine::Dynamic::ReactToCollisionFrom(btRigidBody & body)
{
	body.setUserPointer((void*)this);
}

ALLEGRO_MOUSE_STATE Engine::Input::mouseState;
ALLEGRO_KEYBOARD_STATE Engine::Input::keyboardState;

bool Engine::InputRoot::Event(ALLEGRO_EVENT & event)
{
	return false;
}

void Engine::DynamicRoot::Tick() {}
void Engine::UpdateRoot::Step() {}
void Engine::GraphicRoot::Draw() {}

Engine::GraphicTarget::GraphicTarget(ALLEGRO_BITMAP* bitmap) :
	ownsBitmap(false),
	bitmap(bitmap),
	clearColor(0.0f)
{
}

Engine::GraphicTarget::GraphicTarget(int width, int height, bool depth) :
	ownsBitmap(true),
	bitmap(nullptr),
	clearColor(0.0f)
{
	int olddepth = al_get_new_bitmap_depth();
	if (depth) {
		al_set_new_bitmap_depth(16);
	}

	bitmap = al_create_bitmap(width, height);

	if (depth) {
		al_set_new_bitmap_depth(olddepth);
	}
}

Engine::GraphicTarget::~GraphicTarget()
{
	if (ownsBitmap && bitmap) {
		al_destroy_bitmap(bitmap);
	}
}

void Engine::GraphicTarget::Draw()
{
	if (bitmap) {
		al_set_target_bitmap(bitmap);
		glViewport(0, 0, al_get_bitmap_width(bitmap), al_get_bitmap_height(bitmap));
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

Engine::ShaderGraphic::ShaderGraphic(GraphicContext & g) : graphics(g)
{
}

void Engine::ShaderGraphic::Draw()
{
	for (std::map<Program*, GraphicRoot>::iterator it = programChildren.begin(); it != programChildren.end(); ++it) {
		graphics.programs.Use(it->first);
		graphics.SetCommonUniforms();
		RecursiveGraphic(&it->second);
	}
	graphics.programs.StopToUse();
}

void Engine::ShaderGraphic::AddChildToProgram(Graphic * child, const std::string & progFile)
{
	programChildren[&graphics.programs.Get(progFile)].AddChild(child);
}

void Engine::ShaderGraphic::RemoveChildFromProgram(Graphic* child, const std::string& progFile)
{
	programChildren[&graphics.programs.Get(progFile)].RemoveChild(child);
}

Engine::MainGraphic::MainGraphic(Engine& e) : ShaderGraphic(e.graphics), engine(e)
{
}

void Engine::MainGraphic::Draw()
{
	//viewport and clear handled in defaultGraphicTarget

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(glm::value_ptr(engine.graphics.proj));
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(glm::value_ptr(engine.graphics.view));

	ShaderGraphic::Draw();
}

Engine::DebugGraphic::DebugGraphic(GraphicContext & g) : ShaderGraphic(g)
{
}

void Engine::DebugGraphic::Draw()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	ShaderGraphic::Draw();
}

Engine::OverlayGraphic::OverlayGraphic(Engine & e) : ShaderGraphic(e.graphics), engine(e)
{
}

void Engine::OverlayGraphic::Draw()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, (double)al_get_display_width(engine.display), (double)al_get_display_height(engine.display), 0, 1, -1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor3ub(255, 255, 255);

	ShaderGraphic::Draw();
}
