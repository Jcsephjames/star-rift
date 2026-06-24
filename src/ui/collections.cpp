#include "collections.hpp"
#include "globals.hpp"
#include "textures.hpp"
#include "unlocks.hpp"
#include "uiHelpers.hpp"
#include "metaProgression.hpp"
#include <cstdio>
#include <cmath>

Collections::Collections() : selectedShipIndex(0) {}

void Collections::Update() {
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        selectedShipIndex--;
        if (selectedShipIndex < 0) selectedShipIndex = MAX_SHIPS - 1;
    }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        selectedShipIndex++;
        if (selectedShipIndex >= MAX_SHIPS) selectedShipIndex = 0;
    }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        TryPurchaseShip(selectedShipIndex);
    }
}

void Collections::FormatPlayTime(float seconds, char* buffer, size_t bufferSize) {
    int totalSec = (int)seconds;
    int hours = totalSec / 3600;
    int mins = (totalSec % 3600) / 60;
    int secs = totalSec % 60;
    if (hours > 0) {
        snprintf(buffer, bufferSize, "%dh %dm %ds", hours, mins, secs);
    } else if (mins > 0) {
        snprintf(buffer, bufferSize, "%dm %ds", mins, secs);
    } else {
        snprintf(buffer, bufferSize, "%ds", secs);
    }
}

void Collections::DrawShipThumbnail(int index, int x, int y, int size) const {
    const ShipStats* ship = shipSelection.GetShip(index);
    if (!ship) return;

    bool unlocked = IsShipUnlocked(index);
    bool selected = (index == selectedShipIndex);

    Rectangle thumbRect = { (float)x, (float)y, (float)size, (float)size };
    Color bgColor = selected ? (Color){ 40, 60, 90, 255 } : (Color){ 30, 30, 42, 255 };
    DrawRectangleRounded(thumbRect, 0.15f, 4, bgColor);
    DrawRectangleRoundedLines(thumbRect, 0.15f, 4, selected ? YELLOW : (Color){ 70, 70, 90, 255 });

    float maxDim = (float)fmax(ship->image.width, ship->image.height);
    float scale = (float)size * 0.8f / maxDim;
    int imgW = (int)(ship->image.width * scale);
    int imgH = (int)(ship->image.height * scale);
    int imgX = x + (size - imgW) / 2;
    int imgY = y + (size - imgH) / 2;

    Color tint = unlocked ? WHITE : (Color){ 100, 100, 100, 200 };
    DrawTextureEx(ship->image, (Vector2){ (float)imgX, (float)imgY }, 0.0f, scale, tint);

    if (!unlocked) {
        DrawRectangle((float)x, (float)y, (float)size, (float)size, (Color){ 0, 0, 0, 100 });
        const char* lockLabel = "LOCKED";
        int lockW = MeasureText(lockLabel, 10);
        DrawText(lockLabel, x + size / 2 - lockW / 2, y + size / 2 - 5, 10, RED);
    }
}

void Collections::DrawShipPreview(int centerX, int centerY) const {
    const ShipStats* ship = shipSelection.GetShip(selectedShipIndex);
    if (!ship) return;

    bool unlocked = IsShipUnlocked(selectedShipIndex);
    const int maxPreview = 150;
    float maxDim = (float)fmax(ship->image.width, ship->image.height);
    float scale = (float)maxPreview / maxDim;
    if (scale > 1.0f) scale = 1.0f;

    int imgW = (int)(ship->image.width * scale);
    int imgH = (int)(ship->image.height * scale);
    int imgX = centerX - imgW / 2;
    int imgY = centerY - imgH / 2;

    Color tint = unlocked ? WHITE : (Color){ 120, 120, 120, 220 };
    DrawTextureEx(ship->image, (Vector2){ (float)imgX, (float)imgY }, 0.0f, scale, tint);

    if (!unlocked) {
        DrawRectangle((float)imgX, (float)imgY, (float)imgW, (float)imgH, (Color){ 0, 0, 0, 110 });
        const char* lockLabel = "LOCKED";
        DrawText(lockLabel, centerX - MeasureText(lockLabel, 24) / 2, centerY - 12, 24, RED);
    }
}

