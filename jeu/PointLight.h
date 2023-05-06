#pragma once

#include "Game.h"

class PointLight : public UpdateTask, public IDrawTask {
private:
    Camera lightsource;
    RenderTexture2D rendertex;

    struct Uniforms {
        int shadowmap;
        int lsm;
        int lightpos;
        int bias;
    };

    Uniforms uniforms;
    Matrix lsm;

    class UniformSetup : public IDrawTask {
    private:
        PointLight& pl;
    public:
        UniformSetup(PointLight& pl) : pl(pl) {}

        void Draw() {
            pl.SetupUniforms();
        }
    };
    UniformSetup uniformSetup;

public:

    PointLight();
    ~PointLight();

    void Do();

    void Draw();

    void SetupUniforms();
};