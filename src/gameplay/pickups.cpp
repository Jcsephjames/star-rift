#include "pickups.hpp"
#include "globals.hpp"
#include "audio.hpp"
#include "player.hpp"
#include "runModifiers.hpp"
#include <raymath.h>
#include <cmath>

static HealthPickup pickups[MAX_PICKUPS] = { 0 };

static Color PickupColor(float healthValue) {
    if (healthValue >= 50.0f) return (Color){ 255, 215, 80, 255 };
    if (healthValue >= 15.0f) return (Color){ 180, 80, 255, 255 };
    if (healthValue >= 10.0f) return (Color){ 80, 180, 255, 255 };
    return (Color){ 80, 255, 140, 255 };
}

static void ResetPickupSparks(HealthPickup* pickup) {
    for (int i = 0; i < SPARKS_PER_PICKUP; i++) {
        pickup->sparks[i] = { false };
    }
}

static void EmitSpark(HealthPickup* pickup) {
    PickupSpark* spark = nullptr;
    for (int i = 0; i < SPARKS_PER_PICKUP; i++) {
        if (!pickup->sparks[i].active) {
            spark = &pickup->sparks[i];
            break;
        }
    }
    if (!spark) return;

    float angle = DEG2RAD * (float)GetRandomValue(0, 360);
    float speed = (float)GetRandomValue(36, 90);
    spark->active = true;
    spark->position = pickup->position;
    spark->velocity = { cosf(angle) * speed, sinf(angle) * speed };
    spark->maxLifetime = 0.35f + (float)GetRandomValue(0, 15) / 100.0f;
    spark->lifetime = spark->maxLifetime;
}

void InitPickups() {
    for (int i = 0; i < MAX_PICKUPS; i++) {
        pickups[i] = { false };
    }
}

void SpawnHealthPickup(Vector2 position, float healthValue, Vector2 driftVelocity) {
    for (int i = 0; i < MAX_PICKUPS; i++) {
        if (pickups[i].active) continue;

        pickups[i] = {
            .active = true,
            .position = position,
            .velocity = driftVelocity,
            .healthValue = healthValue,
            .lifetime = 12.0f,
            .sparkTimer = 0.0f
        };
        ResetPickupSparks(&pickups[i]);
        return;
    }
}

void UpdatePickups(float deltaTime) {
    for (int i = 0; i < MAX_PICKUPS; i++) {
        HealthPickup* pickup = &pickups[i];
        if (!pickup->active) continue;

        pickup->lifetime -= deltaTime;
        pickup->position.x += pickup->velocity.x * deltaTime;
        pickup->position.y += pickup->velocity.y * deltaTime;
        pickup->velocity = Vector2Scale(pickup->velocity, 0.985f);

        pickup->sparkTimer += deltaTime;
        if (pickup->sparkTimer >= 0.07f) {
            pickup->sparkTimer = 0.0f;
            EmitSpark(pickup);
        }

        for (int s = 0; s < SPARKS_PER_PICKUP; s++) {
            PickupSpark* spark = &pickup->sparks[s];
            if (!spark->active) continue;

            spark->lifetime -= deltaTime;
            if (spark->lifetime <= 0.0f) {
                spark->active = false;
                continue;
            }

            spark->position.x += spark->velocity.x * deltaTime;
            spark->position.y += spark->velocity.y * deltaTime;
            spark->velocity = Vector2Scale(spark->velocity, 0.92f);
        }

        if (pickup->lifetime <= 0.0f) {
            pickup->active = false;
            continue;
        }

        float baseRadius = 28.0f + pickup->healthValue * 0.08f;
        float collectRadius = GetEffectivePickupRadius(baseRadius);
        if (CheckCollisionPointCircle(player.position, pickup->position, collectRadius)) {
            float healAmount = GetEffectiveHealthPickupAmount(pickup->healthValue);
            player.life = fminf(player.life + healAmount, player.maxHealth);
            PlaySfx(SfxId::HEALTH_PICKUP);
            pickup->active = false;
        }
    }
}

void DrawPickups() {
    for (int i = 0; i < MAX_PICKUPS; i++) {
        HealthPickup* pickup = &pickups[i];
        if (!pickup->active) continue;

        Color color = PickupColor(pickup->healthValue);
        float alpha = fminf(pickup->lifetime / 2.0f, 1.0f);
        float size = 7.0f + fminf(pickup->healthValue * 0.05f, 6.0f);
        Vector2 center = pickup->position;

        for (int s = 0; s < SPARKS_PER_PICKUP; s++) {
            PickupSpark* spark = &pickup->sparks[s];
            if (!spark->active) continue;

            float sparkAlpha = (spark->lifetime / spark->maxLifetime) * alpha;
            DrawCircleV(
                spark->position,
                2.4f + spark->lifetime * 3.0f,
                Fade(color, sparkAlpha * 0.75f)
            );
        }

        Vector2 points[4] = {
            { center.x, center.y - size },
            { center.x + size, center.y },
            { center.x, center.y + size },
            { center.x - size, center.y }
        };

        DrawLineEx(points[0], points[1], 2.5f, Fade(color, alpha));
        DrawLineEx(points[1], points[2], 2.5f, Fade(color, alpha));
        DrawLineEx(points[2], points[3], 2.5f, Fade(color, alpha));
        DrawLineEx(points[3], points[0], 2.5f, Fade(color, alpha));
        DrawCircleV(center, 3.0f, Fade(color, alpha * 0.9f));
    }
}
