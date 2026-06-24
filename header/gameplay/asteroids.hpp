#pragma once
#ifndef ASTEROIDS_HPP
#define ASTEROIDS_HPP

#include <raylib.h>

enum AsteroidSize { SMALL = 1, MEDIUM = 2, LARGE = 4 };

#define ASTEROID_ROTATION_MAX 200
#define ASTEROID_ROTATION_MIN 5
#define MAX_ASTEROIDS 64
#define RIFT_ASTEROID_MAX 10
#define RIFT_ASTEROID_FIELD_MAX 24

#define ASTEROIDS_VICTORY_TIME_SEC 300.0f
#define ASTEROIDS_VICTORY_SCORE 5000
#define ASTEROIDS_VICTORY_CLEARS 100

struct Asteroid {
    bool active;
    float age;
    float health;
    float hitFlashTimer;
    float collisionDamageCooldown;

    Vector2 position;
    Vector2 velocity;
    AsteroidSize size;

    float rotation;
    float rotationSpeed;
};

extern Asteroid asteroids[MAX_ASTEROIDS];

int CountActiveAsteroids();
int GetAsteroidsCleared();
float GetAsteroidRadius(AsteroidSize size);
void CheckAsteroidsVictory();

void InitAsteroids();
void SpawnAsteroidFromEdge();

void AsteroidUpdate(Asteroid* asteroid, float deltaTime);
void AsteroidDraw(Asteroid asteroid);
void AddAsteroid(Vector2 position, Vector2 velocity, AsteroidSize size);
int AddAsteroidAt(Vector2 position, Vector2 velocity, AsteroidSize size, float rotationSpeed);

void AsteroidSpawnTimer(float deltaTime);

void CheckLaserAsteroidCollisions();
void CheckAsteroidAsteroidCollisions();
void CheckAsteroidPlayerCollisions(float deltaTime);
void CheckAsteroidEnemyCollisions(float deltaTime);
void AsteroidUpdateLoop();
void AsteroidDrawAll();

#endif // ASTEROIDS_HPP
