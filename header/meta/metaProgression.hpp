#pragma once
#ifndef META_PROGRESSION_HPP
#define META_PROGRESSION_HPP

#include <raylib.h>
#include <cstddef>

enum MetaItemId {
    META_NEON_TRAIL = 0,
    META_GOLD_HULL,
    META_STARTING_HULL,
    META_CRIMSON_HULL,
    META_ION_TRAIL,
    META_OVERCHARGED_CELLS,
    META_MAGNETIC_SCOOP,
    META_BOUNTY_SCANNER,
    META_ITEM_COUNT
};

extern int playerCredits;
extern int lastRunCreditsEarned;
extern int riftVictories;
extern int asteroidsVictories;

void InitMetaProgression();
bool IsMetaOwned(int itemId);
void SetMetaOwned(int itemId, bool owned);
bool IsMetaEnabled(int itemId);
bool IsMetaCosmetic(int itemId);
bool TryToggleMetaItem(int itemId);
int GetMetaOwnedBitmask();
void ApplyMetaOwnedBitmask(int mask);
int GetMetaEnabledBitmask();
void ApplyMetaEnabledBitmask(int mask);
bool TryPurchaseMetaItem(int itemId);
const char* GetMetaItemName(int itemId);
const char* GetMetaItemDescription(int itemId);
int GetMetaItemCost(int itemId);
void AwardRunCredits(int kills, int scrap, float timeSeconds, int wave, bool victory = false);
void FinalizeRunCredits();
void DrawCreditsBadge(int x, int y, int fontSize = 18);

enum class CreditGoalLocation {
    HANGAR,
    COLLECTIONS
};

struct CreditGoal {
    char name[48];
    int totalCost;
    int creditsNeeded;
    CreditGoalLocation location;
};

bool GetNextCreditGoal(CreditGoal& out);
void FormatCreditsWithCommas(int value, char* buffer, size_t bufferSize);
void DrawCreditGoalNudge(int centerX, int y, int fontSize = 14);

#endif
