#include "runModifiers.hpp"
#include "player.hpp"
#include "metaProgression.hpp"
#include "globals.hpp"
#include "laserManager.hpp"
#include "game.hpp"
#include <cmath>

extern Game game;

RunModifiers runMods;

static constexpr float kLaserDrCap = 0.50f;
static constexpr float kMeleeDrCap = 0.50f;
static constexpr float kReflectCap = 0.75f;
static constexpr float kCooldownFloor = 0.45f;
static constexpr float kLaserDamageFloor = 0.25f;
static constexpr float kBoostStallBase = 2.5f;

void InitRunModifiers() {
    runMods = {};
}

void RecalculatePlayerMaxHealth(bool healIncrease) {
    float oldMax = player.maxHealth;
    player.maxHealth = GetEffectiveMaxHealth();
    if (healIncrease && player.maxHealth > oldMax) {
        player.life += player.maxHealth - oldMax;
    }
    player.life = fminf(player.life, player.maxHealth);
}

float GetEffectiveMaxHealth() {
    float base = (100.0f + runMods.maxHealthFlat) * runMods.maxHealthMult;
    if (IsMetaOwned(META_STARTING_HULL)) {
        base += 5.0f;
    }
    return base;
}

float GetEffectiveLaserDamageMult() {
    return fmaxf(kLaserDamageFloor, runMods.laserDamageMult * runMods.laserDamagePenaltyMult);
}

int GetEffectiveDamage() {
    int damage = (int)(player.damage * GetEffectiveLaserDamageMult() + 0.5f);
    if (IsMetaOwned(META_OVERCHARGED_CELLS)) {
        damage += 1;
    }
    return damage;
}

float GetEffectiveShotCooldown() {
    return fmaxf(player.shotCooldown * runMods.cooldownMult, player.shotCooldown * kCooldownFloor);
}

float GetEffectiveSpeed() {
    return player.speed * runMods.speedMult;
}

float GetEffectiveLaserDamageTaken(float damage) {
    float reduction = fminf(runMods.laserDamageReduction, kLaserDrCap);
    return damage * (1.0f - reduction);
}

float GetEffectiveMeleeDamageTaken(float damage) {
    float reduction = fminf(runMods.meleeDamageReduction, kMeleeDrCap);
    return damage * (1.0f - reduction) * runMods.meleeDamageTakenMult;
}

float GetEffectiveOutgoingMeleeDamage(float baseDamage) {
    return baseDamage * runMods.meleeDamageMult * runMods.collisionDamageMult;
}

float GetEffectiveReflectChance() {
    return fminf(runMods.reflectChance, kReflectCap);
}

float GetEffectivePickupRadius(float baseRadius) {
    float mult = runMods.pickupRadiusMult;
    if (IsMetaOwned(META_MAGNETIC_SCOOP)) {
        mult *= 1.30f;
    }
    return baseRadius * mult;
}

int GetBonusScrap(int baseAmount) {
    return (int)(baseAmount * runMods.scrapMult + 0.5f);
}

float GetEffectiveHealthPickupAmount(float baseAmount) {
    return baseAmount * runMods.healthPickupMult;
}

float GetEffectiveBoostStallDuration() {
    return fmaxf(0.5f, kBoostStallBase - runMods.boostStallReduction);
}

bool IsBoostDisabled() {
    return runMods.boostDisabled;
}

void UpdatePassiveLaser(float deltaTime) {
    if (runMods.passiveLaserInterval <= 0.0f) return;
    if (game.cardDraftOpen || game.pauseMenuOpen || game.gameOver) return;

    runMods.passiveLaserTimer += deltaTime;
    if (runMods.passiveLaserTimer < runMods.passiveLaserInterval) return;

    runMods.passiveLaserTimer = 0.0f;
    float randomDir = (float)GetRandomValue(0, 359);
    int passiveDamage = (int)(GetEffectiveDamage() * 0.55f + 0.5f);
    if (passiveDamage < 1) passiveDamage = 1;

    FireSingleLaser(
        player.position,
        randomDir,
        player.shotSpeed,
        player.ship.laserSize,
        player.ship.laserOffsets,
        passiveDamage,
        false
    );
}

static void ApplyMaxBoostBonus(float amount) {
    runMods.maxBoostBonus += amount;
    player.ship.maxBoost += amount;
    if (player.ship.boostTimer > player.ship.maxBoost) {
        player.ship.boostTimer = player.ship.maxBoost;
    }
}

