#pragma once
#ifndef UNLOCKS_HPP
#define UNLOCKS_HPP

#include "shipSelection.hpp"

struct UnlockRequirement {
    int creditCost;
};

extern const UnlockRequirement shipUnlockRequirements[MAX_SHIPS];

void InitDefaultUnlocks();
bool IsShipUnlocked(int shipIndex);
int GetShipCreditCost(int shipIndex);
const char* GetUnlockRequirementText(int shipIndex);
bool TryPurchaseShip(int shipIndex);
int CountUnlockedShips();

#endif
