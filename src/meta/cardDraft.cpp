#include "cardDraft.hpp"
#include "runModifiers.hpp"
#include "globals.hpp"
#include "game.hpp"
#include "enemies.hpp"
#include "player.hpp"
#include "effects.hpp"
#include "audio.hpp"
#include "laserManager.hpp"
#include <cstdio>
#include <cmath>

extern Game game;

static bool draftOpen = false;
static int draftSelection = 0;
static int nextDraftMilestone = SCRAP_PER_DRAFT;
static UpgradeRarity currentDraftRarity = UpgradeRarity::COMMON;
static DraftCard draftCards[CARD_DRAFT_COUNT] = { 0 };

static constexpr int DRAFT_WEIGHT_COMMON = 55;
static constexpr int DRAFT_WEIGHT_RARE = 28;
static constexpr int DRAFT_WEIGHT_EPIC = 14;
static constexpr int DRAFT_WEIGHT_LEGENDARY = 3;

static struct UpgradeDef {
    int id;
    const char* title;
    const char* description;
    UpgradeRarity rarity;
} upgradePool[] = {
    { 0, "Full Repair", "Restore hull to 100%", UpgradeRarity::COMMON },

    // Commons
    { 1, "Focused Optics", "+10% laser damage", UpgradeRarity::COMMON },
    { 2, "Kinetic Ram", "+10% melee damage", UpgradeRarity::COMMON },
    { 3, "Hull Padding", "+10% melee damage reduction", UpgradeRarity::COMMON },
    { 4, "Deflector Coating", "+10% laser damage reduction", UpgradeRarity::COMMON },
    { 5, "Quick Trigger", "-5% shot cooldown", UpgradeRarity::COMMON },
    { 6, "Thruster Wash", "+10% ship speed", UpgradeRarity::COMMON },
    { 7, "Scrap Sifter", "+10% scrap gained", UpgradeRarity::COMMON },
    { 8, "Magnetic Rig", "+50% pickup range", UpgradeRarity::COMMON },
    { 9, "Reinforced Frame", "+10% max hull", UpgradeRarity::COMMON },
    { 10, "Tailshot Gyro", "+0.5x tailshot damage", UpgradeRarity::COMMON },
    { 11, "Extended Tanks", "+20 max boost", UpgradeRarity::COMMON },
    { 12, "Medkit Tuning", "+50% health pickup heal", UpgradeRarity::COMMON },

    // Rares
    { 13, "Mirror Plating", "+20% laser reflect chance", UpgradeRarity::RARE },
    { 14, "Erratic Turret", "Random laser every 4s", UpgradeRarity::RARE },
    { 15, "Rapid Cycle", "-10% shot cooldown", UpgradeRarity::RARE },
    { 16, "Field Surgeon", "+100% health pickup heal", UpgradeRarity::RARE },
    { 17, "Reinforced Ram", "+20% melee damage", UpgradeRarity::RARE },
    { 18, "Rear Gunner", "+0.5x tailshot damage", UpgradeRarity::RARE },
    { 19, "Salvage Tuning", "+25% scrap gained", UpgradeRarity::RARE },
    { 20, "Boost Recovery", "-25% boost stall time", UpgradeRarity::RARE },

    // Epics
    { 21, "Reflect Matrix", "+20% laser reflect chance", UpgradeRarity::EPIC },
    { 22, "Overdrive Package", "+25% boost regen, -1s stall", UpgradeRarity::EPIC },
    { 23, "Scrap Hoarder", "+40% scrap gained", UpgradeRarity::EPIC },
    { 24, "Bulkhead Upgrade", "+20 max hull", UpgradeRarity::EPIC },
    { 25, "Ramming Speed", "+30% collision dmg, +10% speed", UpgradeRarity::EPIC },
    { 26, "Siege Lasers", "+30% laser dmg, -10% cooldown", UpgradeRarity::EPIC },

    // Legendaries
    { 27, "Juggernaut Protocol", "+100% collision dmg", UpgradeRarity::LEGENDARY },
    { 28, "Scrap Magnate", "+200% scrap, +100% pickup range", UpgradeRarity::LEGENDARY },
    { 29, "Fortress Hull", "+50% max hull", UpgradeRarity::LEGENDARY },
    { 30, "Mirror Fortress", "+50% reflect chance", UpgradeRarity::LEGENDARY },
    { 31, "Storm Turret", "Random laser every 1s", UpgradeRarity::LEGENDARY }
};

