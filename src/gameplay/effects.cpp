#include "effects.hpp"
#include "globals.hpp"
#include "audio.hpp"
#include <raymath.h>
#include <cmath>
#include <cstdio>

static Particle particles[MAX_PARTICLES];
static ImpactFlash impactFlashes[MAX_IMPACT_FLASHES];
static FloatingText floatingTexts[MAX_FLOATING_TEXTS];

static float shakeTimer = 0.0f;
static float shakeDuration = 0.0f;
static float shakeIntensity = 0.0f;

static struct EmpPulse {
    bool active;
    Vector2 origin;
    float radius;
    float maxRadius;
    float lifetime;
    float maxLifetime;
} empPulse = { false };

void InitEffects() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i] = { false };
    }
    for (int i = 0; i < MAX_IMPACT_FLASHES; i++) {
        impactFlashes[i] = { false };
    }
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) {
        floatingTexts[i] = { false };
    }
    shakeTimer = 0.0f;
    shakeDuration = 0.0f;
    shakeIntensity = 0.0f;
    empPulse = { false };
}

void TriggerEmpPulse(Vector2 origin) {
    PlaySfx(SfxId::EMP_PULSE);
    empPulse.active = true;
    empPulse.origin = origin;
    empPulse.radius = 0.0f;
    empPulse.maxRadius = hypotf((float)gameWidth, (float)gameHeight) * 0.85f;
    empPulse.lifetime = 0.55f;
    empPulse.maxLifetime = 0.55f;
}

static Particle* AcquireParticle() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) return &particles[i];
    }
    return nullptr;
}

static ImpactFlash* AcquireImpactFlash() {
    for (int i = 0; i < MAX_IMPACT_FLASHES; i++) {
        if (!impactFlashes[i].active) return &impactFlashes[i];
    }
    return nullptr;
}

void TriggerScreenShake(float intensity, float duration) {
    if (intensity > shakeIntensity) {
        shakeIntensity = intensity;
    }
    if (duration > shakeDuration) {
        shakeDuration = duration;
    }
    shakeTimer = shakeDuration;
}

void TriggerPlayerHitShake(float damage) {
    PlaySfx(SfxId::PLAYER_HIT);
    float intensity = fminf(4.0f + damage * 0.18f, 11.0f);
    float duration = 0.1f + fminf(damage * 0.004f, 0.12f);
    TriggerScreenShake(intensity, duration);
}

Camera2D GetShakeCamera() {
    Camera2D camera = { 0 };
    camera.offset = { gameWidth / 2.0f, gameHeight / 2.0f };
    camera.target = { gameWidth / 2.0f, gameHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    if (shakeTimer > 0.0f && shakeDuration > 0.0f) {
        float decay = shakeTimer / shakeDuration;
        float magnitude = shakeIntensity * decay;
        camera.target.x += (GetRandomValue(-100, 100) / 100.0f) * magnitude;
        camera.target.y += (GetRandomValue(-100, 100) / 100.0f) * magnitude;
    }

    return camera;
}

void SpawnExplosion(Vector2 position, AsteroidSize size, bool playSfx) {
    int particleCount = 0;
    float baseSpeed = 0.0f;
    float maxRadius = 0.0f;
    float shakePower = 0.0f;
    float shakeTime = 0.0f;

    switch (size) {
        case SMALL:
            particleCount = 8;
            baseSpeed = 120.0f;
            maxRadius = 4.0f;
            shakePower = 2.5f;
            shakeTime = 0.08f;
            break;
        case MEDIUM:
            particleCount = 14;
            baseSpeed = 160.0f;
            maxRadius = 6.0f;
            shakePower = 5.0f;
            shakeTime = 0.12f;
            break;
        case LARGE:
            particleCount = 22;
            baseSpeed = 220.0f;
            maxRadius = 9.0f;
            shakePower = 8.0f;
            shakeTime = 0.18f;
            break;
    }

    TriggerScreenShake(shakePower, shakeTime);
    if (playSfx) {
        PlaySfx(size == LARGE ? SfxId::EXPLOSION_LARGE : SfxId::EXPLOSION_SMALL);
    }

    Color colors[] = { WHITE, YELLOW, ORANGE, (Color){255, 120, 40, 255} };

    for (int i = 0; i < particleCount; i++) {
        Particle* particle = AcquireParticle();
        if (!particle) break;

        float angle = DEG2RAD * (360.0f * i / particleCount + GetRandomValue(-18, 18));
        float speed = baseSpeed + GetRandomValue(-40, 80);
        particle->active = true;
        particle->position = position;
        particle->velocity = { cosf(angle) * speed, sinf(angle) * speed };
        particle->maxLifetime = 0.25f + GetRandomValue(0, 20) / 100.0f;
        particle->lifetime = particle->maxLifetime;
        particle->radius = 1.5f + GetRandomValue(0, (int)(maxRadius * 10)) / 10.0f;
        particle->color = colors[GetRandomValue(0, 3)];
    }
}

void SpawnShipExplosion(Vector2 position) {
    PlaySfx(SfxId::SHIP_EXPLOSION);
    SpawnExplosion(position, SMALL, false);
    TriggerScreenShake(3.0f, 0.1f);
}

void SpawnLaserImpact(Vector2 position) {
    PlaySfx(SfxId::LASER_IMPACT);
    ImpactFlash* flash = AcquireImpactFlash();
    if (flash) {
        flash->active = true;
        flash->position = position;
        flash->maxLifetime = 0.08f;
        flash->lifetime = flash->maxLifetime;
        flash->radius = 10.0f;
    }

    for (int i = 0; i < 4; i++) {
        Particle* particle = AcquireParticle();
        if (!particle) break;

        float angle = DEG2RAD * GetRandomValue(0, 360);
        float speed = GetRandomValue(60, 140);
        particle->active = true;
        particle->position = position;
        particle->velocity = { cosf(angle) * speed, sinf(angle) * speed };
        particle->maxLifetime = 0.12f;
        particle->lifetime = particle->maxLifetime;
        particle->radius = 2.0f;
        particle->color = (GetRandomValue(0, 1) == 0) ? WHITE : YELLOW;
    }

    TriggerScreenShake(1.5f, 0.05f);
}

void SpawnFloatingText(Vector2 position, const char* text, Color color) {
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) {
        if (floatingTexts[i].active) continue;

        floatingTexts[i].active = true;
        floatingTexts[i].position = position;
        floatingTexts[i].lifetime = 0.9f;
        floatingTexts[i].maxLifetime = 0.9f;
        floatingTexts[i].color = color;
        snprintf(floatingTexts[i].text, sizeof(floatingTexts[i].text), "%s", text);
        return;
    }
}

