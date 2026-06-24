#include "shipSelection.hpp"
#include "textures.hpp"
#include "globals.hpp"
#include "unlocks.hpp"
#include "uiHelpers.hpp"
#include "metaProgression.hpp"
#include "audio.hpp"
#include "difficulty.hpp"
#include "raylib.h"
#include <cstdio>

ShipStats* ShipSelection::CreateShip(const char* name, Texture2D &texture, int shotPower, int shotCooldown, int speed, int boostMultiplier) {
    ShipStats* ship = new ShipStats;
    ship->name = name;
    ship->shotPower = shotPower;
    ship->shotCooldown = shotCooldown;
    ship->speed = speed;
    ship->boostMultiplier = boostMultiplier;
    ship->image = texture;
    return ship;
}

ShipSelection::ShipSelection() {
    shipSelection = 0;
    targetShipIndex = 0;
    difficultySelection = static_cast<int>(runDifficulty);
    shipSlideOffset = 0.0f;
    for (int i = 0; i < MAX_SHIPS; i++) ships[i] = nullptr;
}

ShipSelection::~ShipSelection() {
    for (int i = 0; i < MAX_SHIPS; i++) {
        if (ships[i]) {
            delete ships[i];
            ships[i] = nullptr;
        }
    }
}

void ShipSelection::SetInitialShip(int index) {
    if (index < 0 || index >= MAX_SHIPS) {
        index = 0;
    }
    shipSelection = index;
    targetShipIndex = index;
    shipSlideOffset = -(float)index * (float)GetScreenWidth();
    difficultySelection = 0;
    runDifficulty = RiftDifficulty::RIFT;
}

void ShipSelection::InitShips() {
    ships[0] = CreateShip("X-Wing", xWing, 6, 6, 6, 7);
    ships[1] = CreateShip("Y-Wing", yWing, 7, 5, 5, 6);
    ships[2] = CreateShip("TIE Fighter", tieFighter, 5, 8, 7, 8);
    ships[3] = CreateShip("TIE Bomber", tieBomber, 8, 5, 4, 6);
    ships[4] = CreateShip("Star Fighter", starWarsFighterOne, 8, 5, 5, 5);
    ships[5] = CreateShip("A-Wing", shiptwo, 5, 10, 7, 7);
    ships[6] = CreateShip("Death Fighter", deathFighter, 6, 9, 8, 9);
    ships[7] = CreateShip("Shlasher", shlasher, 6, 7, 6, 8);
    ships[8] = CreateShip("Falcon", falcon, 10, 6, 4, 10);
}

void ShipSelection::Update() {
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        targetShipIndex--;
        if (targetShipIndex < 0) targetShipIndex = MAX_SHIPS - 1;
        shipSelection = targetShipIndex;
        PlaySfx(SfxId::MENU_NAVIGATE);
    }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        targetShipIndex++;
        if (targetShipIndex >= MAX_SHIPS) targetShipIndex = 0;
        shipSelection = targetShipIndex;
        PlaySfx(SfxId::MENU_NAVIGATE);
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        if (difficultySelection < RIFT_DIFFICULTY_COUNT - 1) {
            difficultySelection++;
            runDifficulty = GetDifficulty();
            PlaySfx(SfxId::MENU_NAVIGATE);
        }
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        if (difficultySelection > 0) {
            difficultySelection--;
            runDifficulty = GetDifficulty();
            PlaySfx(SfxId::MENU_NAVIGATE);
        }
    }

    // Smooth slide
    float desiredOffset = -targetShipIndex * GetScreenWidth();
    shipSlideOffset += (desiredOffset - shipSlideOffset) * GetFrameTime() * slideSpeed;

    // Snap when close to desired offset
    if (fabs(shipSlideOffset - desiredOffset) < 1.0f) {
        shipSlideOffset = desiredOffset;
        shipSelection = targetShipIndex;
    }
}

const ShipStats* ShipSelection::GetShip(int index) const {
    if (index < 0 || index >= MAX_SHIPS) return nullptr;
    return ships[index];
}

bool ShipSelection::IsCurrentShipUnlocked() const {
    return IsShipUnlocked(shipSelection);
}

