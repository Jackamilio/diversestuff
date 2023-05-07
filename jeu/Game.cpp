#include "Game.h"
#include "raymath.h"
#include "rlImGui.h"
#include "rlgl.h"
#include <fstream>

using nlohmann::json;

Game game;

typedef std::multimap<int, IUpdateTask&>::iterator TLUit;
typedef std::multimap<int, IDrawTask&>::iterator TLDit;

void Game::RemoveUpdateTask(IUpdateTask& task, int priority)
{
    std::pair<TLUit, TLUit> iterpair = update.equal_range(priority);

    TLUit it = iterpair.first;
    for (; it != iterpair.second; ++it) {
        if (&it->second == &task) {
            update.erase(it);
            break;
        }
    }
}

Game::Game() : camera{0}, mainshader{0}, deltaTime(1.0f/60.0f)
{
}

const char* settingsFileName = "GameSettings.json";

void Game::Init()
{
    const int screenWidth = 1280;
    const int screenHeight = 800;

    if (FileExists(settingsFileName)) {
        std::ifstream in(settingsFileName);
        settings = nlohmann::json::parse(in);
    }

    InitWindow(screenWidth, screenHeight, "jeu");

    camera.position = { 0.0f, 2.0f, 4.0f };    // Camera position
    camera.target = { 0.0f, 1.0f, 0.0f };      // Camera looking at point
    camera.up = { 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    rlImGuiEnable(false);
    DisableCursor();                    // Limit cursor to relative movement inside the window

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second

    mainshader = LoadShader("shadow.vs", "shadow.fs");

    rlImGuiSetup(true);
    rlImGuiBegin();
}

extern void ImGui::ShowDemoWindow(bool* p_open);

void Game::Loop()
{
    while (!WindowShouldClose())
    {
        // F keys and edit options
        if (IsKeyPressed(KEY_F1)) {
            rlImGuiEnable(!rlImGuiIsEnabled());
            rlImGuiIsEnabled() ? EnableCursor() : DisableCursor();
        }

        static bool showsettings = false;
        if (IsKeyPressed(KEY_F2)) {
            showsettings = !showsettings;
        }

        if (showsettings && ImGui::Begin("Reflected values", &showsettings)) {
            GuiReflection("Game settings", settings);
            if (ImGui::Button("Save settings")) {
                std::ofstream out(settingsFileName, std::ios::out);
                out << json(settings);
            }
            ImGui::End();
        }

        static bool demowindow = false;
        if (IsKeyPressed(KEY_F3)) {
            demowindow = !demowindow;
        }
        if (demowindow) {
            ImGui::ShowDemoWindow(&demowindow);
        }

        for (auto& up : update) {
            up.second.Do();
        }

        // inputs for the game
        if (rlImGuiIsEnabled()) {
            input = {};
        }
        else {
            input.movement.x = (float)IsKeyDown(KEY_D) - (float)IsKeyDown(KEY_A);
            input.movement.y = (float)IsKeyDown(KEY_W) - (float)IsKeyDown(KEY_S);
            input.view = Vector2Scale(GetMouseDelta(), 0.05f);
            input.jump.SetCurrentFrame(IsKeyDown(KEY_SPACE));
            input.attack.SetCurrentFrame(IsKeyDown(KEY_F));
            input.placeblock.SetCurrentFrame(IsMouseButtonDown(0));
            input.removeblock.SetCurrentFrame(IsMouseButtonDown(2));
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // first draw any negative priority that is before DRAW_3D_FIRST
        TLDit it = draw.begin();
        while (it != draw.end() && it->first < DRAW_3D_UNIFORMS) {
            it->second.Draw();
            ++it;
        }

        // then all 3D stuff
        BeginMode3D(camera);
        BeginShaderMode(mainshader);

        while (it != draw.end() && it->first <= DRAW_3D_SHADER) {
            it->second.Draw();
            ++it;
        }

        EndShaderMode();

        while (it != draw.end() && it->first <= DRAW_3D_NOSHADER) {
            it->second.Draw();
            ++it;
        }

        EndMode3D();

        // finally the rest
        while (it != draw.end()) {
            it->second.Draw();
            ++it;
        }

        draw.clear();
        shadowcasters.clear();

        rlImGuiEnd();
        EndDrawing();
        rlImGuiBegin();
    }
}

void Game::Quit()
{
    rlImGuiEnd();
    rlImGuiShutdown();

    UnloadShader(mainshader);

    CloseWindow();
}

UpdateTask::UpdateTask(int prio) : priority(prio)
{
    game.AddUpdateTask(*this, priority);
}

UpdateTask::~UpdateTask()
{
    game.RemoveUpdateTask(*this, priority);
}

void DrawTask::Do()
{
    game.AddDrawTask(*this, drawpriority);
}

DrawTask::DrawTask(int prio) : UpdateTask(PRIO_NORMAL), drawpriority(prio)
{
}