void UpdateFloatingTexts(float deltaTime) {
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) {
        FloatingText* label = &floatingTexts[i];
        if (!label->active) continue;

        label->lifetime -= deltaTime;
        label->position.y -= 40.0f * deltaTime;
        if (label->lifetime <= 0.0f) {
            label->active = false;
        }
    }
}

void DrawFloatingTexts() {
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) {
        FloatingText* label = &floatingTexts[i];
        if (!label->active) continue;

        float alpha = label->lifetime / label->maxLifetime;
        int fontSize = 18;
        int textWidth = MeasureText(label->text, fontSize);
        DrawText(
            label->text,
            (int)(label->position.x - textWidth / 2.0f),
            (int)label->position.y,
            fontSize,
            Fade(label->color, alpha)
        );
    }
}

void UpdateEffects(float deltaTime) {
    UpdateFloatingTexts(deltaTime);

    if (empPulse.active) {
        empPulse.lifetime -= deltaTime;
        float progress = 1.0f - (empPulse.lifetime / empPulse.maxLifetime);
        empPulse.radius = empPulse.maxRadius * progress;
        if (empPulse.lifetime <= 0.0f) {
            empPulse.active = false;
        }
    }

    if (shakeTimer > 0.0f) {
        shakeTimer -= deltaTime;
        if (shakeTimer <= 0.0f) {
            shakeTimer = 0.0f;
            shakeDuration = 0.0f;
            shakeIntensity = 0.0f;
        }
    }

    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle* particle = &particles[i];
        if (!particle->active) continue;

        particle->lifetime -= deltaTime;
        if (particle->lifetime <= 0.0f) {
            particle->active = false;
            continue;
        }

        particle->position.x += particle->velocity.x * deltaTime;
        particle->position.y += particle->velocity.y * deltaTime;
        particle->velocity = Vector2Scale(particle->velocity, 0.92f);
    }

    for (int i = 0; i < MAX_IMPACT_FLASHES; i++) {
        ImpactFlash* flash = &impactFlashes[i];
        if (!flash->active) continue;

        flash->lifetime -= deltaTime;
        if (flash->lifetime <= 0.0f) {
            flash->active = false;
        }
    }
}

void DrawEffects() {
    if (empPulse.active) {
        float progress = 1.0f - (empPulse.lifetime / empPulse.maxLifetime);
        float fade = 1.0f - progress;
        Color empBlue = { 80, 180, 255, (unsigned char)(90 * fade) };
        Color empRing = { 120, 210, 255, (unsigned char)(160 * fade) };

        DrawCircleV(empPulse.origin, empPulse.radius, empBlue);
        DrawCircleLinesV(empPulse.origin, empPulse.radius, empRing);
        DrawCircleLinesV(empPulse.origin, empPulse.radius * 0.72f, Fade(empRing, fade * 0.6f));

        const char* label = "EMP";
        int fontSize = 18;
        int textW = MeasureText(label, fontSize);
        DrawText(
            label,
            (int)(empPulse.origin.x - textW / 2.0f),
            (int)(empPulse.origin.y - fontSize / 2.0f),
            fontSize,
            Fade((Color){ 180, 230, 255, 255 }, fade * 0.5f)
        );
    }

    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle* particle = &particles[i];
        if (!particle->active) continue;

        float alpha = particle->lifetime / particle->maxLifetime;
        DrawCircleV(
            particle->position,
            particle->radius * (0.5f + alpha * 0.5f),
            Fade(particle->color, alpha)
        );
    }

    for (int i = 0; i < MAX_IMPACT_FLASHES; i++) {
        ImpactFlash* flash = &impactFlashes[i];
        if (!flash->active) continue;

        float alpha = flash->lifetime / flash->maxLifetime;
        DrawCircleV(flash->position, flash->radius * (1.2f - alpha * 0.5f), Fade(WHITE, alpha * 0.85f));
        DrawCircleV(flash->position, flash->radius * 0.45f * alpha, Fade(YELLOW, alpha));
    }

    DrawFloatingTexts();
}
