#include "Engine.h"
#include <allegro5/allegro_opengl.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_native_dialog.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"
#include "imgui_impl_allegro5.h"
#include "Scene.h"
#include "DefaultColors.h"

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

Engine singleton;

Engine& Engine::Get()
{
	return singleton;
}

Engine::Engine()
	: inputRoot()
	, updateRoot()
	, graphicTargets()
	, defaultGraphicTarget()
	, mainGraphic()
	, debugGraphic()
	, overlayGraphic()
	, collisionConfig(nullptr)
	, dispatcher(nullptr)
	, overlappingPairCache(nullptr)
	, solver(nullptr)
	, physics(nullptr)
	, display(nullptr)
	, eventQueue(nullptr)
	, initSuccess(false)
	, guiEnabled(false)
{
	graphicTargets.AddChild(&defaultGraphicTarget);

	defaultGraphicTarget.AddChild(&mainGraphic);
	defaultGraphicTarget.AddChild(&debugGraphic);
	defaultGraphicTarget.AddChild(&overlayGraphic);

	rootObject.AddChild(&inputRoot);
	rootObject.AddChild(&dynamicRoot);
	rootObject.AddChild(&updateRoot);
	rootObject.AddChild(&graphicTargets);

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

	graphics.Clear();

	al_uninstall_system();
}

