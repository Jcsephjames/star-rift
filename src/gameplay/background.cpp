#include "raylib.h"
#include "background.hpp"
#include "globals.hpp"

Stars::Stars()
{
    stars.resize(starCount);
    for (int i = 0; i < starCount; ++i) {
        stars[i].x = GetRandomValue(-5, gameWidth + 5);
        stars[i].y = GetRandomValue(0, gameHeight);
        stars[i].speed = GetRandomValue(200, 250) / 100.0f; // Speed between 0.5 and 1.5
        stars[i].size = GetRandomValue(1, 3); // Size between 1 and 3
    }
}
void Stars::Update(float deltaTime)
{
    for (auto& star : stars) {
        star.y += star.speed * star.size * deltaTime;
        if (star.y > gameHeight) {
            star.y = 0;
            star.x = GetRandomValue(0, gameWidth);
        }
    }
}
void Stars::Draw()
{
    for (const auto& star : stars) {
        DrawCircle(star.x, static_cast<int>(star.y), star.size, WHITE);
    }
}