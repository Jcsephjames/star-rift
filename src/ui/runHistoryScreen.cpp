#include "runHistoryScreen.hpp"
#include "globals.hpp"
#include "shipSelection.hpp"
#include "cardDraft.hpp"
#include "metaProgression.hpp"
#include <cstdio>

RunHistoryScreen::RunHistoryScreen() : activeTab(RunHistoryMode::RIFT), selectedIndex(0) {}

void RunHistoryScreen::FormatPlayTime(float seconds, char* buffer, size_t bufferSize) {
    int totalSec = (int)seconds;
    int mins = totalSec / 60;
    int secs = totalSec % 60;
    if (mins > 0) {
        snprintf(buffer, bufferSize, "%dm %ds", mins, secs);
    } else {
        snprintf(buffer, bufferSize, "%ds", secs);
    }
}

void RunHistoryScreen::Update() {
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
        activeTab = RunHistoryMode::RIFT;
        selectedIndex = 0;
    }
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
        activeTab = RunHistoryMode::ASTEROIDS;
        selectedIndex = 0;
    }

    int count = GetRunHistoryCount(activeTab);
    if (count <= 0) return;

    if (selectedIndex >= count) {
        selectedIndex = count - 1;
    }

    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        selectedIndex++;
        if (selectedIndex >= count) selectedIndex = 0;
    }
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        selectedIndex--;
        if (selectedIndex < 0) selectedIndex = count - 1;
    }
}

void RunHistoryScreen::DrawTabs(int panelX, int panelY, int panelWidth) const {
    const int tabY = panelY + 12;
    const int tabWidth = 180;
    const int tabHeight = 32;
    const int tabGap = 12;
    const int tabsTotalWidth = tabWidth * 2 + tabGap;
    const int tabsStartX = panelX + panelWidth / 2 - tabsTotalWidth / 2;

    auto drawTab = [&](int x, const char* label, RunHistoryMode mode) {
        bool active = activeTab == mode;
        Rectangle tab = { (float)x, (float)tabY, (float)tabWidth, (float)tabHeight };
        Color bg = active ? (Color){ 40, 60, 90, 255 } : (Color){ 30, 30, 42, 255 };
        Color border = active ? YELLOW : (Color){ 70, 70, 90, 255 };
        DrawRectangleRounded(tab, 0.15f, 4, bg);
        DrawRectangleRoundedLines(tab, 0.15f, 4, border);
        DrawText(label, x + tabWidth / 2 - MeasureText(label, 16) / 2, tabY + 8, 16, active ? WHITE : LIGHTGRAY);
    };

    drawTab(tabsStartX, "RIFT FIGHT", RunHistoryMode::RIFT);
    drawTab(tabsStartX + tabWidth + tabGap, "ASTEROIDS", RunHistoryMode::ASTEROIDS);
}

void RunHistoryScreen::DrawRunList(int panelX, int panelY, int panelWidth, int listHeight) const {
    const int listX = panelX + 16;
    const int listY = panelY + 56;
    const int rowHeight = 28;
    int count = GetRunHistoryCount(activeTab);

    DrawText("Recent Runs", listX, listY, 16, YELLOW);

    if (count <= 0) {
        DrawText("No runs recorded yet.", listX, listY + 30, 14, GRAY);
        if (activeTab == RunHistoryMode::RIFT) {
            DrawText("Complete a Rift Fight to log history.", listX, listY + 50, 14, DARKGRAY);
        } else {
            DrawText("Complete an Asteroids run to log history.", listX, listY + 50, 14, DARKGRAY);
        }
        return;
    }

    int rowsStartY = listY + 28;
    int visibleRows = (listHeight - 84) / rowHeight;
    if (visibleRows > count) visibleRows = count;
    if (visibleRows < 1) visibleRows = 1;

    int scrollOffset = 0;
    if (selectedIndex >= visibleRows) {
        scrollOffset = selectedIndex - visibleRows + 1;
    }

    for (int i = 0; i < visibleRows; i++) {
        int entryIndex = scrollOffset + i;
        if (entryIndex >= count) break;

        bool selected = (entryIndex == selectedIndex);
        int rowY = rowsStartY + i * rowHeight;

        if (selected) {
            DrawRectangle(listX - 6, rowY - 2, panelWidth / 2 - 20, rowHeight, (Color){ 40, 60, 90, 180 });
        }

        char line[96];
        if (activeTab == RunHistoryMode::RIFT) {
            const RiftRunEntry* entry = GetRiftRunHistoryEntry(entryIndex);
            if (!entry) continue;

            const ShipStats* ship = shipSelection.GetShip(entry->shipType);
            const char* shipName = ship ? ship->name.c_str() : "Unknown";
            snprintf(line, sizeof(line), "#%d  %s  W%d  %dK%s", entryIndex + 1, shipName, entry->wave, entry->kills,
                     entry->victory ? "  VICTORY" : "");
        } else {
            const AsteroidsRunEntry* entry = GetAsteroidsRunHistoryEntry(entryIndex);
            if (!entry) continue;

            const ShipStats* ship = shipSelection.GetShip(entry->shipType);
            const char* shipName = ship ? ship->name.c_str() : "Unknown";
            snprintf(line, sizeof(line), "#%d  %s  %d pts%s", entryIndex + 1, shipName, entry->score,
                     entry->victory ? "  SECTOR SECURED" : "");
        }

        DrawText(line, listX, rowY + 4, 14, selected ? WHITE : LIGHTGRAY);
    }
}

