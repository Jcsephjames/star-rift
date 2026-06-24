#include "metaProgression.hpp"
#include "globals.hpp"
#include "saveData.hpp"
#include "game.hpp"
#include "enemies.hpp"
#include "runHistory.hpp"
#include "player.hpp"
#include "unlocks.hpp"
#include <cstdio>
#include <climits>

extern Game game;

int playerCredits = 0;
int lastRunCreditsEarned = 0;
int riftVictories = 0;
int asteroidsVictories = 0;

static bool metaOwned[META_ITEM_COUNT] = { false };
static bool metaEnabled[META_ITEM_COUNT] = { false };

static struct MetaItemDef {
    const char* name;
    const char* description;
    int cost;
    bool cosmetic;
} metaItems[META_ITEM_COUNT] = {
    { "Neon Trail", "Neon-colored engine exhaust", 500, true },
    { "Gold Hull", "Cosmetic gold ship tint", 750, true },
    { "Reinforced Frame", "Permanent +5 max hull", 1000, false },
    { "Crimson Hull", "Cosmetic crimson ship tint", 850, true },
    { "Ion Trail", "Cool blue ion engine exhaust", 650, true },
    { "Overcharged Cells", "Permanent +1 laser damage", 1250, false },
    { "Magnetic Scoop", "Permanent +30% pickup radius", 1100, false },
    { "Bounty Scanner", "Permanent +15% run credits", 1500, false }
};

void InitMetaProgression() {
    for (int i = 0; i < META_ITEM_COUNT; i++) {
        metaOwned[i] = false;
        metaEnabled[i] = false;
    }
}

bool IsMetaOwned(int itemId) {
    if (itemId < 0 || itemId >= META_ITEM_COUNT) return false;
    return metaOwned[itemId];
}

void SetMetaOwned(int itemId, bool owned) {
    if (itemId < 0 || itemId >= META_ITEM_COUNT) return;
    metaOwned[itemId] = owned;
    if (!owned) {
        metaEnabled[itemId] = false;
    }
}

bool IsMetaEnabled(int itemId) {
    if (itemId < 0 || itemId >= META_ITEM_COUNT) return false;
    return metaOwned[itemId] && metaEnabled[itemId];
}

bool IsMetaCosmetic(int itemId) {
    if (itemId < 0 || itemId >= META_ITEM_COUNT) return false;
    return metaItems[itemId].cosmetic;
}

bool TryToggleMetaItem(int itemId) {
    if (itemId < 0 || itemId >= META_ITEM_COUNT) return false;
    if (!metaOwned[itemId] || !metaItems[itemId].cosmetic) return false;

    metaEnabled[itemId] = !metaEnabled[itemId];
    SaveGameData();
    return true;
}

bool TryPurchaseMetaItem(int itemId) {
    if (itemId < 0 || itemId >= META_ITEM_COUNT) return false;
    if (metaOwned[itemId]) return false;
    if (playerCredits < metaItems[itemId].cost) return false;

    playerCredits -= metaItems[itemId].cost;
    metaOwned[itemId] = true;
    if (metaItems[itemId].cosmetic) {
        metaEnabled[itemId] = true;
    }
    SaveGameData();
    return true;
}

int GetMetaOwnedBitmask() {
    int mask = 0;
    for (int i = 0; i < META_ITEM_COUNT; i++) {
        if (metaOwned[i]) mask |= (1 << i);
    }
    return mask;
}

void ApplyMetaOwnedBitmask(int mask) {
    for (int i = 0; i < META_ITEM_COUNT; i++) {
        metaOwned[i] = (mask & (1 << i)) != 0;
        if (!metaOwned[i]) {
            metaEnabled[i] = false;
        }
    }
}

int GetMetaEnabledBitmask() {
    int mask = 0;
    for (int i = 0; i < META_ITEM_COUNT; i++) {
        if (metaEnabled[i]) mask |= (1 << i);
    }
    return mask;
}

void ApplyMetaEnabledBitmask(int mask) {
    for (int i = 0; i < META_ITEM_COUNT; i++) {
        metaEnabled[i] = metaOwned[i] && (mask & (1 << i)) != 0;
    }
}

const char* GetMetaItemName(int itemId) {
    if (itemId < 0 || itemId >= META_ITEM_COUNT) return "Unknown";
    return metaItems[itemId].name;
}

const char* GetMetaItemDescription(int itemId) {
    if (itemId < 0 || itemId >= META_ITEM_COUNT) return "";
    return metaItems[itemId].description;
}

int GetMetaItemCost(int itemId) {
    if (itemId < 0 || itemId >= META_ITEM_COUNT) return 0;
    return metaItems[itemId].cost;
}

void AwardRunCredits(int kills, int scrap, float timeSeconds, int wave, bool victory) {
    if (timeSeconds < 5.0f) {
        lastRunCreditsEarned = 0;
        return;
    }

    int earned = kills * 3 + scrap / 4 + (int)(timeSeconds / 2.0f) + wave * 8;
    if (victory) {
        earned += 500 + wave * 25;
    }
    if (IsMetaOwned(META_BOUNTY_SCANNER)) {
        earned = (int)(earned * 1.15f + 0.5f);
    }
    if (earned < 5) earned = 5;

    lastRunCreditsEarned = earned;
    playerCredits += earned;
    SaveGameData();
}