static const int UPGRADE_POOL_SIZE = sizeof(upgradePool) / sizeof(upgradePool[0]);

static const char* GetUpgradeTradeOffWarning(int upgradeId) {
    switch (upgradeId) {
        case 27: return "-50% laser damage";
        case 29: return "Cannot boost";
        case 30: return "+30% melee damage taken";
        default: return nullptr;
    }
}

Color GetUpgradeRarityColor(UpgradeRarity rarity) {
    switch (rarity) {
        case UpgradeRarity::COMMON:    return (Color){ 80, 255, 140, 255 };
        case UpgradeRarity::RARE:      return (Color){ 100, 180, 255, 255 };
        case UpgradeRarity::EPIC:      return (Color){ 180, 140, 255, 255 };
        case UpgradeRarity::LEGENDARY: return (Color){ 255, 210, 80, 255 };
    }
    return WHITE;
}

const char* GetUpgradeRarityName(UpgradeRarity rarity) {
    switch (rarity) {
        case UpgradeRarity::COMMON:    return "COMMON";
        case UpgradeRarity::RARE:      return "RARE";
        case UpgradeRarity::EPIC:      return "EPIC";
        case UpgradeRarity::LEGENDARY: return "LEGENDARY";
    }
    return "";
}

const char* GetUpgradeTitle(int upgradeId) {
    for (int i = 0; i < UPGRADE_POOL_SIZE; i++) {
        if (upgradePool[i].id == upgradeId) {
            return upgradePool[i].title;
        }
    }
    return "Unknown";
}