void RunHistoryScreen::DrawRiftDetails(int panelX, int panelY, int panelWidth, int panelHeight) const {
    const int detailX = panelX + panelWidth / 2 + 8;
    const int detailY = panelY + 56;
    const int detailWidth = panelWidth / 2 - 24;
    int count = GetRunHistoryCount(RunHistoryMode::RIFT);

    DrawText("Run Details", detailX, detailY, 16, YELLOW);

    if (count <= 0 || selectedIndex < 0 || selectedIndex >= count) {
        return;
    }

    const RiftRunEntry* entry = GetRiftRunHistoryEntry(selectedIndex);
    if (!entry) return;

    const ShipStats* ship = shipSelection.GetShip(entry->shipType);
    const char* shipName = ship ? ship->name.c_str() : "Unknown";

    char timeStr[32];
    FormatPlayTime(entry->timeSeconds, timeStr, sizeof(timeStr));

    int y = detailY + 28;
    const int lineHeight = 22;
    char line[128];

    snprintf(line, sizeof(line), "Run #%d  (%s)", selectedIndex + 1, selectedIndex == 0 ? "Latest" : "Older");
    DrawText(line, detailX, y, 14, GOLD);
    y += lineHeight + 4;

    snprintf(line, sizeof(line), "Ship: %s", shipName);
    DrawText(line, detailX, y, 14, WHITE);
    y += lineHeight;

    snprintf(line, sizeof(line), "Wave: %d   |   Kills: %d   |   Scrap: %d", entry->wave, entry->kills, entry->scrap);
    DrawText(line, detailX, y, 14, WHITE);
    y += lineHeight;

    if (entry->victory) {
        DrawText("Result: VICTORY — Death Star destroyed", detailX, y, 14, (Color){ 120, 255, 160, 255 });
        y += lineHeight;
    }

    snprintf(line, sizeof(line), "Time: %s   |   Credits: +%d", timeStr, entry->creditsEarned);
    DrawText(line, detailX, y, 14, WHITE);
    y += lineHeight + 8;

    DrawLine(detailX, y, detailX + detailWidth, y, (Color){ 70, 70, 90, 255 });
    y += 10;

    DrawText("Upgrades:", detailX, y, 14, LIGHTGRAY);
    y += 22;

    if (entry->upgradeCount <= 0) {
        DrawText("None acquired", detailX, y, 14, DARKGRAY);
        return;
    }

    const int iconSize = 24;
    const int iconGap = 6;
    const int iconsPerRow = 6;
    float iconStartX = (float)detailX;
    float iconY = (float)y;

    for (int i = 0; i < entry->upgradeCount; i++) {
        int col = i % iconsPerRow;
        int row = i / iconsPerRow;
        float cx = iconStartX + col * (iconSize + iconGap) + iconSize / 2.0f;
        float cy = iconY + row * (iconSize + iconGap) + iconSize / 2.0f;

        Color rarityColor = GetUpgradeRarityColor(entry->upgrades[i].rarity);
        Rectangle badge = { cx - iconSize / 2.0f, cy - iconSize / 2.0f, (float)iconSize, (float)iconSize };
        DrawRectangleRounded(badge, 0.2f, 4, (Color){ 20, 22, 32, 200 });
        DrawRectangleRoundedLines(badge, 0.2f, 4, rarityColor);
        DrawUpgradeIcon(entry->upgrades[i].upgradeId, { cx, cy }, iconSize * 0.7f, rarityColor);
    }

    int upgradeTextY = y + ((entry->upgradeCount + iconsPerRow - 1) / iconsPerRow) * (iconSize + iconGap) + 8;
    int maxUpgradeLines = ((panelY + panelHeight) - upgradeTextY - 36) / 16;
    if (maxUpgradeLines < 1) maxUpgradeLines = 1;

    for (int i = 0; i < entry->upgradeCount && i < maxUpgradeLines; i++) {
        const char* title = GetUpgradeTitle(entry->upgrades[i].upgradeId);
        const char* rarityName = GetUpgradeRarityName(entry->upgrades[i].rarity);
        Color rarityColor = GetUpgradeRarityColor(entry->upgrades[i].rarity);
        snprintf(line, sizeof(line), "- %s (%s)", title, rarityName);
        DrawText(line, detailX, upgradeTextY + i * 16, 13, rarityColor);
    }

    if (entry->upgradeCount > maxUpgradeLines) {
        snprintf(line, sizeof(line), "... +%d more", entry->upgradeCount - maxUpgradeLines);
        DrawText(line, detailX, upgradeTextY + maxUpgradeLines * 16, 13, DARKGRAY);
    }
}

