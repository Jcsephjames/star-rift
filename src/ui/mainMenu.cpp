#include "raylib.h"
#include "mainMenu.hpp"
#include "globals.hpp"
#include "game.hpp"
#include "textures.hpp"
#include "metaProgression.hpp"
#include "audio.hpp"
#include "uiHelpers.hpp"

void MainMenu::Update() {
    if (IsKeyPressed(KEY_C)) {
        showControls = !showControls;
    }
    if (showControls) {
        return;
    }

    if (IsKeyPressed(KEY_S)) {
        menuSelection++;
        if (menuSelection > menuOptions) {
            menuSelection = 0; // Loop back to the first option
        }
        PlaySfx(SfxId::MENU_NAVIGATE);
    }
    if (IsKeyPressed(KEY_W)) {
        menuSelection--;
        if (menuSelection < 0) {
            menuSelection = menuOptions; // Loop back to the last option
        }
        PlaySfx(SfxId::MENU_NAVIGATE);
    }
    
    if (IsKeyPressed(KEY_ENTER)) {
        PlaySfx(SfxId::MENU_CONFIRM);
        switch (menuSelection) 
        {
            case 0: { // Rift Fight
                asteroidsMode = false;
                yWrapping = false;
                asteroidsOn = true;
                enemiesOn = true;
                startGame = true;
                break;
            }
            case 1: { // Asteroids Mode
                asteroidsMode = true;
                asteroidsOn = true;
                enemiesOn = false;
                yWrapping = true;
                startGame = true;
                break;
            }
            case 2: { // Hangar
                hangar = true;
                break;
            }
            case 3: { // Collections
                collections = true;
                break;
            }
            case 4: { // Run History
                runHistory = true;
                break;
            }
            case 5: { // Settings
                settings = true;
                break;
            }
        }
    }
    
}

void MainMenu::Draw() {

    Color selectionColor = WHITE;
    Color nonSelectionColor = DARKGRAY;

    Color riftFightColor = (menuSelection == 0) ? selectionColor : nonSelectionColor;
    Color asteroidsColor = (menuSelection == 1) ? selectionColor : nonSelectionColor;
    Color hangarColor = (menuSelection == 2) ? selectionColor : nonSelectionColor;
    Color collectionsColor = (menuSelection == 3) ? selectionColor : nonSelectionColor;
    Color runHistoryColor = (menuSelection == 4) ? selectionColor : nonSelectionColor;
    Color settingsColor = (menuSelection == 5) ? selectionColor : nonSelectionColor;

    DrawTexture(starRift, gameWidth/2 - starRift.width/2, 120, WHITE);
    DrawCreditsBadge(gameWidth - 180, 12, 18);

    char victoriesLine[64];
    snprintf(victoriesLine, sizeof(victoriesLine), "Rift Victories: %d", riftVictories);
    DrawText(victoriesLine, gameWidth / 2 - MeasureText(victoriesLine, 16) / 2, 348, 16,
             (Color){ 120, 255, 160, 255 });
    snprintf(victoriesLine, sizeof(victoriesLine), "Asteroids Victories: %d", asteroidsVictories);
    DrawText(victoriesLine, gameWidth / 2 - MeasureText(victoriesLine, 16) / 2, 366, 16,
             (Color){ 120, 255, 160, 255 });

    DrawText("RIFT FIGHT", gameWidth/2 - MeasureText("RIFT FIGHT", 20)/2, 390, 20, riftFightColor);
    DrawText("ASTEROIDS MODE", gameWidth/2 - MeasureText("ASTEROIDS MODE", 20)/2, 415, 20, asteroidsColor);
    DrawText("HANGAR", gameWidth/2 - MeasureText("HANGAR", 20)/2, 440, 20, hangarColor);
    DrawText("COLLECTIONS", gameWidth/2 - MeasureText("COLLECTIONS", 20)/2, 465, 20, collectionsColor);
    DrawText("RUN HISTORY", gameWidth/2 - MeasureText("RUN HISTORY", 20)/2, 490, 20, runHistoryColor);
    DrawText("SETTINGS", gameWidth/2 - MeasureText("SETTINGS", 20)/2, 515, 20, settingsColor);

    DrawCreditGoalNudge(gameWidth / 2, 548, 14);

    DrawText("Press C for Controls", 5, gameHeight - 20, 20, RED);
    DrawText("Press ESC to Exit", gameWidth - MeasureText("Press ESC to Exit", 20), gameHeight - 20, 20, RED);

    if (showControls) {
        DrawControlsOverlay();
    }
}

bool MainMenu::ShouldStartGame() const {
    return startGame;
}
bool MainMenu::ShouldOpenSettings() const {
    return settings;
}
bool MainMenu::ShouldOpenCollections() const {
    return collections;
}
bool MainMenu::ShouldOpenHangar() const {
    return hangar;
}
bool MainMenu::ShouldOpenRunHistory() const {
    return runHistory;
}