static void SetPassiveLaserInterval(float interval) {
    if (runMods.passiveLaserInterval <= 0.0f || interval < runMods.passiveLaserInterval) {
        runMods.passiveLaserInterval = interval;
    }
}

void ApplyRunUpgrade(int upgradeId, UpgradeRarity rarity) {
    if (runMods.acquiredCount < MAX_ACQUIRED_UPGRADES) {
        runMods.acquired[runMods.acquiredCount] = { upgradeId, rarity };
        runMods.acquiredCount++;
    }

    switch (upgradeId) {
        case 0: // Full Repair
            player.life = GetEffectiveMaxHealth();
            break;

        // --- Commons ---
        case 1: // Focused Optics
            runMods.laserDamageMult += 0.10f;
            break;
        case 2: // Kinetic Ram
            runMods.meleeDamageMult += 0.10f;
            break;
        case 3: // Hull Padding
            runMods.meleeDamageReduction = fminf(runMods.meleeDamageReduction + 0.10f, kMeleeDrCap);
            break;
        case 4: // Deflector Coating
            runMods.laserDamageReduction = fminf(runMods.laserDamageReduction + 0.10f, kLaserDrCap);
            break;
        case 5: // Quick Trigger
            runMods.cooldownMult *= 0.95f;
            break;
        case 6: // Thruster Wash
            runMods.speedMult += 0.10f;
            break;
        case 7: // Scrap Sifter
            runMods.scrapMult += 0.10f;
            break;
        case 8: // Magnetic Rig
            runMods.pickupRadiusMult += 0.50f;
            break;
        case 9: // Reinforced Frame
            runMods.maxHealthMult += 0.10f;
            RecalculatePlayerMaxHealth(true);
            break;
        case 10: // Tailshot Gyro
            runMods.tailshotMult += 0.5f;
            break;
        case 11: // Extended Boost Tanks
            ApplyMaxBoostBonus(20.0f);
            break;
        case 12: // Medkit Tuning
            runMods.healthPickupMult += 0.50f;
            break;

        // --- Rares ---
        case 13: // Mirror Plating
            runMods.reflectChance += 0.20f;
            break;
        case 14: // Erratic Turret
            SetPassiveLaserInterval(4.0f);
            break;
        case 15: // Rapid Cycle
            runMods.cooldownMult *= 0.90f;
            break;
        case 16: // Field Surgeon
            runMods.healthPickupMult += 1.00f;
            break;
        case 17: // Reinforced Ram
            runMods.meleeDamageMult += 0.20f;
            break;
        case 18: // Rear Gunner
            runMods.tailshotMult += 0.5f;
            break;
        case 19: // Salvage Tuning
            runMods.scrapMult += 0.25f;
            break;
        case 20: // Boost Recovery
            runMods.boostStallReduction += 0.625f;
            break;

        // --- Epics ---
        case 21: // Reflect Matrix
            runMods.reflectChance += 0.20f;
            break;
        case 22: // Overdrive Package
            runMods.boostRegenMult += 0.25f;
            runMods.boostStallReduction += 1.0f;
            break;
        case 23: // Scrap Hoarder
            runMods.scrapMult += 0.40f;
            break;
        case 24: // Bulkhead Upgrade
            runMods.maxHealthFlat += 20.0f;
            RecalculatePlayerMaxHealth(true);
            break;
        case 25: // Ramming Speed
            runMods.collisionDamageMult += 0.30f;
            runMods.speedMult += 0.10f;
            break;
        case 26: // Siege Lasers
            runMods.laserDamageMult += 0.30f;
            runMods.cooldownMult *= 0.90f;
            break;

        // --- Legendaries ---
        case 27: // Juggernaut Protocol
            runMods.collisionDamageMult += 1.00f;
            runMods.laserDamagePenaltyMult *= 0.50f;
            break;
        case 28: // Scrap Magnate
            runMods.scrapMult += 2.00f;
            runMods.pickupRadiusMult += 1.00f;
            break;
        case 29: // Fortress Hull
            runMods.maxHealthMult += 0.50f;
            runMods.boostDisabled = true;
            player.isBoosting = false;
            RecalculatePlayerMaxHealth(true);
            break;
        case 30: // Mirror Fortress
            runMods.reflectChance += 0.50f;
            runMods.meleeDamageTakenMult += 0.30f;
            break;
        case 31: // Storm Turret
            SetPassiveLaserInterval(1.0f);
            break;

        default:
            break;
    }

    if (upgradeId != 0) {
        runMods.upgradeCount++;
    }
}
