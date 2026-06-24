#include "unlocks.hpp"
#include "globals.hpp"
#include "saveData.hpp"
#include "metaProgression.hpp"
#include <cstdio>

const UnlockRequirement shipUnlockRequirements[MAX_SHIPS] = {
    { 0     },  // 0 X-Wing — free starter (Rebellion grunt)
    { 3000  },  // 1 Y-Wing — pairs with TIE Bomber
    { 0     },  // 2 TIE Fighter — free starter (Empire grunt)
    { 3000  },  // 3 TIE Bomber — pairs with Y-Wing
    { 6000  },  // 4 Star Fighter — elite tier
    { 6000  },  // 5 Ship Two — elite tier
    { 6000  },  // 6 Death Fighter — elite tier
    { 10000 },  // 7 Shlasher — boss tier
    { 10000 },  // 8 Falcon — boss tier
};

void InitDefaultUnlocks() {
    for (int i = 0; i < MAX_SHIPS; i++) {
        if (shipUnlockRequirements[i].creditCost == 0) {
            shipUnlocked[i] = true;
        }
    }
}

bool IsShipUnlocked(int shipIndex) {
    if (shipIndex < 0 || shipIndex >= MAX_SHIPS) return false;
    return shipUnlocked[shipIndex];
}

int GetShipCreditCost(int shipIndex) {
    if (shipIndex < 0 || shipIndex >= MAX_SHIPS) return 0;
    return shipUnlockRequirements[shipIndex].creditCost;
}

const char* GetUnlockRequirementText(int shipIndex) {
    if (shipIndex < 0 || shipIndex >= MAX_SHIPS) return "Unknown";
    if (shipUnlockRequirements[shipIndex].creditCost == 0) {
        return "Starter Ship";
    }
    static char buffer[64];
    snprintf(buffer, sizeof(buffer), "%d Credits (Collections)",
             shipUnlockRequirements[shipIndex].creditCost);
    return buffer;
}

bool TryPurchaseShip(int shipIndex) {
    if (shipIndex < 0 || shipIndex >= MAX_SHIPS) return false;
    if (shipUnlocked[shipIndex]) return false;

    int cost = shipUnlockRequirements[shipIndex].creditCost;
    if (cost <= 0) return false;
    if (playerCredits < cost) return false;

    playerCredits -= cost;
    shipUnlocked[shipIndex] = true;
    SaveGameData();

    if (debugging) {
        printf("Purchased ship %d for %d credits\n", shipIndex, cost);
    }
    return true;
}

int CountUnlockedShips() {
    int count = 0;
    for (int i = 0; i < MAX_SHIPS; i++) {
        if (shipUnlocked[i]) count++;
    }
    return count;
}
