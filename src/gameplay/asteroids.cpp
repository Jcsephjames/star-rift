#include "asteroids.hpp"
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include "globals.hpp"
#include "ship.hpp"
#include "laser.hpp"
#include "laserManager.hpp"
#include "effects.hpp"
#include "enemies.hpp"
#include "scrap.hpp"
#include "runModifiers.hpp"
#include "difficulty.hpp"
#include "game.hpp"

Asteroid asteroids[MAX_ASTEROIDS] = { 0 };
static int asteroidsCleared = 0;

extern Player player;
extern std::vector<Laser*> activeLasers;

void AddAsteroid(Vector2 position, Vector2 velocity, AsteroidSize size);

int CountActiveAsteroids() {
    int count = 0;
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (asteroids[i].active) count++;
    }
    return count;
}

int GetAsteroidsCleared() {
    return asteroidsCleared;
}

void CheckAsteroidsVictory() {
    if (!asteroidsMode || game.gameOver || game.victory) return;

    const char* message = nullptr;

    if (currentRun.time >= ASTEROIDS_VICTORY_TIME_SEC) {
        message = "Sector secured — 5 minutes survived!";
    } else if (currentRun.score >= ASTEROIDS_VICTORY_SCORE) {
        message = "Sector secured — 5,000 points reached!";
    } else if (asteroidsCleared >= ASTEROIDS_VICTORY_CLEARS) {
        message = "Sector secured — 100 asteroids cleared!";
    }

    if (!message) return;

    SpawnFloatingText(
        { gameWidth / 2.0f, gameHeight * 0.32f },
        "SECTOR SECURED!",
        (Color){ 120, 255, 160, 255 }
    );
    game.TriggerVictory(message);
}

static int GetRiftAsteroidLoad() {
    int load = 0;
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (asteroids[i].active) {
            load += static_cast<int>(asteroids[i].size);
        }
    }
    return load;
}

static int GetRiftAsteroidCap() {
    if (IsAsteroidFieldWave()) return RIFT_ASTEROID_FIELD_MAX;
    return RIFT_ASTEROID_MAX;
}

static bool CanSpawnRiftAsteroid(AsteroidSize size) {
    if (asteroidsMode || !asteroidsOn) return true;
    return GetRiftAsteroidLoad() + static_cast<int>(size) <= GetRiftAsteroidCap();
}

void InitAsteroids() {
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        asteroids[i] = { 0 };
    }
    asteroidSpawnTimer = 0.0f;
    asteroidsCleared = 0;

    SpawnAsteroidFromEdge();
}

void SpawnAsteroidFromEdge() {
    bool spawnLeft = GetRandomValue(0, 1);
    Vector2 position = {
        spawnLeft ? 0.0f : (float)gameWidth,
        (float)GetRandomValue(0, gameHeight)
    };

    float angleBase = spawnLeft ? 0.0f : 180.0f;
    float angleOffset = (float)GetRandomValue(-30, 30);
    float angle = angleBase + angleOffset;

    Vector2 velocity = Vector2Rotate({ (float)GetRandomValue(100, 150), 0.0f }, DEG2RAD * angle);

    AsteroidSize validSizes[] = { SMALL, MEDIUM, LARGE };
    AsteroidSize randomSize = validSizes[GetRandomValue(0, 2)];

    AddAsteroid(position, velocity, randomSize);
}

static void CheckAsteroidYBounds(int index) {
    if (yWrapping) return;

    Asteroid* asteroid = &asteroids[index];
    if (!asteroid->active) return;

    float radius = GetAsteroidRadius(asteroid->size);
    float margin = radius + 12.0f;
    if (asteroid->position.y >= -margin && asteroid->position.y <= gameHeight + margin) return;

    asteroid->active = false;
    if (CanSpawnRiftAsteroid(SMALL)) {
        SpawnAsteroidFromEdge();
    }
}

float GetAsteroidHealth(AsteroidSize size) {
    switch (size) {
        case SMALL: return 10;
        case MEDIUM: return 20;
        case LARGE: return 40;
    }
    return 0;
}

float GetAsteroidRadius(AsteroidSize size) {
    switch (size) {
        case SMALL: return 15;
        case MEDIUM: return 30;
        case LARGE: return 60;  
    }
    return 0;
}