void ShipSelection::Draw() const {

    DrawTexture(selection, ((gameWidth/2 - collections.width/2)+35), 40, WHITE);
    DrawCreditsBadge(gameWidth - 180, 12, 18);

    const ShipStats* selected = ships[shipSelection];
    if (!selected) return;
#ifdef DEV_BUILD
    if (debugging) {
        printf("Selected ship index: %d, Ship name: %s\n", shipSelection, selected->name.c_str());
        printf("Texture width: %d, height: %d\n", selected->image.width, selected->image.height);
    }
#endif
    // Layout
    int cardCount = 4;
    int cardWidth = 120;
    int cardHeight = 70;
    int cardSpacing = 20;
    int totalWidth = cardCount * cardWidth + (cardCount - 1) * cardSpacing;
    int startX = GetScreenWidth()/2 - totalWidth/2;
    int cardsY = 150;

    DrawStatCard({ (float)startX, (float)cardsY, (float)cardWidth, (float)cardHeight }, "Shot Power", selected->shotPower, DARKGRAY);
    DrawStatCard({ (float)(startX + (cardWidth + cardSpacing)), (float)cardsY, (float)cardWidth, (float)cardHeight }, "Lazer-Rate", selected->shotCooldown, DARKGRAY);
    DrawStatCard({ (float)(startX + 2 * (cardWidth + cardSpacing)), (float)cardsY, (float)cardWidth, (float)cardHeight }, "Speed", selected->speed, DARKGRAY);
    DrawStatCard({ (float)(startX + 3 * (cardWidth + cardSpacing)), (float)cardsY, (float)cardWidth, (float)cardHeight }, "Boost", selected->boostMultiplier, DARKGRAY);

    RiftDifficulty difficulty = GetDifficulty();
    const char* diffLabel = GetDifficultyTag(difficulty)[0] == '\0'
        ? TextFormat("Difficulty: %s", GetDifficultyName(difficulty))
        : TextFormat("Difficulty: %s  (%s%s)", GetDifficultyName(difficulty), "Rift", GetDifficultyTag(difficulty));
    int diffY = GetScreenHeight() - 115;
    DrawText(diffLabel,
             GetScreenWidth() / 2 - MeasureText(diffLabel, 20) / 2,
             diffY, 20, LIGHTGRAY);
    DrawText("W / S — change difficulty",
             GetScreenWidth() / 2 - MeasureText("W / S — change difficulty", 14) / 2,
             diffY + 24, 14, DARKGRAY);

    for (int i = 0; i < MAX_SHIPS; i++) {
        if (!ships[i]) continue;

        float x = i * GetScreenWidth() + shipSlideOffset;

        int shipX = x + GetScreenWidth() / 2 - ships[i]->image.width / 2;
        int shipY = GetScreenHeight() / 2 - ships[i]->image.height / 2;
        bool unlocked = IsShipUnlocked(i);
        Color tint = unlocked ? WHITE : (Color){ 80, 80, 80, 180 };
        DrawTexture(ships[i]->image, shipX, shipY, tint);

        if (!unlocked) {
            DrawRectangle((float)shipX, (float)shipY,
                          (float)ships[i]->image.width, (float)ships[i]->image.height,
                          (Color){ 0, 0, 0, 120 });
        }

        Color nameColor = unlocked ? GREEN : GRAY;
        DrawText(ships[i]->name.c_str(),
                 shipX + ships[i]->image.width/2 - MeasureText(ships[i]->name.c_str(), 30)/2,
                 shipY + ships[i]->image.height + 20,
                 30, nameColor);
    }

    if (!IsCurrentShipUnlocked()) {
        const char* lockMsg = TextFormat("LOCKED — %s", GetUnlockRequirementText(shipSelection));
        DrawText(lockMsg,
                 GetScreenWidth() / 2 - MeasureText(lockMsg, 20) / 2,
                 GetScreenHeight() - 80, 20, RED);
        DrawText("Purchase ships with credits in Collections",
                 GetScreenWidth() / 2 - MeasureText("Purchase ships with credits in Collections", 16) / 2,
                 GetScreenHeight() - 55, 16, ORANGE);
    } else {
        DrawText("ENTER — launch",
                 GetScreenWidth() / 2 - MeasureText("ENTER — launch", 16) / 2,
                 GetScreenHeight() - 55, 16, GREEN);
    }
    // direction arrows
        // LEFT ARROW
        DrawTriangle(
            (Vector2){ 40.0f, (float)GetScreenHeight() / 2 },
            (Vector2){ 80.0f, (float)GetScreenHeight() / 2 + 40.0f },
            (Vector2){ 80.0f, (float)GetScreenHeight() / 2 - 40.0f },
            WHITE
        );
        // RIGHT ARROW
        DrawTriangle(
            (Vector2){ (float)GetScreenWidth() - 40.0f, (float)GetScreenHeight() / 2 },
            (Vector2){ (float)GetScreenWidth() - 80.0f, (float)GetScreenHeight() / 2 - 40.0f },
            (Vector2){ (float)GetScreenWidth() - 80.0f, (float)GetScreenHeight() / 2 + 40.0f },
            WHITE
        );
}
