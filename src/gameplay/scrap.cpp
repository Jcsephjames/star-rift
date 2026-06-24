#include "scrap.hpp"
#include "audio.hpp"
#include "globals.hpp"
#include "player.hpp"
#include "effects.hpp"
#include "cardDraft.hpp"
#include "runModifiers.hpp"
#include <raymath.h>
#include <cmath>

static ScrapPickup scrapPickups[MAX_SCRAP_PICKUPS] = { 0 };

static void ResetScrapEmbers(ScrapPickup* scrap) {
    for (int i = 0; i < EMBERS_PER_SCRAP; i++) {
        scrap->embers[i] = { false };
    }
}

static void EmitScrapEmber(ScrapPickup* scrap) {
    ScrapEmber* ember = nullptr;
    for (int i = 0; i < EMBERS_PER_SCRAP; i++) {
        if (!scrap->embers[i].active) {
            ember = &scrap->embers[i];
            break;
        }
    }
    if (!ember) return;

    float angle = DEG2RAD * (scrap->rotation + (float)GetRandomValue(-40, 40));
    float speed = (float)GetRandomValue(20, 55);
    ember->active = true;
    ember->position = scrap->position;
    ember->velocity = {
        cosf(angle) * speed + scrap->velocity.x * 0.15f,
        sinf(angle) * speed + scrap->velocity.y * 0.15f
    };
    ember->maxLifetime = 0.25f + (float)GetRandomValue(0, 20) / 100.0f;
    ember->lifetime = ember->maxLifetime;
}

int GetAsteroidScrapDrop(AsteroidSize size) {
    switch (size) {
        case SMALL: return 5;
        case MEDIUM: return 11;
        case LARGE: return 23;
    }
    return 3;
}

void AddScrap(int amount) {
    if (amount <= 0) return;
    currentRun.scrap += GetBonusScrap(amount);
    CheckCardDraftTrigger();
}

void InitScrapPickups() {
    for (int i = 0; i < MAX_SCRAP_PICKUPS; i++) {
        scrapPickups[i] = { false };
    }
}

void SpawnScrapPickup(Vector2 position, int amount, Vector2 driftVelocity) {
    if (amount <= 0) return;

    for (int i = 0; i < MAX_SCRAP_PICKUPS; i++) {
        if (scrapPickups[i].active) continue;

        scrapPickups[i] = {
            .active = true,
            .position = position,
            .velocity = driftVelocity,
            .amount = amount,
            .lifetime = 14.0f,
            .rotation = (float)GetRandomValue(0, 360),
            .spinSpeed = (float)GetRandomValue(-90, 90),
            .emberTimer = 0.0f
        };
        ResetScrapEmbers(&scrapPickups[i]);
        return;
    }
}

static void DrawScrapShard(Vector2 center, float size, float rotation, float alpha, int amount) {
    float rad = rotation * DEG2RAD;
    Color hull = { 165, 175, 190, (unsigned char)(255 * alpha) };
    Color edge = { 210, 130, 70, (unsigned char)(255 * alpha) };
    if (amount >= 15) {
        hull = { 190, 185, 160, (unsigned char)(255 * alpha) };
        edge = { 255, 210, 90, (unsigned char)(255 * alpha) };
    }

    Vector2 local[5] = {
        { 0.0f, -size },
        { size * 0.85f, -size * 0.15f },
        { size * 0.55f, size * 0.95f },
        { -size * 0.55f, size * 0.95f },
        { -size * 0.85f, -size * 0.15f }
    };

    Vector2 points[5];
    for (int i = 0; i < 5; i++) {
        float x = local[i].x;
        float y = local[i].y;
        points[i].x = center.x + x * cosf(rad) - y * sinf(rad);
        points[i].y = center.y + x * sinf(rad) + y * cosf(rad);
    }

    for (int i = 0; i < 5; i++) {
        int next = (i + 1) % 5;
        DrawLineEx(points[i], points[next], 2.0f, edge);
    }
    DrawLineEx(points[0], points[2], 1.0f, Fade(hull, alpha * 0.5f));
    DrawLineEx(points[1], points[3], 1.0f, Fade(hull, alpha * 0.5f));
    DrawCircleV(center, 2.0f, Fade(edge, alpha * 0.85f));
}

void UpdateScrapPickups(float deltaTime) {
    for (int i = 0; i < MAX_SCRAP_PICKUPS; i++) {
        ScrapPickup* scrap = &scrapPickups[i];
        if (!scrap->active) continue;

        scrap->lifetime -= deltaTime;
        scrap->position.x += scrap->velocity.x * deltaTime;
        scrap->position.y += scrap->velocity.y * deltaTime;
        scrap->velocity = Vector2Scale(scrap->velocity, 0.988f);
        scrap->rotation += scrap->spinSpeed * deltaTime;
        if (scrap->rotation >= 360.0f) scrap->rotation -= 360.0f;
        else if (scrap->rotation < 0.0f) scrap->rotation += 360.0f;

        scrap->emberTimer += deltaTime;
        if (scrap->emberTimer >= 0.09f) {
            scrap->emberTimer = 0.0f;
            EmitScrapEmber(scrap);
        }

        for (int e = 0; e < EMBERS_PER_SCRAP; e++) {
            ScrapEmber* ember = &scrap->embers[e];
            if (!ember->active) continue;

            ember->lifetime -= deltaTime;
            if (ember->lifetime <= 0.0f) {
                ember->active = false;
                continue;
            }

            ember->position.x += ember->velocity.x * deltaTime;
            ember->position.y += ember->velocity.y * deltaTime;
            ember->velocity = Vector2Scale(ember->velocity, 0.9f);
        }

        if (scrap->lifetime <= 0.0f) {
            scrap->active = false;
            continue;
        }

        float baseRadius = 28.0f + fminf((float)scrap->amount * 0.6f, 12.0f);
        float collectRadius = GetEffectivePickupRadius(baseRadius);
        if (CheckCollisionPointCircle(player.position, scrap->position, collectRadius)) {
            int totalScrap = GetBonusScrap(scrap->amount);
            AddScrap(scrap->amount);
            SpawnFloatingText(
                scrap->position,
                TextFormat("+%d Scrap", totalScrap),
                (Color){ 210, 170, 110, 255 }
            );
            PlaySfx(SfxId::SCRAP_PICKUP);
            scrap->active = false;
        }
    }
}

void DrawScrapPickups() {
    for (int i = 0; i < MAX_SCRAP_PICKUPS; i++) {
        ScrapPickup* scrap = &scrapPickups[i];
        if (!scrap->active) continue;

        float alpha = fminf(scrap->lifetime / 2.0f, 1.0f);
        float size = 5.0f + fminf((float)scrap->amount * 0.15f, 5.0f);

        for (int e = 0; e < EMBERS_PER_SCRAP; e++) {
            ScrapEmber* ember = &scrap->embers[e];
            if (!ember->active) continue;

            float emberAlpha = (ember->lifetime / ember->maxLifetime) * alpha;
            DrawCircleV(
                ember->position,
                1.5f + ember->lifetime * 2.0f,
                Fade((Color){ 255, 160, 60, 255 }, emberAlpha * 0.8f)
            );
        }

        DrawScrapShard(scrap->position, size, scrap->rotation, alpha, scrap->amount);
    }
}
