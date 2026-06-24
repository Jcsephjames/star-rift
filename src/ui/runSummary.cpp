#include "runSummary.hpp"
#include "globals.hpp"
#include "game.hpp"
#include "enemies.hpp"
#include "cardDraft.hpp"
#include "runModifiers.hpp"
#include "metaProgression.hpp"
#include "shipSelection.hpp"
#include "uiHelpers.hpp"
#include "audio.hpp"
#include <cmath>
#include <cstdio>

extern Game game;

static int snapshotQuickPlay = 0;
static int snapshotAsteroids = 0;
static int snapshotShipHigh = 0;
static bool newGlobalAsteroidsRecord = false;
static bool newShipBest = false;
static bool newSurvivalRecord = false;
static double summaryAnimStart = 0.0;
static bool summaryAnimStarted = false;

static void FormatPlayTime(float seconds, char* buffer, size_t bufferSize) {
    int totalSec = (int)seconds;
    int mins = totalSec / 60;
    int secs = totalSec % 60;
    if (mins > 0) {
        snprintf(buffer, bufferSize, "%dm %ds", mins, secs);
    } else {
        snprintf(buffer, bufferSize, "%ds", secs);
    }
}

static float EaseOutCubic(float t) {
    float inv = 1.0f - t;
    return 1.0f - inv * inv * inv;
}

void InitRunSummarySnapshot() {
    snapshotQuickPlay = highScoreQuickPlay;
    snapshotAsteroids = highScoreAsteroids;
    int shipIdx = player.starshipType;
    snapshotShipHigh = (shipIdx >= 0 && shipIdx < MAX_SHIPS) ? shipHighScores[shipIdx] : 0;
    newGlobalAsteroidsRecord = false;
    newShipBest = false;
    newSurvivalRecord = false;
    summaryAnimStarted = false;
}

void CaptureRunSummaryRecords() {
    summaryAnimStart = GetTime();
    summaryAnimStarted = true;

    if (asteroidsMode) {
        newGlobalAsteroidsRecord = currentRun.score > snapshotAsteroids;
        int shipIdx = player.starshipType;
        if (shipIdx >= 0 && shipIdx < MAX_SHIPS) {
            newShipBest = currentRun.score > snapshotShipHigh;
        }
    } else if (enemiesOn && !asteroidsMode) {
        newSurvivalRecord = (int)currentRun.time > snapshotQuickPlay;
    }
}

void UpdateRunSummary() {
    if (!game.gameOver) return;

    const bool riftVictoryChoice = game.victory && !game.endlessMode && enemiesOn && !asteroidsMode;

    if (riftVictoryChoice) {
        if (IsKeyPressed(KEY_C) || IsKeyPressed(KEY_ENTER)) {
            game.ContinueEndless();
            return;
        }
    }

    if (IsKeyPressed(KEY_M)) {
        game.returnToMainMenu = true;
    }
    if (IsKeyPressed(KEY_R)) {
        game.gameOver = false;
        game.InitGame();
    }
}

static void DrawUpgradeRecap(int panelX, int panelY, int panelWidth, int startY, int maxHeight) {
    DrawText("Build Recap", panelX + 24, startY, 16, YELLOW);
    int y = startY + 24;

    if (runMods.acquiredCount <= 0) {
        DrawText("No upgrades acquired", panelX + 24, y, 14, DARKGRAY);
        return;
    }

    const int iconSize = 28;
    const int iconGap = 8;
    const int iconsPerRow = 8;
    float iconStartX = (float)(panelX + 24);
    float iconY = (float)y;

    for (int i = 0; i < runMods.acquiredCount; i++) {
        int col = i % iconsPerRow;
        int row = i / iconsPerRow;
        float cx = iconStartX + col * (iconSize + iconGap) + iconSize / 2.0f;
        float cy = iconY + row * (iconSize + iconGap) + iconSize / 2.0f;

        Color rarityColor = GetUpgradeRarityColor(runMods.acquired[i].rarity);
        Rectangle badge = { cx - iconSize / 2.0f, cy - iconSize / 2.0f, (float)iconSize, (float)iconSize };
        DrawRectangleRounded(badge, 0.2f, 4, (Color){ 20, 22, 32, 200 });
        DrawRectangleRoundedLines(badge, 0.2f, 4, rarityColor);
        DrawUpgradeIcon(runMods.acquired[i].upgradeId, { cx, cy }, iconSize * 0.7f, rarityColor);
    }

    int iconRows = (runMods.acquiredCount + iconsPerRow - 1) / iconsPerRow;
    int textY = y + iconRows * (iconSize + iconGap) + 6;
    int maxLines = (maxHeight - (textY - startY)) / 16;
    if (maxLines < 1) maxLines = 1;

    char line[128];
    for (int i = 0; i < runMods.acquiredCount && i < maxLines; i++) {
        const char* title = GetUpgradeTitle(runMods.acquired[i].upgradeId);
        const char* rarityName = GetUpgradeRarityName(runMods.acquired[i].rarity);
        Color rarityColor = GetUpgradeRarityColor(runMods.acquired[i].rarity);
        snprintf(line, sizeof(line), "- %s (%s)", title, rarityName);
        DrawText(line, panelX + 24, textY + i * 16, 13, rarityColor);
    }

    if (runMods.acquiredCount > maxLines) {
        snprintf(line, sizeof(line), "... +%d more", runMods.acquiredCount - maxLines);
        DrawText(line, panelX + 24, textY + maxLines * 16, 13, DARKGRAY);
    }
}