void DrawUpgradeIcon(int upgradeId, Vector2 center, float size, Color color) {
    float s = size * 0.5f;
    switch (upgradeId) {
        case 0: { // Full Repair
            DrawLineEx({ center.x - s, center.y }, { center.x + s, center.y }, 3.0f, color);
            DrawLineEx({ center.x, center.y - s }, { center.x, center.y + s }, 3.0f, color);
            DrawCircleLinesV(center, s * 0.7f, Fade(color, 0.5f));
            break;
        }
        case 1: case 26: { // Laser damage
            Vector2 bolt[] = {
                { center.x + s * 0.1f, center.y - s },
                { center.x - s * 0.3f, center.y + s * 0.05f },
                { center.x + s * 0.15f, center.y + s * 0.05f },
                { center.x - s * 0.1f, center.y + s }
            };
            for (int i = 0; i < 3; i++) DrawLineEx(bolt[i], bolt[i + 1], 3.0f, color);
            break;
        }
        case 2: case 17: case 27: { // Melee / collision
            DrawLineEx({ center.x - s * 0.8f, center.y + s * 0.4f }, { center.x + s * 0.9f, center.y - s * 0.5f }, 3.0f, color);
            DrawLineEx({ center.x + s * 0.5f, center.y - s * 0.2f }, { center.x + s * 0.9f, center.y - s * 0.5f }, 2.5f, color);
            break;
        }
        case 3: case 30: { // Melee reduction / fortress
            DrawRectangleLines((int)(center.x - s * 0.75f), (int)(center.y - s * 0.55f), (int)(s * 1.5f), (int)(s * 1.1f), color);
            DrawLineEx({ center.x - s * 0.4f, center.y }, { center.x + s * 0.4f, center.y }, 2.0f, Fade(color, 0.7f));
            break;
        }
        case 4: case 13: case 21: { // Laser DR / reflect
            DrawCircleLinesV(center, s * 0.75f, color);
            DrawLineEx({ center.x - s * 0.5f, center.y - s * 0.5f }, { center.x + s * 0.5f, center.y + s * 0.5f }, 2.0f, color);
            break;
        }
        case 5: case 15: { // Cooldown
            DrawCircleLinesV(center, s * 0.75f, color);
            DrawLineEx({ center.x, center.y }, { center.x + s * 0.45f, center.y - s * 0.35f }, 2.5f, color);
            break;
        }
        case 6: { // Speed
            for (int i = 0; i < 3; i++) {
                float offset = (float)i * s * 0.35f;
                DrawLineEx({ center.x - s * 0.6f + offset, center.y - s * 0.5f }, { center.x - s * 0.2f + offset, center.y }, 2.5f, color);
                DrawLineEx({ center.x - s * 0.2f + offset, center.y }, { center.x - s * 0.6f + offset, center.y + s * 0.5f }, 2.5f, color);
            }
            break;
        }
        case 7: case 19: case 23: { // Scrap
            Vector2 shard[] = {
                { center.x, center.y - s * 0.85f }, { center.x + s * 0.7f, center.y - s * 0.1f },
                { center.x + s * 0.45f, center.y + s * 0.8f }, { center.x - s * 0.45f, center.y + s * 0.8f },
                { center.x - s * 0.7f, center.y - s * 0.1f }
            };
            for (int i = 0; i < 5; i++) DrawLineEx(shard[i], shard[(i + 1) % 5], 2.5f, color);
            break;
        }
        case 8: { // Pickup range
            DrawCircleLinesV(center, s * 0.7f, color);
            DrawCircleV(center, s * 0.15f, color);
            break;
        }
        case 28: { // Scrap magnate — scrap + magnet
            Vector2 shard[] = {
                { center.x, center.y - s * 0.5f }, { center.x + s * 0.45f, center.y },
                { center.x, center.y + s * 0.5f }, { center.x - s * 0.45f, center.y }
            };
            for (int i = 0; i < 4; i++) DrawLineEx(shard[i], shard[(i + 1) % 4], 2.5f, color);
            DrawCircleLinesV(center, s * 0.85f, Fade(color, 0.45f));
            break;
        }
        case 25: { // Ramming speed — ram + chevron
            DrawLineEx({ center.x - s * 0.7f, center.y + s * 0.3f }, { center.x + s * 0.8f, center.y - s * 0.4f }, 3.0f, color);
            DrawLineEx({ center.x + s * 0.2f, center.y }, { center.x + s * 0.7f, center.y - s * 0.35f }, 2.5f, color);
            break;
        }
        case 9: case 24: case 29: { // Max hull
            Vector2 shield[] = {
                { center.x, center.y - s }, { center.x + s * 0.85f, center.y - s * 0.35f },
                { center.x + s * 0.7f, center.y + s * 0.6f }, { center.x, center.y + s },
                { center.x - s * 0.7f, center.y + s * 0.6f }, { center.x - s * 0.85f, center.y - s * 0.35f }
            };
            for (int i = 0; i < 6; i++) DrawLineEx(shield[i], shield[(i + 1) % 6], 2.5f, color);
            break;
        }
        case 10: case 18: { // Tailshot
            DrawCircleLinesV(center, s * 0.65f, color);
            DrawLineEx({ center.x, center.y + s * 0.35f }, { center.x, center.y + s * 0.95f }, 2.5f, color);
            break;
        }
        case 11: case 22: { // Boost
            DrawLineEx({ center.x, center.y - s }, { center.x, center.y + s * 0.4f }, 3.0f, color);
            DrawLineEx({ center.x, center.y + s * 0.4f }, { center.x - s * 0.45f, center.y + s }, 2.5f, color);
            DrawLineEx({ center.x, center.y + s * 0.4f }, { center.x + s * 0.45f, center.y + s }, 2.5f, color);
            break;
        }
        case 12: case 16: { // Health pickup
            DrawLineEx({ center.x - s * 0.5f, center.y }, { center.x + s * 0.5f, center.y }, 2.5f, color);
            DrawLineEx({ center.x, center.y - s * 0.5f }, { center.x, center.y + s * 0.5f }, 2.5f, color);
            DrawCircleLinesV(center, s * 0.65f, Fade(color, 0.6f));
            break;
        }
        case 14: case 31: { // Passive turret
            DrawCircleV(center, s * 0.25f, color);
            for (int i = 0; i < 8; i++) {
                float angle = (float)i * PI / 4.0f;
                DrawLineEx(center, { center.x + cosf(angle) * s * 0.85f, center.y + sinf(angle) * s * 0.85f }, 1.5f, Fade(color, 0.5f));
            }
            break;
        }
        case 20: { // Boost recovery
            DrawCircleLinesV(center, s * 0.55f, color);
            DrawLineEx({ center.x + s * 0.2f, center.y - s * 0.2f }, { center.x - s * 0.35f, center.y + s * 0.35f }, 2.5f, color);
            break;
        }
        default:
            DrawCircleV(center, s * 0.4f, color);
            break;
    }
}

static UpgradeRarity RollDraftRarity() {
    int roll = GetRandomValue(1, 100);
    if (roll <= DRAFT_WEIGHT_COMMON) return UpgradeRarity::COMMON;
    roll -= DRAFT_WEIGHT_COMMON;
    if (roll <= DRAFT_WEIGHT_RARE) return UpgradeRarity::RARE;
    roll -= DRAFT_WEIGHT_RARE;
    if (roll <= DRAFT_WEIGHT_EPIC) return UpgradeRarity::EPIC;
    return UpgradeRarity::LEGENDARY;
}