Asteroid CreateAsteroid(Vector2 position, Vector2 velocity, float rotation, float rotationSpeed, AsteroidSize size, float age, float health) {
    return (Asteroid){
        .active = true,
        .position = position,
        .velocity = velocity,
        .rotation = rotation,
        .rotationSpeed = rotationSpeed,
        .size = size,
        .age = age,
        .health = health,
        .hitFlashTimer = 0.0f,
        .collisionDamageCooldown = 0.0f
    };
}

static void SplitAsteroid(const Asteroid& asteroid, float speedMultiplier);

static void DestroyAsteroidAt(int index) {
    Asteroid& asteroid = asteroids[index];
    if (!asteroid.active) return;

    Vector2 scrapDrift = Vector2Scale(asteroid.velocity, 0.35f);
    int scrapAmount = GetAsteroidScrapDrop(asteroid.size);

    if (asteroid.size != SMALL) {
        SplitAsteroid(asteroid, 1.0f);
    } else {
        SpawnExplosion(asteroid.position, asteroid.size);
    }
    SpawnScrapPickup(asteroid.position, scrapAmount, scrapDrift);
    asteroid.active = false;

    if (asteroidsMode) {
        asteroidsCleared++;
        if (asteroid.size == SMALL) {
            currentRun.score += 10;
        } else if (asteroid.size == MEDIUM) {
            currentRun.score += 20;
        } else if (asteroid.size == LARGE) {
            currentRun.score += 30;
        }
        CheckAsteroidsVictory();
    }
}

static void DamageAsteroidFromShip(int index, float damage) {
    Asteroid& asteroid = asteroids[index];
    if (!asteroid.active || damage <= 0.0f) return;

    asteroid.health -= damage;
    asteroid.hitFlashTimer = 0.15f;
    SpawnLaserImpact(asteroid.position);

    if (asteroid.health <= 0.0f) {
        DestroyAsteroidAt(index);
    }
}

static void DestroyEnemyFromAsteroid(Enemy& enemy) {
    SpawnShipExplosion(enemy.position);
    float driftRad = (enemy.direction - 90.0f) * DEG2RAD;
    Vector2 driftVelocity = {
        cosf(driftRad) * enemy.ship.speed * 0.4f,
        sinf(driftRad) * enemy.ship.speed * 0.4f
    };
    SpawnScrapPickup(enemy.position, GetEnemyScrapDrop(enemy.tier), driftVelocity);
    enemy.active = false;
}

static void SplitAsteroid(const Asteroid& asteroid, float speedMultiplier) {
    if (asteroid.size == SMALL) {
        SpawnExplosion(asteroid.position, asteroid.size);
        return;
    }

    AsteroidSize smaller = (AsteroidSize)(asteroid.size / 2);
    float parentSpeed = Vector2Length(asteroid.velocity);
    float baseSpeed = (80.0f + parentSpeed * 0.4f) * speedMultiplier;
    float baseAngle = atan2f(asteroid.velocity.y, asteroid.velocity.x);
    float spread = DEG2RAD * GetRandomValue(70, 130);
    float childRadius = GetAsteroidRadius(smaller);

    for (int s = 0; s < 2; s++) {
        float directionSign = (s == 0) ? -1.0f : 1.0f;
        float angle = baseAngle + directionSign * (spread * 0.5f + DEG2RAD * GetRandomValue(-15, 15));
        float speed = baseSpeed + GetRandomValue(-25, 45);
        Vector2 velocity = { cosf(angle) * speed, sinf(angle) * speed };
        Vector2 offset = Vector2Scale(Vector2Normalize(velocity), childRadius * 0.6f);
        Vector2 spawnPos = Vector2Add(asteroid.position, offset);
        float rotationSpeed = static_cast<float>(GetRandomValue(-ASTEROID_ROTATION_MAX, ASTEROID_ROTATION_MAX));

        AddAsteroidAt(spawnPos, velocity, smaller, rotationSpeed);
    }

    SpawnExplosion(asteroid.position, asteroid.size);
}

void AsteroidUpdate(Asteroid* asteroid, float deltaTime){
    if (!asteroid->active) return;
    if (asteroid->hitFlashTimer > 0.0f) {
        asteroid->hitFlashTimer -= deltaTime;
        if (asteroid->hitFlashTimer < 0.0f) asteroid->hitFlashTimer = 0.0f;
    }
    asteroid->position = WrapPosition(asteroid->position, gameWidth, gameHeight);
    asteroid->position.x += asteroid->velocity.x * deltaTime;
    asteroid->position.y += asteroid->velocity.y * deltaTime;
    asteroid->rotation += asteroid->rotationSpeed * deltaTime;
}

