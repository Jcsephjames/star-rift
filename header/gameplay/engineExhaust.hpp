#pragma once
#ifndef ENGINE_EXHAUST_HPP
#define ENGINE_EXHAUST_HPP

#include <raylib.h>

struct EngineExhaustParams {
    Vector2 position;
    float direction;
    float scale;
    float intensity;
    float boostFactor;
    float turnRate;
    bool neonColors;
    bool ionColors;
    float time;
};

void InitEngineExhaust();
void UpdateEngineExhaust(float deltaTime);
void EmitEngineExhaust(const EngineExhaustParams& params);
void DrawAllEngineExhaust();

#endif