void Collections::Draw() const {
    const int titleY = 40;
    const int titleBottom = titleY + collections.height;

    DrawTexture(collections, gameWidth / 2 - collections.width / 2, titleY, WHITE);
    DrawCreditsBadge(gameWidth - 180, 12, 18);

    const int panelWidth = 900;
    const int panelHeight = 430;
    const float panelX = (gameWidth - panelWidth) / 2.0f;
    const float panelY = (float)(titleBottom + 10);
    const Rectangle panel = { panelX, panelY, (float)panelWidth, (float)panelHeight };

    DrawRectangleRounded(panel, 0.05f, 8, (Color){ 25, 25, 35, 240 });
    DrawRectangleRoundedLines(panel, 0.05f, 8, WHITE);

    const ShipStats* ship = shipSelection.GetShip(selectedShipIndex);
    if (!ship) {
        DrawText("No ship data loaded.", (int)panelX + 20, (int)panelY + 20, 20, RED);
        return;
    }

    bool unlocked = IsShipUnlocked(selectedShipIndex);
    int contentY = (int)panelY + 14;

    char headerLine[128];
    snprintf(headerLine, sizeof(headerLine), "Ships Unlocked: %d / %d", CountUnlockedShips(), MAX_SHIPS);
    DrawText(headerLine, (int)panelX + panelWidth / 2 - MeasureText(headerLine, 18) / 2, contentY, 18, GOLD);
    contentY += 24;

    snprintf(headerLine, sizeof(headerLine),
             "Rift Fight: %d sec   |   Asteroids: %d   |   Ships Destroyed: %d",
             highScoreQuickPlay, highScoreAsteroids, totalShipsDestroyed);
    DrawText(headerLine, (int)panelX + panelWidth / 2 - MeasureText(headerLine, 14) / 2, contentY, 14, LIGHTGRAY);
    contentY += 26;

    int cardCount = 4;
    int cardWidth = 130;
    int cardHeight = 52;
    int cardSpacing = 16;
    int totalCardWidth = cardCount * cardWidth + (cardCount - 1) * cardSpacing;
    int cardsStartX = (int)panelX + panelWidth / 2 - totalCardWidth / 2;

    DrawStatCard({ (float)cardsStartX, (float)contentY, (float)cardWidth, (float)cardHeight },
                 "Shot Power", ship->shotPower, (Color){ 35, 35, 48, 255 });
    DrawStatCard({ (float)(cardsStartX + (cardWidth + cardSpacing)), (float)contentY, (float)cardWidth, (float)cardHeight },
                 "Lazer Rate", ship->shotCooldown, (Color){ 35, 35, 48, 255 });
    DrawStatCard({ (float)(cardsStartX + 2 * (cardWidth + cardSpacing)), (float)contentY, (float)cardWidth, (float)cardHeight },
                 "Speed", ship->speed, (Color){ 35, 35, 48, 255 });
    DrawStatCard({ (float)(cardsStartX + 3 * (cardWidth + cardSpacing)), (float)contentY, (float)cardWidth, (float)cardHeight },
                 "Boost", ship->boostMultiplier, (Color){ 35, 35, 48, 255 });
    contentY += cardHeight + 12;

    const int panelBottom = (int)(panelY + panelHeight);
    const int hintFontSize = 13;
    const int thumbSize = 56;
    const int thumbSpacing = 12;

    const char* hint = "A / D: Browse   |   ENTER: Purchase   |   BACKSPACE: Menu";
    const int hintY = panelBottom - hintFontSize - 8;
    const int galleryY = hintY - thumbSize - 10;
    const int infoBottom = galleryY - 12;

    char playTimeStr[32];
    FormatPlayTime(shipPlayTime[selectedShipIndex], playTimeStr, sizeof(playTimeStr));
    snprintf(headerLine, sizeof(headerLine), "Best: %d   |   Flown: %s   |   Destroyed: %d",
             shipHighScores[selectedShipIndex], playTimeStr, shipKills[selectedShipIndex]);

    const int statsFontSize = 14;
    const int statusFontSize = 16;
    const int nameFontSize = 22;
    int infoY = infoBottom - statsFontSize;
    DrawText(headerLine, (int)panelX + panelWidth / 2 - MeasureText(headerLine, statsFontSize) / 2, infoY, statsFontSize, WHITE);
    infoY -= statusFontSize + 6;

    if (unlocked) {
        DrawText("UNLOCKED", (int)panelX + panelWidth / 2 - MeasureText("UNLOCKED", statusFontSize) / 2, infoY, statusFontSize, LIME);
    } else {
        int cost = GetShipCreditCost(selectedShipIndex);
        snprintf(headerLine, sizeof(headerLine), "Cost: %d credits", cost);
        Color costColor = playerCredits >= cost
            ? (Color){ 120, 220, 255, 255 }
            : (Color){ 255, 120, 90, 255 };
        DrawText(headerLine, (int)panelX + panelWidth / 2 - MeasureText(headerLine, statusFontSize) / 2, infoY, statusFontSize, costColor);
        infoY -= statusFontSize + 4;
        const char* buyHint = "ENTER: Purchase ship";
        DrawText(buyHint, (int)panelX + panelWidth / 2 - MeasureText(buyHint, 13) / 2, infoY, 13, DARKGRAY);
        infoY -= 18;
    }
    infoY -= nameFontSize + 6;
    DrawText(ship->name.c_str(),
             (int)panelX + panelWidth / 2 - MeasureText(ship->name.c_str(), nameFontSize) / 2,
             infoY, nameFontSize, unlocked ? GREEN : GRAY);

    const int previewAreaTop = contentY;
    const int previewAreaBottom = infoY - 8;
    int previewCenterX = (int)panelX + panelWidth / 2;
    int previewCenterY = previewAreaTop + (previewAreaBottom - previewAreaTop) / 2;
    DrawShipPreview(previewCenterX, previewCenterY);

    DrawTriangle(
        (Vector2){ panelX + 24.0f, (float)previewCenterY },
        (Vector2){ panelX + 50.0f, (float)previewCenterY + 26.0f },
        (Vector2){ panelX + 50.0f, (float)previewCenterY - 26.0f },
        WHITE
    );
    DrawTriangle(
        (Vector2){ panelX + panelWidth - 24.0f, (float)previewCenterY },
        (Vector2){ panelX + panelWidth - 50.0f, (float)previewCenterY - 26.0f },
        (Vector2){ panelX + panelWidth - 50.0f, (float)previewCenterY + 26.0f },
        WHITE
    );

    int totalGalleryWidth = MAX_SHIPS * thumbSize + (MAX_SHIPS - 1) * thumbSpacing;
    int galleryStartX = (int)panelX + panelWidth / 2 - totalGalleryWidth / 2;
    for (int i = 0; i < MAX_SHIPS; i++) {
        int x = galleryStartX + i * (thumbSize + thumbSpacing);
        DrawShipThumbnail(i, x, galleryY, thumbSize);
    }

    DrawText(hint, (int)panelX + panelWidth / 2 - MeasureText(hint, hintFontSize) / 2, hintY, hintFontSize, DARKGRAY);
}