void RecursiveTick(Engine::Dynamic* dynamic) {
	dynamic->Tick();
	for (int i = 0; i < dynamic->ChildrenSize(); ++i) {
		RecursiveTick(dynamic->GetChild(i));
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

	InitDefaultColors();

	if (!al_install_system(ALLEGRO_VERSION_INT, nullptr)) {
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

bool Engine::RecursiveInput(Engine::Input* input, ALLEGRO_EVENT& event, bool doroot) {
	for (int i = 0; i < input->ChildrenSize(); ++i) {
		if (RecursiveInput(input->GetChild(i), event)) {
			return true;
		}
	}
	bool ret = false;
	if (doroot) {
		ret = input->Event(event);
	}
	return ret;
}

void Engine::RecursiveUpdate(Engine::Update* update) {
	update->Step();
	for (int i = 0; i < update->ChildrenSize(); ++i) {
		RecursiveUpdate(update->GetChild(i));
	}
	update->PostStep();
}

void Engine::RecursiveGraphic(Engine::Graphic* graphic) {
	graphic->Draw();
	for (int i = 0; i < graphic->ChildrenSize(); ++i) {
		RecursiveGraphic(graphic->GetChild(i));
	}
	graphic->PostDraw();
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
		else if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_F1) {
			guiEnabled = !guiEnabled;
		}
		else if (!DearImguiIntegration::Event(event)) {
			RecursiveInput(&inputRoot, event);
		}
	}

	DearImguiIntegration::NewFrame();

	physics->stepSimulation(dt, 10);

	RecursiveUpdate(&updateRoot);
	RecursiveGraphic(&graphicTargets);

	ShowGui();

	DearImguiIntegration::Render();

	al_flip_display();

	double elapsed = al_get_time() - time;

	if (elapsed < dtTarget) {
		al_rest(dtTarget - elapsed);
	}

	return stayOpen;
}

static Engine::Object* selectedobj = nullptr;

bool ImgObjTreeNode(Engine::Object* obj, bool leaf) {
	const ImGuiTreeNodeFlags leafFlag = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
	const bool ret = ImGui::TreeNodeEx((void*)obj,
		(leaf ? leafFlag : 0) | (selectedobj == obj ? ImGuiTreeNodeFlags_Selected : 0),
		obj->ObjectTypeName());
	if (ImGui::IsItemClicked()) {
		selectedobj = obj;
	}
	return ret;
}

void RecursiveGuiHierarchyObject(Engine::Object* obj, const ProgramManager& programs, int depth=0) {
	Engine::ShaderGraphic* sg = dynamic_cast<Engine::ShaderGraphic*>(obj);
	
	ImGui::PushID(obj);
	if (obj->ChildrenSize() > 0 || sg) {
		if (depth==0 || ImgObjTreeNode(obj, false)) {
			if (sg) {
				if (ImGui::TreeNode("Programs")) {
					for (auto&& it : sg->programChildren) {
						const Program& prg = *it.first;
						std::string prgName = programs.GetKey(prg, "program not found");
						if (it.second.ChildrenSize() > 0) {
							if (ImGui::TreeNode(prgName.c_str())) {
								for (int j = 0; j < it.second.ChildrenSize(); ++j) {
									//ImGui::BulletText(it.second.GetChild(j)->ObjectTypeName());
									ImgObjTreeNode(it.second.GetChild(j), true);
								}
								ImGui::TreePop();
							}
						}
						else {
							ImGui::BulletText(prgName.c_str());
						}
					}
					ImGui::TreePop();
				}
			}

			for (int i = 0; i < obj->ChildrenSize(); ++i) {
				RecursiveGuiHierarchyObject(obj->GetChild(i), programs, depth+1);
			}

			if (depth > 0) { ImGui::TreePop(); }
		}
	}
	else {
		ImgObjTreeNode(obj, true);
	}
	ImGui::PopID();
}

void ConstructUsedObjects(Engine::Object* obj, std::set<Engine::Object*>& usedObjs) {
	usedObjs.insert(obj);

	Engine::ShaderGraphic* sg = dynamic_cast<Engine::ShaderGraphic*>(obj);
	if (sg) {
		for (auto&& it : sg->programChildren) {
			if (it.second.ChildrenSize() > 0) {
				for (int j = 0; j < it.second.ChildrenSize(); ++j) {
					usedObjs.insert(it.second.GetChild(j));
				}
			}
		}
	}

	for (int i = 0; i < obj->ChildrenSize(); ++i) {
		ConstructUsedObjects(obj->GetChild(i), usedObjs);
	}
}

void Engine::ShowGui() {
	if (guiEnabled) {
		static bool win_objects = false;
		static bool win_demo = false;
		static bool win_scene = false;

		if (win_objects) {
			// verify selectedobj validity
			if (objectsTracker.find(selectedobj) == objectsTracker.end()) {
				selectedobj = nullptr;
			}

			if (ImGui::Begin("Objects", &win_objects)) {
				ImVec2 r = ImGui::GetContentRegionAvail();
				r.x *= 0.4f;
				if (ImGui::BeginChild("Objects list", r, true, ImGuiWindowFlags_HorizontalScrollbar)) {
					static bool show_hierarchy = false;
					ImGui::Checkbox("Show hierarchy", &show_hierarchy);
					if (show_hierarchy) {
						RecursiveGuiHierarchyObject(&rootObject, graphics.programs);
						if (ImGui::TreeNode("Orphans")) {
							std::set<Object*> usedObjs;
							ConstructUsedObjects(&rootObject, usedObjs);
							for (auto obj : objectsTracker) {
								if (usedObjs.find(obj) == usedObjs.end()) {
									ImgObjTreeNode(obj, true);
								}
							}
							ImGui::TreePop();
						}
					}
					else {
						for (auto obj : objectsTracker) {
							ImgObjTreeNode(obj, true);
						}
					}
				}
				ImGui::EndChild();
				ImGui::SameLine();
				r = ImGui::GetContentRegionAvail();
				if (ImGui::BeginChild("Object infos", r)) {
					if (selectedobj) {
						selectedobj->ExposeFunction();
					}
				}
				ImGui::EndChild();
			}
			ImGui::End();
		}
		if (win_demo) {
			ImGui::ShowDemoWindow(&win_demo);
		}
		if (win_scene) {
			if (ImGui::Begin("Scene", &win_scene)) {
				for (auto&& it : ConstructorCollection::Get().themap) {
					if (ImGui::Button("Spawn")) {
						it.second->Construct();
					}
					ImGui::SameLine();
					ImGui::Text(it.first.c_str());
				}
			}
			ImGui::End();
		}

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Windows"))
			{
				ImGui::MenuItem("Objects", nullptr, &win_objects);
				ImGui::MenuItem("Demo", nullptr, &win_demo);
				ImGui::MenuItem("Scene", nullptr, &win_scene);

				bool hasDebugChild = defaultGraphicTarget.HasChild(&debugGraphic);
				bool debugDraw = hasDebugChild;
				ImGui::MenuItem("Debug draw", nullptr, &debugDraw);
				
				if (debugDraw != hasDebugChild) {
					if (debugDraw) {
						defaultGraphicTarget.AddChildAfter(&debugGraphic, &mainGraphic);
					}
					else {
						defaultGraphicTarget.RemoveChild(&debugGraphic);
					}
				}

				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}
}

Engine::Object::Object() :
	engine(Engine::Get())
{
	engine.objectsTracker.insert(this);
}

Engine::Object::~Object()
{
	engine.objectsTracker.erase(this);
}

/*void Engine::Object::AddChild(Object* c, bool onTop)
{
	if (onTop) {
		children.insert(children.begin(), c);
	}
	else {
		children.push_back(c);
	}
}

void Engine::Object::RemoveChild(Object* c)
{
	for (auto it = children.begin(); it != children.end(); ) {
		if (*it == c) {
			it = children.erase(it);
		}
		else {
			++it;
		}
	}
}*/

IMPLEMENT_EXPOSE_MEMBER(Engine::Object) {
	ImGui::Text("This object did not implement exposing.");
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

IMPLEMENT_EXPOSE_MEMBER(Engine::GraphicTarget) {
	EXPOSE_VALUE(bitmap);
}

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

Engine::ShaderGraphic::ShaderGraphic()
{
}

void Engine::ShaderGraphic::Draw()
{
	for (std::map<Program*, GraphicRoot>::iterator it = programChildren.begin(); it != programChildren.end(); ++it) {
		engine.graphics.programs.Use(it->first);
		engine.graphics.SetCommonUniforms();
		RecursiveGraphic(&it->second);
	}
	engine.graphics.programs.StopToUse();
}

void Engine::ShaderGraphic::AddChildToProgram(Graphic * child, const std::string & progFile)
{
	programChildren[&engine.graphics.programs.Get(progFile)].AddChild(child);
}

void Engine::ShaderGraphic::RemoveChildFromProgram(Graphic* child, const std::string& progFile)
{
	programChildren[&engine.graphics.programs.Get(progFile)].RemoveChild(child);
}

Engine::MainGraphic::MainGraphic()
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

Engine::DebugGraphic::DebugGraphic()
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

Engine::OverlayGraphic::OverlayGraphic()
{
}

void Engine::OverlayGraphic::Draw()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, (double)al_get_display_width(engine.display), (double)al_get_display_height(engine.display), 0, 1, -1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor3ub(255, 255, 255);

	ShaderGraphic::Draw();
}
