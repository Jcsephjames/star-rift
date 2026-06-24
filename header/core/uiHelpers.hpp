#pragma once
#ifndef UI_HELPERS_HPP
#define UI_HELPERS_HPP

#include "raylib.h"
#include "globals.hpp"

inline void DrawStatCard(Rectangle rect, const char* label, int value, Color cardColor) {
    DrawRectangleRounded(rect, 0.25f, 6, cardColor);
    DrawRectangleRoundedLines(rect, 0.25f, 6, WHITE);

    int labelFontSize = 14;
    int valueFontSize = 22;
    int maxLabelWidth = (int)rect.width - 8;
    int labelWidth = MeasureText(label, labelFontSize);
    while (labelWidth > maxLabelWidth && labelFontSize > 9) {
        labelFontSize--;
        labelWidth = MeasureText(label, labelFontSize);
    }

    const char* valueText = TextFormat("%d", value);
    int valueWidth = MeasureText(valueText, valueFontSize);

    DrawText(label, rect.x + rect.width / 2 - labelWidth / 2, rect.y + 5, labelFontSize, WHITE);
    DrawText(valueText, rect.x + rect.width / 2 - valueWidth / 2,
             rect.y + rect.height - valueFontSize - 5, valueFontSize, YELLOW);
}

inline void DrawCenteredText(const char* text, int y, int fontSize, Color color) {
    int x = GetScreenWidth() / 2 - MeasureText(text, fontSize) / 2;
    DrawText(text, x, y, fontSize, color);
}

inline void DrawControlsOverlay(const char* dismissText = "Press C to close") {
    DrawRectangle(0, 0, gameWidth, gameHeight, (Color){ 0, 0, 0, 180 });

    const int panelWidth = 620;
    const int panelHeight = 510;
    const float panelX = (gameWidth - panelWidth) / 2.0f;
    const float panelY = (gameHeight - panelHeight) / 2.0f;
    const Rectangle panel = { panelX, panelY, (float)panelWidth, (float)panelHeight };
    const int paddingX = 32;
    const int paddingTop = 22;
    const int paddingBottom = 28;
    const int titleSize = 26;
    const int fontSize = 16;
    const int lineHeight = 22;
    const int sectionGap = 12;
    const int keysColumnWidth = 190;
    const float contentLeft = panelX + paddingX;
    const float keysX = contentLeft;
    const float actionX = contentLeft + keysColumnWidth;
    const float contentWidth = panelWidth - 2 * paddingX;

    DrawRectangleRounded(panel, 0.05f, 8, (Color){ 25, 25, 35, 240 });
    DrawRectangleRoundedLines(panel, 0.05f, 8, WHITE);

    const char* title = "CONTROLS";
    DrawText(title, panelX + panelWidth / 2 - MeasureText(title, titleSize) / 2, panelY + paddingTop, titleSize, WHITE);

    int y = panelY + paddingTop + titleSize + 18;

    auto drawSection = [&](const char* heading) {
        DrawText(heading, contentLeft, y, fontSize, YELLOW);
        y += lineHeight + 4;
    };

    auto drawLine = [&](const char* keys, const char* action) {
        DrawText(keys, keysX, y, fontSize, LIGHTGRAY);
        DrawText(action, actionX, y, fontSize, WHITE);
        y += lineHeight;
    };

    auto drawSeparator = [&]() {
        y += 6;
        DrawLine(contentLeft, (float)y, contentLeft + contentWidth, (float)y, (Color){ 70, 70, 90, 255 });
        y += sectionGap;
    };

    drawSection("Main Menu");
    drawLine("W / S", "Navigate options");
    drawLine("ENTER", "Select option");
    drawLine("C", "Toggle controls");

    drawSeparator();
    drawSection("Ship Selection");
    drawLine("A / D, Arrows", "Browse ships");
    drawLine("W / S", "Change difficulty");
    drawLine("ENTER", "Confirm ship");
    drawLine("BACKSPACE", "Back to menu");

    drawSeparator();
    drawSection("In-Game");
    drawLine("Q / E", "Heavy rotate");
    drawLine("A / D", "Light rotate");
    drawLine("S", "Brake");
    drawLine("LEFT SHIFT", "Boost");
    drawLine("SPACE", "Fire lasers");
    drawLine("P", "Pause");
    drawLine("BACKSPACE", "Back to menu");

    const int dismissY = panelY + panelHeight - paddingBottom - fontSize;
    DrawText(dismissText, panelX + panelWidth / 2 - MeasureText(dismissText, fontSize) / 2, dismissY, fontSize, RED);
}

#endif
