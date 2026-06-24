#include "hangar.hpp"
#include "globals.hpp"
#include "metaProgression.hpp"
#include <cstdio>

void Hangar::Update() {
    for (int i = 0; i < META_ITEM_COUNT; i++) {
        if (IsKeyPressed(KEY_ONE + i)) {
            if (IsMetaOwned(i) && IsMetaCosmetic(i)) {
                TryToggleMetaItem(i);
            } else {
                TryPurchaseMetaItem(i);
            }
        }
    }
}

static void DrawHangarCard(int itemX, int cardY, int cardWidth, int cardHeight, int itemId) {
    bool owned = IsMetaOwned(itemId);
    Color bg = owned ? (Color){ 35, 55, 40, 240 } : (Color){ 35, 35, 48, 240 };
    Color border = owned ? LIME : (Color){ 80, 80, 100, 255 };

    DrawRectangle(itemX, cardY, cardWidth, cardHeight, bg);
    DrawRectangleLines(itemX, cardY, cardWidth, cardHeight, border);

    char line[128];
    snprintf(line, sizeof(line), "[%d] %s", itemId + 1, GetMetaItemName(itemId));
    DrawText(line, itemX + 12, cardY + 12, 16, owned ? LIME : WHITE);

    DrawText(GetMetaItemDescription(itemId), itemX + 12, cardY + 38, 13, GRAY);

    if (owned) {
        if (IsMetaCosmetic(itemId)) {
            bool enabled = IsMetaEnabled(itemId);
            const char* status = enabled ? "ENABLED" : "DISABLED";
            Color statusColor = enabled ? LIME : (Color){ 140, 140, 150, 255 };
            DrawText(status, itemX + 12, cardY + cardHeight - 50, 15, statusColor);
            DrawText("Press to toggle", itemX + 12, cardY + cardHeight - 32, 13, DARKGRAY);
        } else {
            DrawText("INSTALLED", itemX + 12, cardY + cardHeight - 32, 15, LIME);
        }
    } else {
        snprintf(line, sizeof(line), "Cost: %d credits", GetMetaItemCost(itemId));
        Color costColor = playerCredits >= GetMetaItemCost(itemId)
            ? (Color){ 120, 220, 255, 255 }
            : (Color){ 255, 120, 90, 255 };
        DrawText(line, itemX + 12, cardY + cardHeight - 32, 15, costColor);
    }
}

void Hangar::Draw() const {
    const char* title = "HANGAR";
    const int titleSize = 36;
    DrawText(title, gameWidth / 2 - MeasureText(title, titleSize) / 2, 48, titleSize, (Color){ 210, 170, 110, 255 });

    const char* subtitle = "UPGRADES";
    DrawText(subtitle, gameWidth / 2 - MeasureText(subtitle, 18) / 2, 92, 18, LIGHTGRAY);

    const int panelWidth = 900;
    const int panelHeight = 500;
    const float panelX = (gameWidth - panelWidth) / 2.0f;
    const float panelY = 120.0f;
    const Rectangle panel = { panelX, panelY, (float)panelWidth, (float)panelHeight };

    DrawRectangleRounded(panel, 0.05f, 8, (Color){ 25, 25, 35, 240 });
    DrawRectangleRoundedLines(panel, 0.05f, 8, WHITE);

    DrawCreditsBadge((int)panelX + 24, (int)panelY + 18, 20);

    char hint[96];
    snprintf(hint, sizeof(hint), "Press 1-%d: purchase or toggle   |   BACKSPACE: Menu", META_ITEM_COUNT);
    DrawText(hint, (int)panelX + panelWidth / 2 - MeasureText(hint, 14) / 2, (int)panelY + 22, 14, DARKGRAY);

    const int itemsPerRow = 4;
    const int cardWidth = 200;
    const int cardHeight = 190;
    const int cardSpacing = 16;
    const int rowSpacing = 16;
    const int numRows = (META_ITEM_COUNT + itemsPerRow - 1) / itemsPerRow;
    const int gridHeight = numRows * cardHeight + (numRows - 1) * rowSpacing;
    const int cardYStart = (int)panelY + 60 + (panelHeight - 60 - gridHeight) / 2;

    for (int i = 0; i < META_ITEM_COUNT; i++) {
        int row = i / itemsPerRow;
        int col = i % itemsPerRow;
        int itemsInRow = itemsPerRow;
        if (row == numRows - 1) {
            int remainder = META_ITEM_COUNT % itemsPerRow;
            if (remainder != 0) itemsInRow = remainder;
        }

        int rowWidth = itemsInRow * cardWidth + (itemsInRow - 1) * cardSpacing;
        int rowStartX = (int)panelX + panelWidth / 2 - rowWidth / 2;
        int itemX = rowStartX + col * (cardWidth + cardSpacing);
        int cardY = cardYStart + row * (cardHeight + rowSpacing);

        DrawHangarCard(itemX, cardY, cardWidth, cardHeight, i);
    }
}