static int DrawCreditBreakdown(int x, int y, bool isRift, int kills, int scrap, float time, int wave, bool victory) {
    int killPart = kills * 3;
    int scrapPart = scrap / 4;
    int timePart = (int)(time / 2.0f);
    int wavePart = wave * 8;
    int victoryPart = victory ? (500 + wave * 25) : 0;
    int rawTotal = killPart + scrapPart + timePart + wavePart + victoryPart;
    bool belowMin = time >= 5.0f && rawTotal < 5;

    char line[160];
    int lineY = y;
    const int lineHeight = 18;

    if (isRift) {
        snprintf(line, sizeof(line), "Kills:  %d x 3  = %d", kills, killPart);
        DrawText(line, x, lineY, 14, LIGHTGRAY);
        lineY += lineHeight;

        snprintf(line, sizeof(line), "Scrap:  %d / 4  = %d", scrap, scrapPart);
        DrawText(line, x, lineY, 14, LIGHTGRAY);
        lineY += lineHeight;

        char timeStr[32];
        FormatPlayTime(time, timeStr, sizeof(timeStr));
        snprintf(line, sizeof(line), "Time:   %s / 2  = %d", timeStr, timePart);
        DrawText(line, x, lineY, 14, LIGHTGRAY);
        lineY += lineHeight;

        snprintf(line, sizeof(line), "Wave:   %d x 8  = %d", wave, wavePart);
        DrawText(line, x, lineY, 14, LIGHTGRAY);
        lineY += lineHeight;

        if (victory) {
            snprintf(line, sizeof(line), "Victory bonus:  %d", victoryPart);
            DrawText(line, x, lineY, 14, (Color){ 120, 255, 160, 255 });
            lineY += lineHeight;
        }
    } else {
        snprintf(line, sizeof(line), "Scrap:  %d / 4  = %d", scrap, scrapPart);
        DrawText(line, x, lineY, 14, LIGHTGRAY);
        lineY += lineHeight;

        char timeStr[32];
        FormatPlayTime(time, timeStr, sizeof(timeStr));
        snprintf(line, sizeof(line), "Time:   %s / 2  = %d", timeStr, timePart);
        DrawText(line, x, lineY, 14, LIGHTGRAY);
        lineY += lineHeight;
    }

    DrawLine((float)x, (float)lineY + 2, (float)x + 220, (float)lineY + 2, (Color){ 70, 70, 90, 255 });
    lineY += 8;

    if (time < 5.0f) {
        DrawText("Run too short for credits", x, lineY, 14, DARKGRAY);
    } else {
        snprintf(line, sizeof(line), "Total:  +%d Credits", lastRunCreditsEarned);
        DrawText(line, x, lineY, 16, (Color){ 120, 220, 255, 255 });
        if (belowMin) {
            DrawText("(minimum 5 credits)", x + 180, lineY + 2, 12, DARKGRAY);
        }
    }

    return lineY + lineHeight;
}

