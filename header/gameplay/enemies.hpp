#pragma once
#ifndef ENEMIES_HPP
#define ENEMIES_HPP

#include <raylib.h>
#include "ship.hpp"

#define MAX_ENEMIES 16

constexpr int RIFT_BOSS_WAVE = 10;
constexpr int RIFT_FINALE_WAVE = 20;

enum class EnemyTier {
    GRUNT,
    RARE,
    ELITE,
    FALCON,
    DEATH_STAR
};

enum class EnemyPlaystyle {
    STALKER,
    FIRING_RUN,
    PURSUE,
    BOSS_GUNNER,
    BOSS_BLADE,
    BOSS_STATION
};

struct Enemy {
    bool active;
    Ship ship;
    EnemyTier tier;
    EnemyPlaystyle playstyle;
    float healthDrop;
    Vector2 position;
    float direction;
    float health;
    float maxHealth;
    float hitFlashTimer;
    float timeSinceLastShot;
    float breakDirection;
    float radius;
    float telegraphTimer;
    float pendingFireDirection;
    float repositionTimer;
    float commitTimer;
    float runHeading;
    float collisionDamageCooldown;
    bool enteringScreen;
    float prevDirection;
    float rotationRate;
    float thrustIntensity;
    float thrustBoost;
};

extern Enemy enemies[MAX_ENEMIES];

void InitEnemies();
void EnemyUpdateLoop(float deltaTime);
void EnemyDrawAll();
int CountActiveEnemies();
int GetEnemyWave();
bool IsAsteroidFieldWave();
void ContinueEndlessRift();

#endif
