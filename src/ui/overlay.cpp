#include "raylib.h"
#include "overlay.hpp"
#include "globals.hpp"
#include "game.hpp"
#include "ship.hpp"
#include "player.hpp"
#include "enemies.hpp"
#include "asteroids.hpp"
#include <cstdio>

HealthBar::HealthBar(Vector2 pos, OverlaySize overlaySize)
    : position(pos), size(overlaySize) {
    switch (size) {
        case OverlaySize::SMALL:
            width = 100;
            backWidth = 110;
            height = 10;
            backHeight = 14;
            position = {gameWidth - width - 20 , gameHeight - height - 10}; // Position at bottom right
            break;
        case OverlaySize::MEDIUM:
            width = 200;
            backWidth = 210;
            height = 20;
            backHeight = 24;
            position = {gameWidth - width - 20 , gameHeight - height - 10}; // Position at bottom right
            break;
        case OverlaySize::LARGE:
            width = 300;
            backWidth = 310;
            height = 30;
            backHeight = 34;
            position = {gameWidth - width - 20 , gameHeight - height - 10}; // Position at bottom right
            break;
    }
    cornerRadius = 45;
}

BoostBar::BoostBar(Vector2 pos, OverlaySize overlaySize)
{
    position = pos;
    size = overlaySize;
    switch (size) {
        case OverlaySize::SMALL:
            width = 50;
            backWidth = 60;
            height = 8;
            backHeight = 12;
            position = {gameWidth - width - 20 , gameHeight - height - 10 - 10 - 20};  // Position at bottom right
            break;
        case OverlaySize::MEDIUM:
            width = 100;
            backWidth = 110;
            height = 16;
            backHeight = 20;
            position = {gameWidth - width - 20 , gameHeight - height - 10 - 10 - 20};  // Position at bottom right
            break;
        case OverlaySize::LARGE:
            width = 200;
            backWidth = 210;
            height = 24;
            backHeight = 30;
            position = {gameWidth - width - 20 , gameHeight - height - 10 - 10 - 20}; // Position at bottom right
            break;
    }
    cornerRadius = 45;
}

LaserBar::LaserBar(Vector2 pos, OverlaySize overlaySize)
{
    position = pos;
    size = overlaySize;
    switch (size) {
        case OverlaySize::SMALL:
            width = 50;
            backWidth = 60;
            height = 8;
            backHeight = 12;
            position = {gameWidth - width - 20 , gameHeight - height - 10 - 10 - 5 - 20};  // Position at bottom right
            break;
        case OverlaySize::MEDIUM:
            width = 100;
            backWidth = 110;
            height = 16;
            backHeight = 20;
            position = {gameWidth - width - 20 , gameHeight - height - 10 - 10 - 5 - 40};  // Position at bottom right
            break;
        case OverlaySize::LARGE:
            width = 200;
            backWidth = 210;
            height = 24;
            backHeight = 30;
            position = {gameWidth - width - 20 , gameHeight - height - 10 - 10 - 5 - 20}; // Position at bottom right
            break;
    }
    cornerRadius = 45;
}
DirectionPointer::DirectionPointer(Vector2 pos, OverlaySize overlaySize, Color col)
    : position(pos), size(overlaySize), color(col)
{
    // Initialize your points relative to (0,0) origin — example arrow pointing UP:
    point1 = {0, -32 + 10};   // tip of arrow
    point2 = {-15, 0 + 10};   // bottom left
    point3 = {15, 0 + 10};    // bottom right
    position = {static_cast<float>( gameWidth - 60 ), static_cast<float> (gameHeight - 110)};
    // If you want, adjust points based on overlaySize here (optional)
}

ScoreDisplay::ScoreDisplay(Vector2 pos, OverlaySize overlaySize)
{
    position = pos;
    size = overlaySize;
    switch (size) {
        case OverlaySize::SMALL:
            width = 100;
            height = 20;
            break;
        case OverlaySize::MEDIUM:
            width = 150;
            height = 30;
            break;
        case OverlaySize::LARGE:
            width = 200;
            height = 40;
            break;
    }
    position.x = 10; // Position at top left
    position.y = 10; // Position at top left
}
static void FormatOverlayMinutesSeconds(float seconds, char* buffer, size_t bufferSize) {
    int totalSec = (int)seconds;
    if (totalSec < 0) totalSec = 0;
    snprintf(buffer, bufferSize, "%d:%02d", totalSec / 60, totalSec % 60);
}