static void FillDraftCards() {
    currentDraftRarity = RollDraftRarity();

    int eligible[UPGRADE_POOL_SIZE];
    int eligibleCount = 0;
    for (int i = 0; i < UPGRADE_POOL_SIZE; i++) {
        if (upgradePool[i].id == 0) continue;
        if (upgradePool[i].rarity != currentDraftRarity) continue;
        eligible[eligibleCount++] = i;
    }

    int picks = 0;
    while (picks < CARD_DRAFT_COUNT - 1 && eligibleCount > 0) {
        int pickIdx = GetRandomValue(0, eligibleCount - 1);
        int poolIdx = eligible[pickIdx];
        draftCards[picks] = {
            upgradePool[poolIdx].id,
            upgradePool[poolIdx].title,
            upgradePool[poolIdx].description,
            upgradePool[poolIdx].rarity
        };
        picks++;

        eligible[pickIdx] = eligible[eligibleCount - 1];
        eligibleCount--;
    }

    draftCards[CARD_DRAFT_COUNT - 1] = {
        upgradePool[0].id,
        upgradePool[0].title,
        upgradePool[0].description,
        upgradePool[0].rarity
    };
}

void InitCardDraft() {
    draftOpen = false;
    draftSelection = 0;
    nextDraftMilestone = SCRAP_PER_DRAFT;
}

void ResetCardDraftMilestone() {
    nextDraftMilestone = SCRAP_PER_DRAFT;
}

bool IsCardDraftOpen() {
    return draftOpen;
}

void CheckCardDraftTrigger() {
    if (draftOpen || game.gameOver) return;
    if (!enemiesOn || asteroidsMode) return;
    if (currentRun.scrap < nextDraftMilestone) return;

    FillDraftCards();
    draftOpen = true;
    draftSelection = 0;
    game.cardDraftOpen = true;
    PlaySfx(SfxId::CARD_DRAFT_OPEN);
}

static constexpr float DRAFT_CARD_W = 200.0f;
static constexpr float DRAFT_CARD_H = 180.0f;
static constexpr float DRAFT_CARD_GAP = 22.0f;
static constexpr float DRAFT_CARD_Y_OFFSET = 20.0f;

static Rectangle GetDraftCardRect(int index) {
    float totalW = CARD_DRAFT_COUNT * DRAFT_CARD_W + (CARD_DRAFT_COUNT - 1) * DRAFT_CARD_GAP;
    float startX = gameWidth / 2.0f - totalW / 2.0f;
    float cardY = gameHeight / 2.0f - DRAFT_CARD_H / 2.0f + DRAFT_CARD_Y_OFFSET;
    return { startX + index * (DRAFT_CARD_W + DRAFT_CARD_GAP), cardY, DRAFT_CARD_W, DRAFT_CARD_H };
}

static void ConfirmDraftSelection() {
    const DraftCard& card = draftCards[draftSelection];
    ApplyRunUpgrade(card.upgradeId, card.rarity);
    PlaySfx(SfxId::UPGRADE_SELECT);
    currentRun.scrap = 0;
    nextDraftMilestone += SCRAP_MILESTONE_STEP;
    TriggerEmpPulse(player.position);
    DestroyEnemyLasers();
    draftOpen = false;
    game.cardDraftOpen = false;
}

void UpdateCardDraft() {
    if (!draftOpen) return;

    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        draftSelection--;
        if (draftSelection < 0) draftSelection = CARD_DRAFT_COUNT - 1;
    }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        draftSelection++;
        if (draftSelection >= CARD_DRAFT_COUNT) draftSelection = 0;
    }
    for (int i = 0; i < CARD_DRAFT_COUNT; i++) {
        if (IsKeyPressed(KEY_ONE + i)) {
            draftSelection = i;
            ConfirmDraftSelection();
            return;
        }
    }
    if (IsKeyPressed(KEY_ENTER)) {
        ConfirmDraftSelection();
        return;
    }

    Vector2 mouse = GetMousePosition();
    for (int i = 0; i < CARD_DRAFT_COUNT; i++) {
        if (!CheckCollisionPointRec(mouse, GetDraftCardRect(i))) continue;
        draftSelection = i;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            ConfirmDraftSelection();
        }
        break;
    }
}