void RunHistoryScreen::DrawAsteroidsDetails(int panelX, int panelY, int panelWidth, int panelHeight) const {
    const int detailX = panelX + panelWidth / 2 + 8;
    const int detailY = panelY + 56;
    const int detailWidth = panelWidth / 2 - 24;
    int count = GetRunHistoryCount(RunHistoryMode::ASTEROIDS);

    DrawText("Run Details", detailX, detailY, 16, YELLOW);

    if (count <= 0 || selectedIndex < 0 || selectedIndex >= count) {
        return;
    }

    const AsteroidsRunEntry* entry = GetAsteroidsRunHistoryEntry(selectedIndex);
    if (!entry) return;

    const ShipStats* ship = shipSelection.GetShip(entry->shipType);
    const char* shipName = ship ? ship->name.c_str() : "Unknown";

    char timeStr[32];
    FormatPlayTime(entry->timeSeconds, timeStr, sizeof(timeStr));

    int y = detailY + 28;
    const int lineHeight = 22;
    char line[128];

    snprintf(line, sizeof(line), "Run #%d  (%s)", selectedIndex + 1, selectedIndex == 0 ? "Latest" : "Older");
    DrawText(line, detailX, y, 14, GOLD);
    y += lineHeight + 4;

    snprintf(line, sizeof(line), "Ship: %s", shipName);
    DrawText(line, detailX, y, 14, WHITE);
    y += lineHeight;

    snprintf(line, sizeof(line), "Score: %d   |   Scrap: %d", entry->score, entry->scrap);
    DrawText(line, detailX, y, 14, WHITE);
    y += lineHeight;

    if (entry->victory) {
        DrawText("Result: SECTOR SECURED", detailX, y, 14, (Color){ 120, 255, 160, 255 });
        y += lineHeight;
    }

    snprintf(line, sizeof(line), "Time: %s   |   Credits: +%d", timeStr, entry->creditsEarned);
    DrawText(line, detailX, y, 14, WHITE);
    y += lineHeight + 8;

    DrawLine(detailX, y, detailX + detailWidth, y, (Color){ 70, 70, 90, 255 });
    y += 14;

    DrawText("Classic survival mode — no draft upgrades.", detailX, y, 14, DARKGRAY);
}

void RunHistoryScreen::Draw() const {
    const char* title = "RUN HISTORY";
    const int titleSize = 32;
    DrawText(title, gameWidth / 2 - MeasureText(title, titleSize) / 2, 40, titleSize, (Color){ 210, 170, 110, 255 });

    DrawCreditsBadge(gameWidth - 180, 12, 18);

    const int panelWidth = 900;
    const int panelHeight = 430;
    const float panelX = (gameWidth - panelWidth) / 2.0f;
    const float panelY = 100.0f;
    const Rectangle panel = { panelX, panelY, (float)panelWidth, (float)panelHeight };

    DrawRectangleRounded(panel, 0.05f, 8, (Color){ 25, 25, 35, 240 });
    DrawRectangleRoundedLines(panel, 0.05f, 8, WHITE);

    DrawTabs((int)panelX, (int)panelY, panelWidth);

    DrawLine(panelX + panelWidth / 2.0f, panelY + 52.0f, panelX + panelWidth / 2.0f, panelY + panelHeight - 12.0f,
             (Color){ 70, 70, 90, 255 });

    const int listHeight = panelHeight - 24;
    DrawRunList((int)panelX, (int)panelY, panelWidth, listHeight);

    if (activeTab == RunHistoryMode::RIFT) {
        DrawRiftDetails((int)panelX, (int)panelY, panelWidth, panelHeight);
    } else {
        DrawAsteroidsDetails((int)panelX, (int)panelY, panelWidth, panelHeight);
    }

    const char* hint = "A / D: Switch tab   |   W / S: Browse runs   |   BACKSPACE: Menu";
    DrawText(hint, (int)panelX + panelWidth / 2 - MeasureText(hint, 14) / 2,
             (int)(panelY + panelHeight - 24), 14, DARKGRAY);
}