void AsteroidDraw(Asteroid asteroid) {
    if (!asteroid.active) return;

    float speed = Vector2Length(asteroid.velocity);
    float lineThickness = 1.0f + (asteroid.size + 1) * 1.5f * (speed < 120 ? 1.5f : 1.0f);

    int sides = 6;
    float radius = GetAsteroidRadius(asteroid.size);
    Color color;

    switch (asteroid.size) {
        case SMALL: color = GRAY; break;
        case MEDIUM: color = DARKGRAY; break;
        case LARGE: color = LIGHTGRAY; break;
    }

    if (asteroid.hitFlashTimer > 0.0f) {
        float flashStrength = asteroid.hitFlashTimer / 0.15f;
        color = Fade(WHITE, 0.35f + flashStrength * 0.65f);
    }

    DrawPoly(asteroid.position, sides, radius, asteroid.rotation, Fade(color, 0.3f));
    for (int i = 0; i < lineThickness; i++) {
        DrawPolyLines(asteroid.position, sides, radius + i, asteroid.rotation, color);
    }

    if (debugging) {
        DrawText(TextFormat("Rotation: %.2f", asteroid.rotation), asteroid.position.x - 20, asteroid.position.y - 20, 10, WHITE);
    }
}

void AddAsteroid(Vector2 position, Vector2 velocity, AsteroidSize size) {
    AddAsteroidAt(
        position,
        velocity,
        size,
        static_cast<float>(GetRandomValue(ASTEROID_ROTATION_MIN, ASTEROID_ROTATION_MAX)) *
            (GetRandomValue(0, 1) ? 1 : -1)
    );
}

int AddAsteroidAt(Vector2 position, Vector2 velocity, AsteroidSize size, float rotationSpeed) {
    if (!CanSpawnRiftAsteroid(size)) return -1;

    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!asteroids[i].active) {
            asteroids[i] = CreateAsteroid(
                position,
                velocity,
                static_cast<float>(rand() % 360),
                rotationSpeed,
                size,
                0.0f,
                GetAsteroidHealth(size)
            );
            return i;
        }
    }
    return -1;
}

void AsteroidSpawnTimer(float deltaTime) {
    asteroidSpawnTimer += deltaTime;
    if (asteroidSpawnTimer < asteroidSpawnInterval / SPAWN_RATE_BOOST) return;

    asteroidSpawnTimer = 0.0f;

    if (!asteroidsMode) {
        bool fieldWave = IsAsteroidFieldWave();
        int baseChance = fieldWave ? 65 : 42;
        float spawnMult = SPAWN_RATE_BOOST * GetDifficultySpawnMultiplier();
        int spawnChance = (int)(baseChance * spawnMult);
        if (spawnChance > 99) spawnChance = 99;
        if (GetRandomValue(0, 100) > spawnChance) return;
        float baseInterval = fieldWave
            ? (float)GetRandomValue(2, 4)
            : (float)GetRandomValue(5, 9);
        asteroidSpawnInterval = baseInterval / GetDifficultySpawnMultiplier();
    }

    if (!CanSpawnRiftAsteroid(SMALL)) return;

    SpawnAsteroidFromEdge();
}