static void DrawDraftCard(const DraftCard& card, float x, float y, float w, float h, bool selected) {
    Color accent = GetUpgradeRarityColor(card.rarity);
    Color bg = selected ? (Color){ 45, 55, 85, 245 } : (Color){ 28, 30, 42, 235 };
    Color border = selected ? accent : (Color){ 80, 80, 100, 255 };
    Rectangle rect = { x, y, w, h };

    DrawRectangleRounded(rect, 0.06f, 8, bg);
    DrawRectangleRoundedLines(rect, 0.06f, 8, border);
    if (selected) {
        DrawRectangleRounded((Rectangle){ x + 4, y + 4, w - 8, h - 8 }, 0.05f, 6, Fade(accent, 0.12f));
    }

    const char* rarityName = GetUpgradeRarityName(card.rarity);
    int rarityFont = 12;
    DrawText(rarityName, (int)(x + 14), (int)(y + 10), rarityFont, accent);

    Vector2 iconCenter = { x + w / 2.0f, y + 72.0f };
    DrawUpgradeIcon(card.upgradeId, iconCenter, 48.0f, accent);

    DrawText(card.title, (int)(x + 14), (int)(y + 108), 16, WHITE);
    DrawTextEx(GetFontDefault(), card.description, (Vector2){ x + 14, y + 132 }, 13.0f, 1.3f, LIGHTGRAY);

    const char* warning = GetUpgradeTradeOffWarning(card.upgradeId);
    if (warning) {
        const Color warnColor = { 255, 80, 80, 255 };
        DrawTextEx(GetFontDefault(), warning, (Vector2){ x + 14, y + 152 }, 12.0f, 1.2f, warnColor);
    }
}

void DrawCardDraft() {
    if (!draftOpen) return;

    DrawRectangle(0, 0, gameWidth, gameHeight, (Color){ 0, 0, 0, 170 });

    Color draftColor = GetUpgradeRarityColor(currentDraftRarity);
    char title[64];
    snprintf(title, sizeof(title), "%s DRAFT", GetUpgradeRarityName(currentDraftRarity));
    DrawText(title, gameWidth / 2 - MeasureText(title, 28) / 2, 72, 28, draftColor);

    const char* subtitle = "Choose an upgrade";
    DrawText(subtitle, gameWidth / 2 - MeasureText(subtitle, 18) / 2, 108, 18, (Color){ 210, 170, 110, 255 });

    char milestone[64];
    snprintf(milestone, sizeof(milestone), "Next upgrade at %d scrap", nextDraftMilestone);
    DrawText(milestone, gameWidth / 2 - MeasureText(milestone, 16) / 2, 132, 16, GRAY);

    for (int i = 0; i < CARD_DRAFT_COUNT; i++) {
        Rectangle rect = GetDraftCardRect(i);
        DrawDraftCard(draftCards[i], rect.x, rect.y, rect.width, rect.height, i == draftSelection);
        char keyHint[4];
        snprintf(keyHint, sizeof(keyHint), "%d", i + 1);
        DrawText(keyHint, (int)(rect.x + rect.width - 24), (int)(rect.y + rect.height - 28), 16, DARKGRAY);
    }

    const char* hint = "Click a card   |   A / D or 1-4 to browse   |   ENTER to choose";
    Rectangle firstCard = GetDraftCardRect(0);
    DrawText(hint, gameWidth / 2 - MeasureText(hint, 16) / 2, (int)(firstCard.y + firstCard.height + 36), 16, WHITE);
}

void DrawActiveBuildSummary(int x, int y) {
    if (runMods.acquiredCount <= 0) return;

    const char* label = "Build:";
    DrawText(label, x, y + 6, 14, GRAY);

    float iconSize = 28.0f;
    float gap = 6.0f;
    float startX = x + MeasureText(label, 14) + 10.0f;

    for (int i = 0; i < runMods.acquiredCount; i++) {
        const AcquiredUpgrade& upgrade = runMods.acquired[i];
        Color rarityColor = GetUpgradeRarityColor(upgrade.rarity);
        float cx = startX + i * (iconSize + gap) + iconSize / 2.0f;
        float cy = y + iconSize / 2.0f + 2.0f;

        Rectangle badge = { cx - iconSize / 2.0f, cy - iconSize / 2.0f, iconSize, iconSize };
        DrawRectangleRounded(badge, 0.2f, 4, (Color){ 20, 22, 32, 200 });
        DrawRectangleRoundedLines(badge, 0.2f, 4, rarityColor);
        DrawUpgradeIcon(upgrade.upgradeId, { cx, cy }, iconSize * 0.7f, rarityColor);
    }
}
