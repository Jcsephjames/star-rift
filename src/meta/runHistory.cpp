#include "runHistory.hpp"
#include "runModifiers.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

static RiftRunEntry riftEntries[MAX_RUN_HISTORY];
static int riftEntryCount = 0;
static AsteroidsRunEntry asteroidsEntries[MAX_RUN_HISTORY];
static int asteroidsEntryCount = 0;

static void ShiftRiftEntries() {
    for (int i = MAX_RUN_HISTORY - 1; i > 0; i--) {
        riftEntries[i] = riftEntries[i - 1];
    }
}

static void ShiftAsteroidsEntries() {
    for (int i = MAX_RUN_HISTORY - 1; i > 0; i--) {
        asteroidsEntries[i] = asteroidsEntries[i - 1];
    }
}

void InitRunHistory() {
    riftEntryCount = 0;
    asteroidsEntryCount = 0;
    for (int i = 0; i < MAX_RUN_HISTORY; i++) {
        riftEntries[i] = {};
        asteroidsEntries[i] = {};
    }
}

int GetRunHistoryCount(RunHistoryMode mode) {
    return mode == RunHistoryMode::RIFT ? riftEntryCount : asteroidsEntryCount;
}

const RiftRunEntry* GetRiftRunHistoryEntry(int index) {
    if (index < 0 || index >= riftEntryCount) return nullptr;
    return &riftEntries[index];
}

const AsteroidsRunEntry* GetAsteroidsRunHistoryEntry(int index) {
    if (index < 0 || index >= asteroidsEntryCount) return nullptr;
    return &asteroidsEntries[index];
}

void RecordRiftRunHistory(int shipType, int wave, int kills, int scrap, float timeSeconds, int creditsEarned, bool victory) {
    if (timeSeconds < 5.0f) return;

    ShiftRiftEntries();

    RiftRunEntry& entry = riftEntries[0];
    entry.shipType = shipType;
    entry.wave = wave;
    entry.kills = kills;
    entry.scrap = scrap;
    entry.timeSeconds = timeSeconds;
    entry.creditsEarned = creditsEarned;
    entry.victory = victory;
    entry.upgradeCount = runMods.acquiredCount;
    if (entry.upgradeCount > MAX_ACQUIRED_UPGRADES) {
        entry.upgradeCount = MAX_ACQUIRED_UPGRADES;
    }
    for (int i = 0; i < entry.upgradeCount; i++) {
        entry.upgrades[i] = runMods.acquired[i];
    }

    if (riftEntryCount < MAX_RUN_HISTORY) {
        riftEntryCount++;
    }
}

void UpdateLatestRiftRunHistory(int wave, int kills, int scrap, float timeSeconds) {
    if (riftEntryCount <= 0 || timeSeconds < 5.0f) return;

    RiftRunEntry& entry = riftEntries[0];
    entry.wave = wave;
    entry.kills = kills;
    entry.scrap = scrap;
    entry.timeSeconds = timeSeconds;
    entry.upgradeCount = runMods.acquiredCount;
    if (entry.upgradeCount > MAX_ACQUIRED_UPGRADES) {
        entry.upgradeCount = MAX_ACQUIRED_UPGRADES;
    }
    for (int i = 0; i < entry.upgradeCount; i++) {
        entry.upgrades[i] = runMods.acquired[i];
    }
}

void RecordAsteroidsRunHistory(int shipType, int score, int scrap, float timeSeconds, int creditsEarned, bool victory) {
    if (timeSeconds < 5.0f) return;

    ShiftAsteroidsEntries();

    AsteroidsRunEntry& entry = asteroidsEntries[0];
    entry.shipType = shipType;
    entry.score = score;
    entry.scrap = scrap;
    entry.timeSeconds = timeSeconds;
    entry.creditsEarned = creditsEarned;
    entry.victory = victory;

    if (asteroidsEntryCount < MAX_RUN_HISTORY) {
        asteroidsEntryCount++;
    }
}

static bool ParseUpgrades(const char* upgradesText, RiftRunEntry& entry) {
    entry.upgradeCount = 0;
    if (!upgradesText || upgradesText[0] == '\0') return true;

    char buffer[512];
    snprintf(buffer, sizeof(buffer), "%s", upgradesText);

    char* token = strtok(buffer, ",");
    while (token && entry.upgradeCount < MAX_ACQUIRED_UPGRADES) {
        int upgradeId = 0;
        int rarity = 0;
        if (sscanf(token, "%d:%d", &upgradeId, &rarity) == 2) {
            entry.upgrades[entry.upgradeCount].upgradeId = upgradeId;
            entry.upgrades[entry.upgradeCount].rarity = static_cast<UpgradeRarity>(rarity);
            entry.upgradeCount++;
        }
        token = strtok(nullptr, ",");
    }
    return true;
}

