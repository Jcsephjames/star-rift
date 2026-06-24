#pragma once
#ifndef SCRAP_HPP
#define SCRAP_HPP

#include <raylib.h>
#include "enemies.hpp"
#include "asteroids.hpp"

#define MAX_SCRAP_PICKUPS 32
#define EMBERS_PER_SCRAP 6

struct ScrapEmber {
    bool active;
    Vector2 position;
    Vector2 velocity;
    float lifetime;
    float maxLifetime;
};

struct ScrapPickup {
    bool active;
    Vector2 position;
    Vector2 velocity;
    int amount;
    float lifetime;
    float rotation;
    float spinSpeed;
    float emberTimer;
    ScrapEmber embers[EMBERS_PER_SCRAP];
};

void InitScrapPickups();
void SpawnScrapPickup(Vector2 position, int amount, Vector2 driftVelocity);
void UpdateScrapPickups(float deltaTime);
void DrawScrapPickups();

int GetEnemyScrapDrop(EnemyTier tier);
int GetAsteroidScrapDrop(AsteroidSize size);
void AddScrap(int amount);

#endif
