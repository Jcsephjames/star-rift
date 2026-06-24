#pragma once
#ifndef PICKUPS_HPP
#define PICKUPS_HPP

#include <raylib.h>

#define MAX_PICKUPS 24
#define SPARKS_PER_PICKUP 8

struct PickupSpark {
    bool active;
    Vector2 position;
    Vector2 velocity;
    float lifetime;
    float maxLifetime;
};

struct HealthPickup {
    bool active;
    Vector2 position;
    Vector2 velocity;
    float healthValue;
    float lifetime;
    float sparkTimer;
    PickupSpark sparks[SPARKS_PER_PICKUP];
};

void InitPickups();
void SpawnHealthPickup(Vector2 position, float healthValue, Vector2 driftVelocity);
void UpdatePickups(float deltaTime);
void DrawPickups();

#endif
