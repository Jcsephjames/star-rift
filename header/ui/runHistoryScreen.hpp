#pragma once
#ifndef RUN_HISTORY_SCREEN_HPP
#define RUN_HISTORY_SCREEN_HPP

#include "raylib.h"
#include "runHistory.hpp"
#include <cstddef>

class RunHistoryScreen {
public:
    RunHistoryScreen();

    void Update();
    void Draw() const;

private:
    RunHistoryMode activeTab;
    int selectedIndex;

    static void FormatPlayTime(float seconds, char* buffer, size_t bufferSize);
    void DrawTabs(int panelX, int panelY, int panelWidth) const;
    void DrawRunList(int panelX, int panelY, int panelWidth, int listHeight) const;
    void DrawRiftDetails(int panelX, int panelY, int panelWidth, int panelHeight) const;
    void DrawAsteroidsDetails(int panelX, int panelY, int panelWidth, int panelHeight) const;
};

#endif
