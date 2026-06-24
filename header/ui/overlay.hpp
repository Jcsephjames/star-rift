#pragma once
#ifndef OVERLAY_HPP
#define OVERLAY_HPP
#include "raylib.h"
#include "player.hpp"
#include "globals.hpp"
#include "game.hpp"

class Player;

enum class OverlaySize {
    SMALL,
    MEDIUM,
    LARGE
};

struct HealthBar {
    Vector2 position;
    float width;
    float backWidth;
    float height;
    float backHeight;
    float cornerRadius;
    OverlaySize size;

    HealthBar(Vector2 pos, OverlaySize overlaySize = OverlaySize::MEDIUM);

    void Draw(float health) const;
};
struct BoostBar {
    Vector2 position;
    float width;
    float backWidth;
    float height;
    float backHeight;
    float cornerRadius;
    OverlaySize size;

    BoostBar(Vector2 pos, OverlaySize overlaySize = OverlaySize::MEDIUM);

    void Draw(float boost, bool isStalled) const;
};

struct LaserBar {
    Vector2 position;
    float width;
    float backWidth;
    float height;
    float backHeight;
    float cornerRadius;
    OverlaySize size;

    LaserBar(Vector2 pos, OverlaySize overlaySize = OverlaySize::MEDIUM);

    void Draw(float timeSinceLastShot, float shotCooldown) const;
};

struct DirectionPointer {
    Vector2 position;
    OverlaySize size;
    Color color;

    Vector2 point1;
    Vector2 point2;
    Vector2 point3;

    DirectionPointer(Vector2 pos, OverlaySize overlaySize = OverlaySize::MEDIUM, Color col = RED);

    void Draw(float direction) const;
};

struct ScoreDisplay {
    Vector2 position;
    float width;
    float height;
    OverlaySize size;

    ScoreDisplay(Vector2 pos, OverlaySize overlaySize = OverlaySize::MEDIUM);

    void Draw(int score) const;
    void DrawWithGoals(int score, float timeSeconds, int clears) const;
};

struct KillsDisplay {
    Vector2 position;
    float width;
    float height;
    OverlaySize size;

    KillsDisplay(Vector2 pos, OverlaySize overlaySize = OverlaySize::MEDIUM);

    void Draw(int kills) const;
};

struct TimeDisplay {
    Vector2 position;
    float width;
    float height;
    OverlaySize size;

    TimeDisplay(Vector2 pos, OverlaySize overlaySize = OverlaySize::MEDIUM);

    void Draw(float time) const;
};

struct WaveDisplay {
    Vector2 position;
    float width;
    float height;
    OverlaySize size;

    WaveDisplay(Vector2 pos, OverlaySize overlaySize = OverlaySize::MEDIUM);

    void Draw(int wave) const;
};

struct ScrapDisplay {
    Vector2 position;
    float width;
    float height;
    OverlaySize size;

    ScrapDisplay(Vector2 pos, OverlaySize overlaySize = OverlaySize::MEDIUM);

    void Draw(int scrap) const;
};

void DrawGameOverlay(const Player& player, float life, float boost, bool isStalled, int score, int scrap, float time);




#endif // OVERLAY_HPP