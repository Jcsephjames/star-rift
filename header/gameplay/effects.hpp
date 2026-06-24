#pragma once
#ifndef EFFECTS_HPP
#define EFFECTS_HPP

#include <raylib.h>
#include "asteroids.hpp"

#define MAX_PARTICLES 128
#define MAX_IMPACT_FLASHES 32

struct Particle {
    bool active;
    Vector2 position;
    Vector2 velocity;
    float lifetime;
    float maxLifetime;
    float radius;
    Color color;
};

struct ImpactFlash {
    bool active;
    Vector2 position;
    float lifetime;
    float maxLifetime;
    float radius;
};

void InitEffects();
void UpdateEffects(float deltaTime);
void DrawEffects();

void TriggerScreenShake(float intensity, float duration);
void TriggerPlayerHitShake(float damage);
Camera2D GetShakeCamera();

void SpawnExplosion(Vector2 position, AsteroidSize size, bool playSfx = true);
void SpawnShipExplosion(Vector2 position);
void SpawnLaserImpact(Vector2 position);

#define MAX_FLOATING_TEXTS 12

struct FloatingText {
    bool active;
    Vector2 position;
    float lifetime;
    float maxLifetime;
    Color color;
    char text[32];
};

void SpawnFloatingText(Vector2 position, const char* text, Color color);
void UpdateFloatingTexts(float deltaTime);
void DrawFloatingTexts();

void TriggerEmpPulse(Vector2 origin);

#endif