static void DrawRecordCallouts(int panelX, int panelY, int panelWidth, int y) {
    bool hasCallout = newSurvivalRecord || newGlobalAsteroidsRecord || newShipBest;
    if (!hasCallout) return;

    float pulse = 0.75f + 0.25f * sinf((float)GetTime() * 4.0f);
    Color calloutColor = {
        (unsigned char)(255 * pulse),
        (unsigned char)(210 * pulse),
        (unsigned char)(80 * pulse),
        255
    };

    int cx = panelX + panelWidth / 2;
    char msg[80];

    if (newSurvivalRecord) {
        snprintf(msg, sizeof(msg), "NEW SURVIVAL RECORD!");
        DrawText(msg, cx - MeasureText(msg, 16) / 2, y, 16, calloutColor);
        y += 22;
    }
    if (newGlobalAsteroidsRecord) {
        snprintf(msg, sizeof(msg), "NEW ASTEROIDS HIGH SCORE!");
        DrawText(msg, cx - MeasureText(msg, 16) / 2, y, 16, calloutColor);
        y += 22;
    }
    if (newShipBest) {
        const ShipStats* ship = shipSelection.GetShip(player.starshipType);
        const char* shipName = ship ? ship->name.c_str() : "this ship";
        snprintf(msg, sizeof(msg), "NEW PERSONAL BEST ON %s!", shipName);
        int fontSize = MeasureText(msg, 16) > panelWidth - 48 ? 14 : 16;
        DrawText(msg, cx - MeasureText(msg, fontSize) / 2, y, fontSize, calloutColor);
    }
}