void CheckLaserAsteroidCollisions() {
    for (int i = 0; i < activeLasers.size(); ) {
        Laser* laser = activeLasers[i];
        bool laserHit = false;
        int hitAsteroidIdx = -1;

        for (int j = 0; j < MAX_ASTEROIDS; j++) {
            Asteroid& asteroid = asteroids[j];
            if (!asteroid.active) continue;

            float radius = GetAsteroidRadius(asteroid.size);
            if (CheckCollisionPointCircle(laser->position, asteroid.position, radius)) {
                asteroid.health -= laser->damage;
                asteroid.hitFlashTimer = 0.15f;
                SpawnLaserImpact(laser->position);

                if (asteroid.health <= 0) {
                    DestroyAsteroidAt(j);
                }

                laserHit = true;
                hitAsteroidIdx = j;
                break;
            }
        }

        if (laserHit) {
            Asteroid& asteroid = asteroids[hitAsteroidIdx];
            delete laser;
            activeLasers.erase(activeLasers.begin() + i);
            if (debugging) {
                TraceLog(LOG_INFO, "Asteroid hit! Size: %d | Health: %.2f | Damage: %d", asteroid.size, asteroid.health, player.ship.damage);
            }
        } else {
            ++i;
        }
    }
}
void CheckAsteroidAsteroidCollisions() {
    const float breakThreshold = 150.0f;

    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        Asteroid& a1 = asteroids[i];
        if (!a1.active) continue;

        for (int j = i + 1; j < MAX_ASTEROIDS; j++) {
            Asteroid& a2 = asteroids[j];
            if (!a2.active) continue;
            if (a1.age < 0.5f || a2.age < 0.5f) continue;

            float r1 = GetAsteroidRadius(a1.size);
            float r2 = GetAsteroidRadius(a2.size);
            float distance = Vector2Distance(a1.position, a2.position);

            if (distance < r1 + r2) {
                // Calculate relative velocity magnitude
                Vector2 relVel = Vector2Subtract(a1.velocity, a2.velocity);
                float impactEnergy = Vector2Length(relVel);

                // Rule 1: Large vs Small → ignore (just bounce)
                if ((a1.size == LARGE && a2.size == SMALL) || (a1.size == SMALL && a2.size == LARGE)) {
                    // Optional: Still bounce
                    Vector2 tempVel = a1.velocity;
                    a1.velocity = a2.velocity;
                    a2.velocity = tempVel;
                    continue;
                }

                // Rule 2: Small vs Small → only break if impactEnergy > threshold
                if (a1.size == SMALL && a2.size == SMALL) {
                    if (impactEnergy > breakThreshold) {
                        SpawnExplosion(a1.position, SMALL);
                        SpawnExplosion(a2.position, SMALL);
                        a1.active = false;
                        a2.active = false;
                    } else {
                        // Just bounce off
                        Vector2 tempVel = a1.velocity;
                        a1.velocity = a2.velocity;
                        a2.velocity = tempVel;
                    }
                    continue;
                }

                // Rule 3: Larger asteroids — only break if impactEnergy > threshold
                for (Asteroid* a : { &a1, &a2 }) {
                    if (!a->active) continue;

                    if (a->size != SMALL && impactEnergy > breakThreshold) {
                        SplitAsteroid(*a, 0.8f + impactEnergy / 400.0f);
                        a->active = false;
                    }
                }

            // Bounce by default (even if no destruction)
            Vector2 avgVel = Vector2Scale(Vector2Add(a1.velocity, a2.velocity), 0.5f);
            a1.velocity = Vector2Add(Vector2Scale(Vector2Subtract(a1.velocity, avgVel), -0.5f), avgVel);
            a2.velocity = Vector2Add(Vector2Scale(Vector2Subtract(a2.velocity, avgVel), -0.5f), avgVel);
            }
        }
    }
}
void CheckAsteroidPlayerCollisions(float deltaTime) {
    player.damageCooldown -= deltaTime;
    if (player.damageCooldown < 0.0f) player.damageCooldown = 0.0f;

    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        Asteroid& asteroid = asteroids[i];
        if (!asteroid.active) continue;

        float radius = GetAsteroidRadius(asteroid.size);
        float totalRadius = radius + player.radius;
        float distance = Vector2Distance(asteroid.position, player.position);

        if (distance < totalRadius) {
            Vector2 collisionNormal = Vector2Normalize(Vector2Subtract(player.position, asteroid.position));
            float asteroidSpeed = Vector2Length(asteroid.velocity);
            float playerSpeed = Vector2Length(player.velocity);
            float pushStrength = asteroidSpeed * (asteroid.size + 1) * 0.1f;

            player.velocity = Vector2Add(player.velocity, Vector2Scale(collisionNormal, pushStrength * 0.25f));
            asteroid.velocity = Vector2Add(asteroid.velocity, Vector2Scale(collisionNormal, -pushStrength * 0.05f));

            if (player.damageCooldown <= 0.0f) {
                float baseDamage = asteroidSpeed * (asteroid.size + 1) * 0.1f;
                float damageMult = player.ship.GetPlayerCollisionDamageMultiplier(
                    player.direction, player.position, asteroid.position);
                float damage = baseDamage * damageMult;
                if (damage > 0.0f) {
                    player.life -= GetEffectiveMeleeDamageTaken(damage);
                    player.damageCooldown = 1.0f;
                    TriggerPlayerHitShake(damage);

                    if (debugging) {
                        TraceLog(LOG_INFO, "Player hit! Damage: %.2f | Health: %.2f", damage, player.life);
                    }
                }
            }

            if (asteroid.collisionDamageCooldown <= 0.0f) {
                float shipDamage = playerSpeed * 0.08f + 2.0f;
                DamageAsteroidFromShip(i, shipDamage);
                asteroid.collisionDamageCooldown = 0.8f;
            }
        }
    }
}
void CheckAsteroidEnemyCollisions(float deltaTime) {
    if (!enemiesOn || asteroidsMode) return;

    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        Asteroid& asteroid = asteroids[i];
        if (!asteroid.active) continue;

        float asteroidRadius = GetAsteroidRadius(asteroid.size);
        float asteroidSpeed = Vector2Length(asteroid.velocity);

        for (int j = 0; j < MAX_ENEMIES; j++) {
            Enemy& enemy = enemies[j];
            if (!enemy.active) continue;

            float totalRadius = asteroidRadius + enemy.radius;
            if (Vector2Distance(asteroid.position, enemy.position) >= totalRadius) continue;

            Vector2 collisionNormal = Vector2Normalize(Vector2Subtract(enemy.position, asteroid.position));
            asteroid.velocity = Vector2Add(asteroid.velocity, Vector2Scale(collisionNormal, -asteroidSpeed * 0.08f));

            float enemyRad = (enemy.direction - 90.0f) * DEG2RAD;
            Vector2 enemyVelocity = { cosf(enemyRad) * enemy.ship.speed, sinf(enemyRad) * enemy.ship.speed };
            float relativeSpeed = Vector2Length(Vector2Subtract(asteroid.velocity, enemyVelocity));

            if (asteroid.collisionDamageCooldown <= 0.0f) {
                float asteroidDamage = relativeSpeed * 0.08f + 3.0f;
                DamageAsteroidFromShip(i, asteroidDamage);
                asteroid.collisionDamageCooldown = 0.8f;
            }

            if (enemy.collisionDamageCooldown > 0.0f) continue;

            float baseDamage = asteroidSpeed * (asteroid.size + 1) * 0.12f;
            float damageMult = enemy.ship.GetEnemyCollisionDamageMultiplier(
                enemy.direction, enemy.position, asteroid.position);
            float damage = baseDamage * damageMult;
            if (damage <= 0.0f) continue;

            enemy.health -= damage;
            enemy.hitFlashTimer = 0.15f;
            enemy.collisionDamageCooldown = 0.8f;
            SpawnLaserImpact(enemy.position);

            if (enemy.health <= 0.0f) {
                DestroyEnemyFromAsteroid(enemy);
            }
        }
    }
}