static void FormatOverlayScore(int value, char* buffer, size_t bufferSize) {
    char raw[16];
    snprintf(raw, sizeof(raw), "%d", value);

    size_t rawLen = 0;
    while (raw[rawLen] != '\0') rawLen++;

    size_t commaCount = rawLen > 3 ? (rawLen - 1) / 3 : 0;
    size_t outLen = rawLen + commaCount;
    if (outLen + 1 > bufferSize) {
        snprintf(buffer, bufferSize, "%d", value);
        return;
    }

    size_t outIdx = outLen;
    buffer[outIdx] = '\0';
    for (size_t i = 0; i < rawLen; i++) {
        buffer[--outIdx] = raw[rawLen - 1 - i];
        if (i % 3 == 2 && i + 1 < rawLen) {
            buffer[--outIdx] = ',';
        }
    }
}

void ScoreDisplay::Draw(int score) const
{
    Color backboxColor = { 35, 35, 35, 150 };
    DrawRectangle(position.x, position.y, width, height, backboxColor);
    DrawText(TextFormat("Score: %d", score), position.x + 10, position.y + 5, 20, WHITE);
}

void ScoreDisplay::DrawWithGoals(int score, float timeSeconds, int clears) const
{
    const int mainFontSize = 20;
    const int milestoneFontSize = 14;
    const float milestoneRowHeight = 18.0f;
    Color backboxColor = { 35, 35, 35, 150 };

    char timeCurrent[16];
    char timeGoal[16];
    char scoreCurrent[16];
    char scoreGoal[16];
    FormatOverlayMinutesSeconds(timeSeconds, timeCurrent, sizeof(timeCurrent));
    FormatOverlayMinutesSeconds(ASTEROIDS_VICTORY_TIME_SEC, timeGoal, sizeof(timeGoal));
    FormatOverlayScore(score, scoreCurrent, sizeof(scoreCurrent));
    FormatOverlayScore(ASTEROIDS_VICTORY_SCORE, scoreGoal, sizeof(scoreGoal));

    const char* milestone = TextFormat(
        "%s / %s · %s / %s · %d / %d",
        timeCurrent, timeGoal,
        scoreCurrent, scoreGoal,
        clears, ASTEROIDS_VICTORY_CLEARS
    );

    float boxWidth = (float)MeasureText(milestone, milestoneFontSize) + 20.0f;
    if (boxWidth < width) boxWidth = width;
    float boxHeight = height + milestoneRowHeight;

    DrawRectangle(position.x, position.y, boxWidth, boxHeight, backboxColor);
    DrawText(TextFormat("Score: %d", score), position.x + 10, position.y + 5, mainFontSize, WHITE);

    int milestoneX = position.x + 10;
    int milestoneY = position.y + (int)height - 2;
    DrawText(milestone, milestoneX, milestoneY, milestoneFontSize, (Color){ 120, 125, 140, 220 });
}

KillsDisplay::KillsDisplay(Vector2 pos, OverlaySize overlaySize)
{
    position = pos;
    size = overlaySize;
    switch (size) {
        case OverlaySize::SMALL:
            width = 120;
            height = 20;
            break;
        case OverlaySize::MEDIUM:
            width = 170;
            height = 30;
            break;
        case OverlaySize::LARGE:
            width = 220;
            height = 40;
            break;
    }
    position.x = 10;
    position.y = 10;
}

void KillsDisplay::Draw(int kills) const
{
    Color backboxColor = { 35, 35, 35, 150 };
    DrawRectangle(position.x, position.y, width, height, backboxColor);
    DrawText(TextFormat("Kills: %d", kills), position.x + 10, position.y + 5, 20, WHITE);
}
TimeDisplay::TimeDisplay(Vector2 pos, OverlaySize overlaySize)
{
    position = pos;
    size = overlaySize;
    switch (size) {
        case OverlaySize::SMALL:
            width = 100;
            height = 20;
            break;
        case OverlaySize::MEDIUM:
            width = 150;
            height = 30;
            break;
        case OverlaySize::LARGE:
            width = 200;
            height = 40;
            break;
    }
    position.x = 10;
    position.y = pos.y > 0.0f ? pos.y : height + 20.0f;
}
void TimeDisplay::Draw(float time) const{
    Color backboxColor = { 35, 35, 35, 150 }; // Semi-transparent black background
    DrawRectangle(position.x, position.y, width, height, backboxColor);
    DrawText(TextFormat("Time: %.1f", currentRun.time), position.x + 10, position.y + 5, 20, WHITE);
}

WaveDisplay::WaveDisplay(Vector2 pos, OverlaySize overlaySize)
{
    position = pos;
    size = overlaySize;
    switch (size) {
        case OverlaySize::SMALL:
            width = 100;
            height = 20;
            break;
        case OverlaySize::MEDIUM:
            width = 130;
            height = 30;
            break;
        case OverlaySize::LARGE:
            width = 160;
            height = 40;
            break;
    }
    position.x = 10;
    position.y = 50;
}

