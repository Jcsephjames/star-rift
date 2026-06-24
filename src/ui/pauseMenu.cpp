#include "pauseMenu.hpp"
#include "game.hpp"
#include "globals.hpp"
#include "uiHelpers.hpp"
#include "audio.hpp"
#include "raylib.h"

extern Game game;

static int pauseSelection = 0;
static bool showControls = false;
static constexpr int PAUSE_OPTION_COUNT = 3;

void InitPauseMenu() {
    pauseSelection = 0;
    showControls = false;
    game.pauseMenuOpen = false;
}

void OpenPauseMenu() {
    pauseSelection = 0;
    showControls = false;
    game.pauseMenuOpen = true;
}

void UpdatePauseMenu() {
    if (showControls) {
        if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_BACKSPACE)) {
            showControls = false;
        }
        return;
    }

    if (IsKeyPressed(KEY_P)) {
        game.pauseMenuOpen = false;
        return;
    }

    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        pauseSelection++;
        if (pauseSelection >= PAUSE_OPTION_COUNT) pauseSelection = 0;
        PlaySfx(SfxId::MENU_NAVIGATE);
    }
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        pauseSelection--;
        if (pauseSelection < 0) pauseSelection = PAUSE_OPTION_COUNT - 1;
        PlaySfx(SfxId::MENU_NAVIGATE);
    }

    if (IsKeyPressed(KEY_ENTER)) {
        PlaySfx(SfxId::MENU_CONFIRM);
        switch (pauseSelection) {
            case 0:
                game.pauseMenuOpen = false;
                break;
            case 1:
                showControls = true;
                break;
            case 2:
                game.returnToMainMenu = true;
                game.pauseMenuOpen = false;
                break;
        }
    }
}

void DrawPauseMenu() {
    if (!game.pauseMenuOpen) return;

    DrawRectangle(0, 0, gameWidth, gameHeight, (Color){ 0, 0, 0, 180 });

    if (showControls) {
        DrawControlsOverlay("Press P, ENTER, or BACKSPACE to close");
        return;
    }

    const int panelWidth = 360;
    const int panelHeight = 260;
    const float panelX = (gameWidth - panelWidth) / 2.0f;
    const float panelY = (gameHeight - panelHeight) / 2.0f;
    const Rectangle panel = { panelX, panelY, (float)panelWidth, (float)panelHeight };

    DrawRectangleRounded(panel, 0.05f, 8, (Color){ 25, 25, 35, 240 });
    DrawRectangleRoundedLines(panel, 0.05f, 8, WHITE);

    const char* title = "PAUSED";
    const int titleSize = 28;
    DrawText(title, panelX + panelWidth / 2 - MeasureText(title, titleSize) / 2,
             panelY + 24, titleSize, WHITE);

    const char* options[] = { "Resume", "Controls", "Quit to Menu" };
    const int optionSize = 20;
    const int optionStartY = panelY + 90;
    const int optionSpacing = 36;

    for (int i = 0; i < PAUSE_OPTION_COUNT; i++) {
        Color color = (i == pauseSelection) ? WHITE : DARKGRAY;
        int textWidth = MeasureText(options[i], optionSize);
        DrawText(options[i], panelX + panelWidth / 2 - textWidth / 2,
                 optionStartY + i * optionSpacing, optionSize, color);
    }

    const char* hint = "W/S to navigate  |  ENTER to select  |  P to resume";
    const int hintSize = 14;
    DrawText(hint, panelX + panelWidth / 2 - MeasureText(hint, hintSize) / 2,
             panelY + panelHeight - 36, hintSize, GRAY);
}
