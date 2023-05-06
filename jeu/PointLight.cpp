#include "PointLight.h"
#include "raylibext.h"
#include "rlgl.h"
#include "imgui.h"

PointLight::PointLight() : UpdateTask(PRIO_NORMAL), uniformSetup(*this), lsm(MatrixIdentity()) {
    lightsource = game.camera;
    lightsource.fovy = 45.0f;
    rendertex = LoadRenderTextureDepthTex(1024, 1024);

    uniforms.shadowmap = GetShaderLocation(game.mainshader, "shadowmap");
    uniforms.lsm = GetShaderLocation(game.mainshader, "lsm");
    uniforms.lightpos = GetShaderLocation(game.mainshader, "lightpos");
    uniforms.bias = GetShaderLocation(game.mainshader, "bias");
}

PointLight::~PointLight() {
    UnloadRenderTextureDepthTex(rendertex);
}

void PointLight::Do() {
    game.AddDrawTask(*this, DRAW_3D_LIGHTS);
}

void PointLight::Draw() {
    BeginTextureMode(rendertex);
    ClearBackground(WHITE);
    rlSetCullFace(RL_CULL_FACE_FRONT);
    BeginMode3D(lightsource);
    // refactor todo : replace the projection done by raylib to have control over the near and far planes
    lsm = MatrixMultiply(rlGetMatrixModelview(), rlGetMatrixProjection());
    for (auto& sc : game.GetShadowCasters()) {
        sc.get().Draw();
    }
    EndMode3D();
    rlSetCullFace(RL_CULL_FACE_BACK);
    EndTextureMode();

    // added only if this one passed the bounding box test
    game.AddDrawTask(uniformSetup, DRAW_3D_UNIFORMS);

    // lightsource preview
    /*BeginShaderMode(depthrender);
    Texture& tex = rendertex.depth;
    const int texpreviewsize = 150;
    DrawTexturePro(tex,
        { 0, 0, (float)tex.width, (float)-tex.height },
        { 5.0f, float(GetScreenHeight() - texpreviewsize - 5), (float)texpreviewsize , (float)texpreviewsize },
        { 0.0f, 0.0f }, 0.0f, WHITE);
    EndShaderMode();
    DrawRectangleLines(5, GetScreenHeight() - tex.height - 5, texpreviewsize, texpreviewsize, BLUE);*/
}

void PointLight::SetupUniforms() {
    SetShaderValueMatrix(game.mainshader, game.mainshader.locs[SHADER_LOC_MATRIX_MODEL], rlGetMatrixTransform());
    SetShaderValueTexture(game.mainshader, uniforms.shadowmap, rendertex.depth);
    SetShaderValueMatrix(game.mainshader, uniforms.lsm, lsm);
    SetShaderValue(game.mainshader, uniforms.lightpos, &lightsource.position, SHADER_UNIFORM_VEC3);
    static float bias = 0.000005f;
    ImGui::SliderFloat("Shadow bias", &bias, 0.0f, 0.00001f, "%.8f");
    SetShaderValue(game.mainshader, uniforms.bias, &bias, SHADER_UNIFORM_FLOAT);
    Vector3 lightdir = Vector3Normalize(lightsource.target - lightsource.position);
}