void FinalizeRunCredits() {
    if (game.runCreditsFinalized) return;
    game.runCreditsFinalized = true;

    if (enemiesOn && !asteroidsMode) {
        AwardRunCredits(currentRun.score, currentRun.scrap, currentRun.time, GetEnemyWave(), game.victory);
        if (game.victory) {
            riftVictories++;
        }
        RecordRiftRunHistory(
            player.starshipType,
            GetEnemyWave(),
            currentRun.score,
            currentRun.scrap,
            currentRun.time,
            lastRunCreditsEarned,
            game.victory
        );
        SaveGameData();
    } else if (asteroidsMode) {
        AwardRunCredits(0, currentRun.scrap, currentRun.time, 0, game.victory);
        if (game.victory) {
            asteroidsVictories++;
        }
        RecordAsteroidsRunHistory(
            player.starshipType,
            currentRun.score,
            currentRun.scrap,
            currentRun.time,
            lastRunCreditsEarned,
            game.victory
        );
        SaveGameData();
    }
}

void DrawCreditsBadge(int x, int y, int fontSize) {
    char buffer[48];
    snprintf(buffer, sizeof(buffer), "Credits: %d", playerCredits);
    int textW = MeasureText(buffer, fontSize);
    DrawRectangle(x - 8, y - 4, textW + 16, fontSize + 8, (Color){ 30, 30, 40, 180 });
    DrawText(buffer, x, y, fontSize, (Color){ 120, 220, 255, 255 });
}

void FormatCreditsWithCommas(int value, char* buffer, size_t bufferSize) {
    char raw[16];
    snprintf(raw, sizeof(raw), "%d", value);

    size_t rawLen = 0;
    while (raw[rawLen] != '\0') rawLen++;

    size_t commaCount = rawLen > 3 ? (rawLen - 1) / 3 : 0;
    size_t outLen = rawLen + commaCount;
    if (outLen + 1 > bufferSize) {
        snprintf(buffer, bufferSize, "%d", value);
        return;
    }

    size_t outIdx = outLen;
    buffer[outIdx] = '\0';
    for (size_t i = 0; i < rawLen; i++) {
        buffer[--outIdx] = raw[rawLen - 1 - i];
        if (i % 3 == 2 && i + 1 < rawLen) {
            buffer[--outIdx] = ',';
        }
    }
}

static void ConsiderCreditGoal(
    CreditGoal& best,
    bool& found,
    const char* name,
    int cost,
    CreditGoalLocation location
) {
    if (cost <= 0) return;

    int needed = cost - playerCredits;
    if (needed < 0) needed = 0;

    if (!found || needed < best.creditsNeeded ||
        (needed == best.creditsNeeded && cost < best.totalCost)) {
        found = true;
        best.totalCost = cost;
        best.creditsNeeded = needed;
        best.location = location;
        snprintf(best.name, sizeof(best.name), "%s", name);
    }
}

bool GetNextCreditGoal(CreditGoal& out) {
    CreditGoal best = {};
    bool found = false;

    for (int i = 0; i < META_ITEM_COUNT; i++) {
        if (IsMetaOwned(i)) continue;
        ConsiderCreditGoal(best, found, GetMetaItemName(i), GetMetaItemCost(i), CreditGoalLocation::HANGAR);
    }

    for (int i = 0; i < MAX_SHIPS; i++) {
        if (IsShipUnlocked(i)) continue;
        const ShipStats* ship = shipSelection.GetShip(i);
        if (!ship) continue;
        ConsiderCreditGoal(best, found, ship->name.c_str(), GetShipCreditCost(i), CreditGoalLocation::COLLECTIONS);
    }

    if (!found) return false;
    out = best;
    return true;
}

void DrawCreditGoalNudge(int centerX, int y, int fontSize) {
    CreditGoal goal;
    if (!GetNextCreditGoal(goal)) {
        const char* done = "All upgrades unlocked!";
        DrawText(done, centerX - MeasureText(done, fontSize) / 2, y, fontSize, LIME);
        return;
    }

    char costStr[16];
    char line[128];
    FormatCreditsWithCommas(goal.totalCost, costStr, sizeof(costStr));

    Color textColor;
    if (goal.creditsNeeded == 0) {
        const char* location = goal.location == CreditGoalLocation::HANGAR ? "Hangar" : "Collections";
        snprintf(line, sizeof(line), "Unlock %s now! (%s credits in %s)",
                 goal.name, costStr, location);
        textColor = (Color){ 120, 255, 180, 255 };
    } else {
        char neededStr[16];
        FormatCreditsWithCommas(goal.creditsNeeded, neededStr, sizeof(neededStr));
        snprintf(line, sizeof(line), "%s credits until %s", neededStr, goal.name);
        textColor = (Color){ 210, 180, 100, 255 };
    }

    DrawText(line, centerX - MeasureText(line, fontSize) / 2, y, fontSize, textColor);
}
