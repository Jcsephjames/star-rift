#pragma once
#ifndef CARD_DRAFT_HPP
#define CARD_DRAFT_HPP

#include <raylib.h>

#define CARD_DRAFT_COUNT 4
#define SCRAP_PER_DRAFT 100
#define SCRAP_MILESTONE_STEP 10

enum class UpgradeRarity {
    COMMON,
    RARE,
    EPIC,
    LEGENDARY
};

struct DraftCard {
    int upgradeId;
    const char* title;
    const char* description;
    UpgradeRarity rarity;
};

Color GetUpgradeRarityColor(UpgradeRarity rarity);
const char* GetUpgradeRarityName(UpgradeRarity rarity);
const char* GetUpgradeTitle(int upgradeId);
void DrawUpgradeIcon(int upgradeId, Vector2 center, float size, Color color);

void InitCardDraft();
void ResetCardDraftMilestone();
bool IsCardDraftOpen();
void CheckCardDraftTrigger();
void UpdateCardDraft();
void DrawCardDraft();
void DrawActiveBuildSummary(int x, int y);

#endif
