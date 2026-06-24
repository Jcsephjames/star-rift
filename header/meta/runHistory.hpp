#pragma once
#ifndef RUN_HISTORY_HPP
#define RUN_HISTORY_HPP

#include "runModifiers.hpp"
#include <cstddef>

#define MAX_RUN_HISTORY 10

enum class RunHistoryMode {
    RIFT,
    ASTEROIDS
};

struct RiftRunEntry {
    int shipType = 0;
    int wave = 0;
    int kills = 0;
    int scrap = 0;
    float timeSeconds = 0.0f;
    int creditsEarned = 0;
    bool victory = false;
    int upgradeCount = 0;
    AcquiredUpgrade upgrades[MAX_ACQUIRED_UPGRADES];
};

struct AsteroidsRunEntry {
    int shipType = 0;
    int score = 0;
    int scrap = 0;
    float timeSeconds = 0.0f;
    int creditsEarned = 0;
    bool victory = false;
};

void InitRunHistory();
int GetRunHistoryCount(RunHistoryMode mode);
const RiftRunEntry* GetRiftRunHistoryEntry(int index);
const AsteroidsRunEntry* GetAsteroidsRunHistoryEntry(int index);
void RecordRiftRunHistory(int shipType, int wave, int kills, int scrap, float timeSeconds, int creditsEarned, bool victory);
void UpdateLatestRiftRunHistory(int wave, int kills, int scrap, float timeSeconds);
void RecordAsteroidsRunHistory(int shipType, int score, int scrap, float timeSeconds, int creditsEarned, bool victory);
bool TryParseRunHistoryKey(const char* key, const char* value);
void AppendRunHistorySaveLines(char* text, int* offset, size_t capacity);

#endif