void WaveDisplay::Draw(int wave) const
{
    Color backboxColor = { 35, 35, 35, 150 };
    const int mainFontSize = 20;
    const int milestoneFontSize = 14;
    const float milestoneRowHeight = 18.0f;

    if (IsAsteroidFieldWave()) {
        const char* label = "Asteroid Field";
        Color labelColor = { 180, 200, 255, 255 };
        float boxWidth = 170.0f;
        float boxHeight = height + milestoneRowHeight;

        DrawRectangle(position.x, position.y, boxWidth, boxHeight, backboxColor);
        DrawText(label, position.x + 10, position.y + 5, mainFontSize, labelColor);

        const char* milestone = TextFormat("Boss %d · Finale %d", RIFT_BOSS_WAVE, RIFT_FINALE_WAVE);
        int milestoneWidth = MeasureText(milestone, milestoneFontSize);
        int milestoneX = position.x + 10;
        int milestoneY = position.y + (int)height - 2;
        DrawText(milestone, milestoneX, milestoneY, milestoneFontSize, (Color){ 120, 125, 140, 220 });
        return;
    }

    const char* label = TextFormat("Wave %d", wave);
    Color labelColor = { 255, 210, 80, 255 };

    bool showMilestones = wave < RIFT_FINALE_WAVE;
    float boxHeight = showMilestones ? height + milestoneRowHeight : height;
    float boxWidth = showMilestones ? 210.0f : width;

    DrawRectangle(position.x, position.y, boxWidth, boxHeight, backboxColor);
    DrawText(label, position.x + 10, position.y + 5, mainFontSize, labelColor);

    if (!showMilestones) return;

    const int markerX = position.x + 10;
    const int markerY = position.y + (int)height - 2;
    const int markerGap = 8;
    const char* bossMarker = TextFormat("Boss %d", RIFT_BOSS_WAVE);
    const char* finaleMarker = TextFormat("Finale %d", RIFT_FINALE_WAVE);
    int bossWidth = MeasureText(bossMarker, milestoneFontSize);
    int sepWidth = MeasureText(" · ", milestoneFontSize);

    bool bossReached = wave >= RIFT_BOSS_WAVE;
    bool finaleNext = wave >= RIFT_BOSS_WAVE;
    float pulse = 0.65f + 0.35f * sinf((float)GetTime() * 4.0f);

    Color bossColor = bossReached
        ? (Color){ 100, 180, 120, 200 }
        : (Color){ (unsigned char)(255 * pulse), (unsigned char)(150 * pulse), (unsigned char)(90 * pulse), 255 };
    Color finaleColor = finaleNext
        ? (Color){ (unsigned char)(180 * pulse), (unsigned char)(220 * pulse), 255, 255 }
        : (Color){ 120, 125, 140, 220 };

    DrawText(bossMarker, markerX, markerY, milestoneFontSize, bossColor);
    DrawText(" · ", markerX + bossWidth, markerY, milestoneFontSize, (Color){ 90, 90, 100, 180 });
    DrawText(finaleMarker, markerX + bossWidth + sepWidth, markerY, milestoneFontSize, finaleColor);
}

ScrapDisplay::ScrapDisplay(Vector2 pos, OverlaySize overlaySize)
{
    position = pos;
    size = overlaySize;
    switch (size) {
        case OverlaySize::SMALL:
            width = 120;
            height = 20;
            break;
        case OverlaySize::MEDIUM:
            width = 150;
            height = 30;
            break;
        case OverlaySize::LARGE:
            width = 180;
            height = 40;
            break;
    }
    position.x = (float)gameWidth - width - 10.0f;
    position.y = 10.0f;
}

void ScrapDisplay::Draw(int scrap) const
{
    Color backboxColor = { 35, 35, 35, 150 };
    DrawRectangle(position.x, position.y, width, height, backboxColor);
    DrawText(TextFormat("Scrap: %d", scrap), position.x + 10, position.y + 5, 20, (Color){ 210, 170, 110, 255 });
}

