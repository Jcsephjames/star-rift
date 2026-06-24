#pragma once
#ifndef RUN_MODIFIERS_HPP
#define RUN_MODIFIERS_HPP

#include "cardDraft.hpp"

#define MAX_ACQUIRED_UPGRADES 32

struct AcquiredUpgrade {
    int upgradeId;
    UpgradeRarity rarity;
};

struct RunModifiers {
    float laserDamageMult = 1.0f;
    float laserDamagePenaltyMult = 1.0f;
    float meleeDamageMult = 1.0f;
    float collisionDamageMult = 1.0f;
    float laserDamageReduction = 0.0f;
    float meleeDamageReduction = 0.0f;
    float meleeDamageTakenMult = 1.0f;
    float reflectChance = 0.0f;
    float cooldownMult = 1.0f;
    float speedMult = 1.0f;
    float tailshotMult = 3.0f;
    float scrapMult = 1.0f;
    float healthPickupMult = 1.0f;
    float pickupRadiusMult = 1.0f;
    float maxHealthMult = 1.0f;
    float maxHealthFlat = 0.0f;
    float maxBoostBonus = 0.0f;
    float boostRegenMult = 1.0f;
    float boostStallReduction = 0.0f;
    float passiveLaserInterval = 0.0f;
    float passiveLaserTimer = 0.0f;
    bool boostDisabled = false;
    int upgradeCount = 0;
    AcquiredUpgrade acquired[MAX_ACQUIRED_UPGRADES];
    int acquiredCount = 0;
};

extern RunModifiers runMods;

void InitRunModifiers();
void ApplyRunUpgrade(int upgradeId, UpgradeRarity rarity);
void RecalculatePlayerMaxHealth(bool healIncrease = true);
float GetEffectiveMaxHealth();
int GetEffectiveDamage();
float GetEffectiveLaserDamageMult();
float GetEffectiveShotCooldown();
float GetEffectiveSpeed();
float GetEffectiveLaserDamageTaken(float damage);
float GetEffectiveMeleeDamageTaken(float damage);
float GetEffectiveOutgoingMeleeDamage(float baseDamage);
float GetEffectiveReflectChance();
float GetEffectivePickupRadius(float baseRadius);
int GetBonusScrap(int baseAmount);
float GetEffectiveHealthPickupAmount(float baseAmount);
float GetEffectiveBoostStallDuration();
bool IsBoostDisabled();
void UpdatePassiveLaser(float deltaTime);

#endif