static bool ParseRiftEntry(const char* value, RiftRunEntry& entry) {
    entry = {};

    char buffer[768];
    snprintf(buffer, sizeof(buffer), "%s", value);

    char* tokens[9];
    int tokenCount = 0;
    char* token = strtok(buffer, "|");
    while (token && tokenCount < 9) {
        tokens[tokenCount++] = token;
        token = strtok(nullptr, "|");
    }

    if (tokenCount < 6) return false;

    entry.shipType = atoi(tokens[0]);
    entry.wave = atoi(tokens[1]);
    entry.kills = atoi(tokens[2]);
    entry.scrap = atoi(tokens[3]);
    entry.timeSeconds = (float)atof(tokens[4]);
    entry.creditsEarned = atoi(tokens[5]);

    if (tokenCount == 6) {
        return true;
    }

    if (tokenCount == 7) {
        if (strchr(tokens[6], ':') != nullptr || strchr(tokens[6], ',') != nullptr) {
            ParseUpgrades(tokens[6], entry);
        } else {
            entry.victory = (atoi(tokens[6]) != 0);
        }
        return true;
    }

    entry.victory = (atoi(tokens[6]) != 0);
    if (tokenCount >= 8) {
        ParseUpgrades(tokens[7], entry);
    }
    return true;
}

static bool ParseAsteroidsEntry(const char* value, AsteroidsRunEntry& entry) {
    entry = {};

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s", value);

    char* tokens[6];
    int tokenCount = 0;
    char* token = strtok(buffer, "|");
    while (token && tokenCount < 6) {
        tokens[tokenCount++] = token;
        token = strtok(nullptr, "|");
    }

    if (tokenCount < 5) return false;

    entry.shipType = atoi(tokens[0]);
    entry.score = atoi(tokens[1]);
    entry.scrap = atoi(tokens[2]);
    entry.timeSeconds = (float)atof(tokens[3]);
    entry.creditsEarned = atoi(tokens[4]);

    if (tokenCount >= 6) {
        entry.victory = (atoi(tokens[5]) != 0);
    }

    return true;
}

static void AppendRiftSaveLines(char* text, int* offset, size_t capacity) {
    *offset += snprintf(text + *offset, capacity - (size_t)*offset, "riftRunHistoryCount=%d\n", riftEntryCount);

    for (int i = 0; i < riftEntryCount; i++) {
        const RiftRunEntry& entry = riftEntries[i];
        char upgradesText[512] = { 0 };
        int upgradeOffset = 0;

        for (int j = 0; j < entry.upgradeCount; j++) {
            if (j > 0) {
                upgradeOffset += snprintf(
                    upgradesText + upgradeOffset,
                    sizeof(upgradesText) - (size_t)upgradeOffset,
                    ","
                );
            }
            upgradeOffset += snprintf(
                upgradesText + upgradeOffset,
                sizeof(upgradesText) - (size_t)upgradeOffset,
                "%d:%d",
                entry.upgrades[j].upgradeId,
                static_cast<int>(entry.upgrades[j].rarity)
            );
        }

        *offset += snprintf(
            text + *offset,
            capacity - (size_t)*offset,
            "riftRunHistory%d=%d|%d|%d|%d|%.1f|%d|%d|%s\n",
            i,
            entry.shipType,
            entry.wave,
            entry.kills,
            entry.scrap,
            entry.timeSeconds,
            entry.creditsEarned,
            entry.victory ? 1 : 0,
            upgradesText
        );
    }
}

static void AppendAsteroidsSaveLines(char* text, int* offset, size_t capacity) {
    *offset += snprintf(
        text + *offset,
        capacity - (size_t)*offset,
        "asteroidsRunHistoryCount=%d\n",
        asteroidsEntryCount
    );

    for (int i = 0; i < asteroidsEntryCount; i++) {
        const AsteroidsRunEntry& entry = asteroidsEntries[i];
        *offset += snprintf(
            text + *offset,
            capacity - (size_t)*offset,
            "asteroidsRunHistory%d=%d|%d|%d|%.1f|%d|%d\n",
            i,
            entry.shipType,
            entry.score,
            entry.scrap,
            entry.timeSeconds,
            entry.creditsEarned,
            entry.victory ? 1 : 0
        );
    }
}

bool TryParseRunHistoryKey(const char* key, const char* value) {
    if (strcmp(key, "riftRunHistoryCount") == 0 || strcmp(key, "runHistoryCount") == 0) {
        riftEntryCount = atoi(value);
        if (riftEntryCount < 0) riftEntryCount = 0;
        if (riftEntryCount > MAX_RUN_HISTORY) riftEntryCount = MAX_RUN_HISTORY;
        return true;
    }

    if (strcmp(key, "asteroidsRunHistoryCount") == 0) {
        asteroidsEntryCount = atoi(value);
        if (asteroidsEntryCount < 0) asteroidsEntryCount = 0;
        if (asteroidsEntryCount > MAX_RUN_HISTORY) asteroidsEntryCount = MAX_RUN_HISTORY;
        return true;
    }

    int index = 0;
    if (sscanf(key, "riftRunHistory%d", &index) == 1 || sscanf(key, "runHistory%d", &index) == 1) {
        if (index < 0 || index >= MAX_RUN_HISTORY) return true;

        RiftRunEntry entry = {};
        if (ParseRiftEntry(value, entry)) {
            riftEntries[index] = entry;
        }
        return true;
    }

    if (sscanf(key, "asteroidsRunHistory%d", &index) == 1) {
        if (index < 0 || index >= MAX_RUN_HISTORY) return true;

        AsteroidsRunEntry entry = {};
        if (ParseAsteroidsEntry(value, entry)) {
            asteroidsEntries[index] = entry;
        }
        return true;
    }

    return false;
}

void AppendRunHistorySaveLines(char* text, int* offset, size_t capacity) {
    AppendRiftSaveLines(text, offset, capacity);
    AppendAsteroidsSaveLines(text, offset, capacity);
}
