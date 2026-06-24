#include "engineExhaust.hpp"
#include <raymath.h>
#include <cmath>

#define MAX_EXHAUST_PARTICLES 640

struct ExhaustParticle {
    bool active;
    Vector2 position;
    Vector2 velocity;
    float lifetime;
    float maxLifetime;
    float size;
    float rotation;
    Color color;
};

static ExhaustParticle particles[MAX_EXHAUST_PARTICLES];

static Vector2 DirFromAngle(float degrees) {
    float rad = (degrees - 90.0f) * DEG2RAD;
    return { cosf(rad), sinf(rad) };
}

static ExhaustParticle* AcquireParticle() {
    for (int i = 0; i < MAX_EXHAUST_PARTICLES; i++) {
        if (!particles[i].active) return &particles[i];
    }
    return nullptr;
}

static Color PickExhaustColor(bool neon, bool ion) {
    if (ion) {
        Color palette[] = {
            { 120, 220, 255, 230 },
            { 80, 180, 255, 220 },
            { 200, 240, 255, 210 },
            { 60, 140, 255, 200 }
        };
        return palette[GetRandomValue(0, 3)];
    }

    if (neon) {
        Color palette[] = {
            { 80, 255, 230, 230 },
            { 160, 60, 255, 210 },
            { 60, 200, 255, 200 },
            { 255, 80, 220, 190 }
        };
        return palette[GetRandomValue(0, 3)];
    }

    Color palette[] = {
        { 255, 250, 210, 230 },
        { 255, 220, 90, 220 },
        { 255, 150, 40, 200 },
        { 255, 90, 30, 180 }
    };
    return palette[GetRandomValue(0, 3)];
}

static void SpawnExhaustParticle(Vector2 origin, Vector2 direction, float scale,
                                 float intensity, float speed, bool neon, bool ion) {
    ExhaustParticle* particle = AcquireParticle();
    if (!particle) return;

    float spread = (float)GetRandomValue(-28, 28) * DEG2RAD;
    Vector2 dir = {
        direction.x * cosf(spread) - direction.y * sinf(spread),
        direction.x * sinf(spread) + direction.y * cosf(spread)
    };

    float lateral = (float)GetRandomValue(-12, 12);
    Vector2 perp = { -dir.y, dir.x };
    Vector2 velocity = Vector2Add(
        Vector2Scale(dir, speed),
        Vector2Scale(perp, lateral)
    );

    particle->active = true;
    particle->position = origin;
    particle->velocity = velocity;
    particle->maxLifetime = 0.22f + (float)GetRandomValue(0, 35) / 100.0f;
    particle->lifetime = particle->maxLifetime;
    particle->size = (0.6f + (float)GetRandomValue(0, 8) / 10.0f) * scale * (0.55f + intensity * 0.35f);
    particle->rotation = atan2f(velocity.y, velocity.x) * RAD2DEG + (float)GetRandomValue(-25, 25);
    particle->color = PickExhaustColor(neon, ion);
}

static void EmitFromPoint(Vector2 origin, Vector2 exhaustDir, Vector2 lateral,
                          const EngineExhaustParams& params, int count, float speedScale) {
    for (int i = 0; i < count; i++) {
        Vector2 jitter = Vector2Scale(lateral, (float)GetRandomValue(-2, 2) * params.scale);
        Vector2 spawnPos = Vector2Add(origin, jitter);
        float speed = (18.0f + (float)GetRandomValue(0, 28)) * params.intensity * speedScale;
        SpawnExhaustParticle(spawnPos, exhaustDir, params.scale, params.intensity, speed,
                             params.neonColors, params.ionColors);
    }
}

void InitEngineExhaust() {
    for (int i = 0; i < MAX_EXHAUST_PARTICLES; i++) {
        particles[i] = { false };
    }
}

void EmitEngineExhaust(const EngineExhaustParams& params) {
    if (params.intensity < 0.04f) return;

    Vector2 forward = DirFromAngle(params.direction);
    Vector2 rear = Vector2Scale(forward, -1.0f);
    Vector2 right = { forward.y, -forward.x };

    int mainCount = (int)(1 + params.intensity * 1.6f * params.boostFactor);
    if (params.boostFactor > 1.15f) {
        mainCount += 1;
    }

    Vector2 mainOrigin = Vector2Add(params.position, Vector2Scale(rear, 6.0f * params.scale));
    EmitFromPoint(mainOrigin, rear, right, params, mainCount, params.boostFactor);

    float turnNorm = fmaxf(-1.0f, fminf(1.0f, params.turnRate / 220.0f));
    if (fabsf(turnNorm) > 0.07f) {
        float sideIntensity = fabsf(turnNorm) * params.intensity;
        int sideCount = (int)(sideIntensity * 1.5f);
        if (sideCount < 1) sideCount = 1;

        if (turnNorm > 0.0f) {
            Vector2 leftSide = Vector2Subtract(params.position, Vector2Scale(right, 5.0f * params.scale));
            leftSide = Vector2Add(leftSide, Vector2Scale(rear, 2.5f * params.scale));
            EmitFromPoint(leftSide, Vector2Scale(right, -1.0f), rear, params, sideCount, 0.75f);
        } else {
            Vector2 rightSide = Vector2Add(params.position, Vector2Scale(right, 5.0f * params.scale));
            rightSide = Vector2Add(rightSide, Vector2Scale(rear, 2.5f * params.scale));
            EmitFromPoint(rightSide, right, rear, params, sideCount, 0.75f);
        }
    }
}

void UpdateEngineExhaust(float deltaTime) {
    for (int i = 0; i < MAX_EXHAUST_PARTICLES; i++) {
        ExhaustParticle* particle = &particles[i];
        if (!particle->active) continue;

        particle->lifetime -= deltaTime;
        if (particle->lifetime <= 0.0f) {
            particle->active = false;
            continue;
        }

        particle->position.x += particle->velocity.x * deltaTime;
        particle->position.y += particle->velocity.y * deltaTime;
        particle->velocity = Vector2Scale(particle->velocity, 0.90f);
    }
}

void DrawAllEngineExhaust() {
    for (int i = 0; i < MAX_EXHAUST_PARTICLES; i++) {
        ExhaustParticle* particle = &particles[i];
        if (!particle->active) continue;

        float alpha = particle->lifetime / particle->maxLifetime;
        float drawSize = particle->size * (0.5f + alpha * 0.5f);
        Rectangle rect = {
            particle->position.x - drawSize,
            particle->position.y - drawSize,
            drawSize * 2.0f,
            drawSize * 2.0f
        };
        DrawRectanglePro(
            rect,
            { drawSize, drawSize },
            particle->rotation,
            Fade(particle->color, alpha * 0.6f)
        );
    }
}