void HealthBar::Draw(float lifebar) const
{
    float filledWidth = width * lifebar;
    DrawRectangleRounded({position.x, position.y, backWidth, backHeight},
                         cornerRadius / width, 0, DARKGRAY); // background
    DrawRectangleRounded({position.x + ((backWidth - width)/2)/2 ,
                         position.y + ((backHeight - height)/2)/2 ,
                         (backWidth - width)/2 + filledWidth,
                         (backHeight - height)/2 + height},
                         cornerRadius / width, 0, MAROON); // health
}
void BoostBar::Draw(float boostbar, bool isStalled) const
{
    float filledBoost = width * boostbar;
    float time = GetTime();
    Color barColor = isStalled ? (fmod(time * 4, 2) < 1 ? RED : MAROON) : GREEN;

    DrawRectangleRounded(
        {position.x, position.y, backWidth, backHeight},
        cornerRadius / width, 0, DARKGRAY); // background

    DrawRectangleRounded(
        {position.x + ((backWidth - width) / 2) / 2,
         position.y + ((backHeight - height) / 2) / 2,
         (backWidth - width) / 2 + filledBoost,
         (backHeight - height) / 2 + height},
        cornerRadius / width, 0, barColor); // boost bar

}
void LaserBar::Draw(float timeSinceLastShot, float shotCooldown) const
{
    float filledWidth = ((timeSinceLastShot / shotCooldown)) * width;
    DrawRectangleRounded({position.x, position.y, backWidth, backHeight},
                         cornerRadius / width, 0, DARKGRAY); // background
    DrawRectangleRounded({position.x + ((backWidth - width) / 2) / 2,
                         position.y + ((backHeight - height) / 2) / 2,
                         (backWidth - width) / 2 + filledWidth,
                         (backHeight - height) / 2 + height},
                         cornerRadius / width, 0, BLUE); // laser bar
}
void DirectionPointer::Draw(float direction) const {
    // Convert direction to radians
    float rad = direction * (PI / 180.0f);

    auto RotatePoint = [rad](Vector2 p) -> Vector2 {
        return Vector2{
            p.x * cosf(rad) - p.y * sinf(rad),
            p.x * sinf(rad) + p.y * cosf(rad)
        };
    };
    // Rotate your stored points around origin
    Vector2 rPoint1 = RotatePoint(point1);
    Vector2 rPoint2 = RotatePoint(point2);
    Vector2 rPoint3 = RotatePoint(point3);

    // Translate rotated points to arrow position
    rPoint1.x += position.x;
    rPoint1.y += position.y;

    rPoint2.x += position.x;
    rPoint2.y += position.y;

    rPoint3.x += position.x;
    rPoint3.y += position.y;

    DrawTriangle(rPoint1, rPoint2, rPoint3, color);
}

// You can move this health bar outside to be used across frames
static HealthBar healthBar({10, 10}, OverlaySize::MEDIUM);
static BoostBar boostBar({10, 40}, OverlaySize::MEDIUM);
static DirectionPointer directionPointer({50, 50}, OverlaySize::MEDIUM, RED);
static ScoreDisplay scoreDisplay({10, 10}, OverlaySize::MEDIUM);
static KillsDisplay killsDisplay({10, 10}, OverlaySize::MEDIUM);
static WaveDisplay waveDisplay({10, 50}, OverlaySize::MEDIUM);
static TimeDisplay timeDisplay({10, 108}, OverlaySize::MEDIUM);
static ScrapDisplay scrapDisplay({0, 10}, OverlaySize::MEDIUM);


void DrawGameOverlay(const Player& player, float life, float boost, bool isStalled, int score, int scrap, float time)
{
    healthBar.Draw(life);
    boostBar.Draw(boost, isStalled);

    if (asteroidsMode) {
        scoreDisplay.DrawWithGoals(score, time, GetAsteroidsCleared());
        scrapDisplay.Draw(scrap);
        timeDisplay.Draw(time);
    } else if (enemiesOn) {
        killsDisplay.Draw(score);
        waveDisplay.Draw(GetEnemyWave());
        scrapDisplay.Draw(scrap);
        timeDisplay.Draw(time);
        if (game.endlessMode) {
            const char* endlessLabel = "ENDLESS";
            int labelWidth = MeasureText(endlessLabel, 12);
            DrawText(endlessLabel, gameWidth - labelWidth - 14, 88, 12, (Color){ 255, 210, 80, 255 });
        }
    }
    if ((!player.bellowY) && (directionPointerOn)) {
        directionPointer.Draw(player.direction);
    }
    if (player.bellowY && !yWrapping) {
        const char* warning = "Return to the rift!";
        const int fontSize = 24;
        const int textWidth = MeasureText(warning, fontSize);
        const int x = gameWidth / 2 - textWidth / 2;
        const int y = 36;

        float pulse = 0.75f + 0.25f * sinf((float)GetTime() * 5.0f);
        Color warnColor = {
            (unsigned char)(255 * pulse),
            (unsigned char)(60 * pulse),
            (unsigned char)(60 * pulse),
            255
        };

        DrawRectangle(x - 14, y - 8, textWidth + 28, fontSize + 16, (Color){ 25, 0, 0, 190 });
        DrawText(warning, x, y, fontSize, warnColor);
    }
    if (debugging){

        if (debugTimer >= 10.0f) {
            printf("Player direction: %.2f\n", player.direction);
            debugTimer = 0.0f; // Reset timer
        } else {
            debugTimer += GetFrameTime();
        }
    }
}