void DrawRunSummary() {
    if (!game.gameOver) return;

    float progress = 1.0f;
    if (summaryAnimStarted) {
        float animT = (float)(GetTime() - summaryAnimStart);
        progress = fminf(animT / 0.45f, 1.0f);
        progress = EaseOutCubic(progress);
    }

    int overlayAlpha = (int)(175 * progress);
    DrawRectangle(0, 0, gameWidth, gameHeight, (Color){ 0, 0, 0, (unsigned char)overlayAlpha });

    const bool isRift = enemiesOn && !asteroidsMode;
    const bool riftVictoryChoice = game.victory && !game.endlessMode && isRift;
    const int panelWidth = 900;
    const int panelHeight = isRift ? (riftVictoryChoice ? 530 : 500) : 400;
    const float panelX = (gameWidth - panelWidth) / 2.0f;
    float slideOffset = (1.0f - progress) * 50.0f;
    const float panelY = (gameHeight - panelHeight) / 2.0f + slideOffset;
    const Rectangle panel = { panelX, panelY, (float)panelWidth, (float)panelHeight };

    DrawRectangleRounded(panel, 0.05f, 8, (Color){ 25, 25, 35, (unsigned char)(240 * progress) });
    DrawRectangleRoundedLines(panel, 0.05f, 8, WHITE);

    int kills = isRift ? currentRun.score : 0;
    int wave = isRift ? GetEnemyWave() : 0;

    const char* title;
    if (riftVictoryChoice || (game.victory && !game.endlessMode)) {
        title = "VICTORY";
    } else if (game.victory && game.endlessMode) {
        title = "ENDLESS COMPLETE";
    } else {
        title = "RUN COMPLETE";
    }
    const int titleSize = 28;
    Color titleColor = game.victory ? (Color){ 120, 255, 160, 255 } : (Color){ 210, 170, 110, 255 };
    DrawText(title, panelX + panelWidth / 2 - MeasureText(title, titleSize) / 2,
             (int)panelY + 18, titleSize, titleColor);

    if (riftVictoryChoice) {
        const char* victoryMsg = game.victoryMessage[0] != '\0'
            ? game.victoryMessage
            : "Victory!";
        DrawText(victoryMsg, panelX + panelWidth / 2 - MeasureText(victoryMsg, 14) / 2,
                 (int)panelY + 52, 14, (Color){ 180, 230, 200, 255 });
        const char* endlessPrompt = "Waves continue past the Death Star — how far can you go?";
        DrawText(endlessPrompt, panelX + panelWidth / 2 - MeasureText(endlessPrompt, 13) / 2,
                 (int)panelY + 72, 13, (Color){ 255, 210, 80, 255 });
    } else if (game.victory) {
        const char* victoryMsg = game.victoryMessage[0] != '\0'
            ? game.victoryMessage
            : "Victory!";
        if (game.endlessMode) {
            char endlessMsg[96];
            snprintf(endlessMsg, sizeof(endlessMsg), "Endless run ended at Wave %d", wave);
            DrawText(endlessMsg, panelX + panelWidth / 2 - MeasureText(endlessMsg, 14) / 2,
                     (int)panelY + 52, 14, (Color){ 180, 230, 200, 255 });
        } else {
            DrawText(victoryMsg, panelX + panelWidth / 2 - MeasureText(victoryMsg, 14) / 2,
                     (int)panelY + 52, 14, (Color){ 180, 230, 200, 255 });
        }
    } else {
        const ShipStats* ship = shipSelection.GetShip(player.starshipType);
        const char* shipName = ship ? ship->name.c_str() : "Unknown";
        char subtitle[64];
        snprintf(subtitle, sizeof(subtitle), "%s  |  %s", shipName, isRift ? "Rift Fight" : "Asteroids");
        DrawText(subtitle, panelX + panelWidth / 2 - MeasureText(subtitle, 14) / 2,
                 (int)panelY + 52, 14, LIGHTGRAY);
    }

    int y = (int)panelY + (riftVictoryChoice ? 92 : 78);
    DrawLine(panelX + 20, (float)y, panelX + panelWidth - 20, (float)y, (Color){ 70, 70, 90, 255 });
    y += 14;

    const int cardHeight = 58;
    const int cardGap = 12;
    if (isRift) {
        const int cardWidth = (panelWidth - 48 - 3 * cardGap) / 4;
        int cardX = (int)panelX + 24;
        DrawStatCard({ (float)cardX, (float)y, (float)cardWidth, (float)cardHeight }, "TIME", (int)currentRun.time, (Color){ 35, 40, 55, 255 });
        cardX += cardWidth + cardGap;
        DrawStatCard({ (float)cardX, (float)y, (float)cardWidth, (float)cardHeight }, "WAVE", wave, (Color){ 40, 35, 55, 255 });
        cardX += cardWidth + cardGap;
        DrawStatCard({ (float)cardX, (float)y, (float)cardWidth, (float)cardHeight }, "KILLS", kills, (Color){ 55, 35, 35, 255 });
        cardX += cardWidth + cardGap;
        DrawStatCard({ (float)cardX, (float)y, (float)cardWidth, (float)cardHeight }, "SCRAP", currentRun.scrap, (Color){ 35, 50, 40, 255 });
    } else {
        const int cardWidth = (panelWidth - 48 - 2 * cardGap) / 3;
        int cardX = (int)panelX + 24;
        DrawStatCard({ (float)cardX, (float)y, (float)cardWidth, (float)cardHeight }, "SCORE", currentRun.score, (Color){ 55, 35, 35, 255 });
        cardX += cardWidth + cardGap;
        DrawStatCard({ (float)cardX, (float)y, (float)cardWidth, (float)cardHeight }, "SCRAP", currentRun.scrap, (Color){ 35, 50, 40, 255 });
        cardX += cardWidth + cardGap;
        DrawStatCard({ (float)cardX, (float)y, (float)cardWidth, (float)cardHeight }, "TIME", (int)currentRun.time, (Color){ 35, 40, 55, 255 });
    }

    y += cardHeight + 18;
    DrawRecordCallouts((int)panelX, (int)panelY, panelWidth, y);
    int calloutCount = (newSurvivalRecord ? 1 : 0) + (newGlobalAsteroidsRecord ? 1 : 0) + (newShipBest ? 1 : 0);
    if (calloutCount > 0) {
        y += calloutCount * 22 + 4;
    }

    const int leftCol = (int)panelX + 24;
    const int rightCol = (int)panelX + panelWidth / 2 + 8;

    DrawText("Credit Breakdown", leftCol, y, 16, YELLOW);
    DrawCreditBreakdown(leftCol, y + 24, isRift, kills, currentRun.scrap, currentRun.time, wave, game.victory);

    if (isRift) {
        DrawUpgradeRecap(rightCol, (int)panelY, panelWidth, y, (int)(panelY + panelHeight) - y - 50);
    }

    const char* hint = riftVictoryChoice
        ? "C: Continue endless   |   R: Restart   |   M: Return to menu"
        : "R: Restart   |   M: Return to menu";
    int hintY = (int)(panelY + panelHeight) - 32;
    DrawCreditGoalNudge(panelX + panelWidth / 2, hintY - 22, 14);
    Color hintColor = riftVictoryChoice ? (Color){ 255, 210, 80, 255 } : DARKGRAY;
    DrawText(hint, panelX + panelWidth / 2 - MeasureText(hint, 14) / 2, hintY, 14, hintColor);
}