void AsteroidUpdateLoop() {
    if (debugging) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 randomVelocity = Vector2Rotate({ (float)GetRandomValue(100, 200), 0.0f }, DEG2RAD * (float)GetRandomValue(0, 360));

            AsteroidSize validSizes[] = { SMALL, MEDIUM, LARGE };
            int randomIndex = GetRandomValue(0, 2);
            AsteroidSize randomSize = validSizes[randomIndex];

            AddAsteroid(GetMousePosition(), randomVelocity, randomSize);
        }
    }

    AsteroidSpawnTimer(GetFrameTime());
    CheckLaserAsteroidCollisions();
    CheckAsteroidAsteroidCollisions();
    CheckAsteroidPlayerCollisions(GetFrameTime());
    CheckAsteroidEnemyCollisions(GetFrameTime());

    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        Asteroid* asteroid = &asteroids[i];
        if (asteroid->active) {
            asteroid->age += GetFrameTime();
            if (asteroid->collisionDamageCooldown > 0.0f) {
                asteroid->collisionDamageCooldown -= GetFrameTime();
                if (asteroid->collisionDamageCooldown < 0.0f) {
                    asteroid->collisionDamageCooldown = 0.0f;
                }
            }
            AsteroidUpdate(asteroid, GetFrameTime());
            CheckAsteroidYBounds(i);
        }
    }
}

void AsteroidDrawAll() {
    if (debugging) {
        DrawText("Asteroid Loop Running", gameWidth - MeasureText("Asteroid Loop Running", 20), 10, 20, GREEN);
    }

    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (asteroids[i].active) {
            AsteroidDraw(asteroids[i]);
        }
    }
}
