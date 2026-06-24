#include "stdio.h"
#include "ship.hpp"
#include "textures.hpp"
#include "globals.hpp"
#include "runModifiers.hpp"
#include "laserManager.hpp"
#include <cmath>
#include <raymath.h>

bool debugMessage = false; // Global variable to control debug message output

// Constructor definition
Ship::Ship(const std::string& name, ShipAllegiance allegiance, Texture2D texture, float scale, float speed, float boost,
           int damage, float shotSpeed, float shotCooldown,
           Vector2 laserSize, std::vector<Vector2> laserOffsets, float maxBoost,
           float playerCollisionReductionArc, float enemyCollisionReductionArc,
           bool playerCollisionReductionFromFront, bool enemyCollisionReductionFromFront,
           float playerCollisionDamageReduction, float enemyCollisionDamageReduction,
           bool flipOnControl, bool hasLasers, float meleeDamageMultiplier)
    : name(name), allegiance(allegiance), texture(texture), scale(scale), speed(speed), boost(boost),
      damage(damage), shotSpeed(shotSpeed), shotCooldown(shotCooldown),
      laserSize(laserSize), laserOffsets(laserOffsets), maxBoost(maxBoost),
      playerCollisionReductionArc(playerCollisionReductionArc),
      enemyCollisionReductionArc(enemyCollisionReductionArc),
      playerCollisionReductionFromFront(playerCollisionReductionFromFront),
      enemyCollisionReductionFromFront(enemyCollisionReductionFromFront),
      playerCollisionDamageReduction(playerCollisionDamageReduction),
      enemyCollisionDamageReduction(enemyCollisionDamageReduction),
      flipOnControl(flipOnControl),
      hasLasers(hasLasers),
      meleeDamageMultiplier(meleeDamageMultiplier),
      boostTimer(maxBoost) {}

static bool IsInProtectedArc(float arc, bool fromFront, float facingDirection, Vector2 targetPos, Vector2 hitterPos) {
    if (arc <= 0.0f) return false;
    if (arc >= 360.0f) return true;

    float rad = (facingDirection - 90.0f) * (PI / 180.0f);
    Vector2 forward = { cosf(rad), sinf(rad) };
    Vector2 hitDir = Vector2Normalize(Vector2Subtract(hitterPos, targetPos));
    float dot = Vector2DotProduct(forward, hitDir);

    if (fromFront) {
        float halfArc = arc * 0.5f;
        return dot >= cosf(halfArc * DEG2RAD);
    }

    float vulnerableHalfArc = (360.0f - arc) * 0.5f;
    return dot <= cosf(vulnerableHalfArc * DEG2RAD);
}

static float GetCollisionDamageMultiplier(float reductionArc, bool fromFront, float damageReduction,
                                          float facingDirection, Vector2 targetPos, Vector2 hitterPos) {
    if (damageReduction <= 0.0f) return 1.0f;
    if (!IsInProtectedArc(reductionArc, fromFront, facingDirection, targetPos, hitterPos)) return 1.0f;
    if (damageReduction >= 1.0f) return 0.0f;
    return 1.0f - damageReduction;
}

float Ship::GetPlayerCollisionDamageMultiplier(float facingDirection, Vector2 targetPos, Vector2 hitterPos) const {
    return GetCollisionDamageMultiplier(
        playerCollisionReductionArc, playerCollisionReductionFromFront, playerCollisionDamageReduction,
        facingDirection, targetPos, hitterPos);
}

float Ship::GetEnemyCollisionDamageMultiplier(float facingDirection, Vector2 targetPos, Vector2 hitterPos) const {
    return GetCollisionDamageMultiplier(
        enemyCollisionReductionArc, enemyCollisionReductionFromFront, enemyCollisionDamageReduction,
        facingDirection, targetPos, hitterPos);
}

float Ship::ComputeMeleeDamage(float approachSpeed, float tierMultiplier) const {
    float base = (8.0f + approachSpeed * 0.09f + (float)damage * 0.38f) * tierMultiplier;
    return base * meleeDamageMultiplier;
}

void Ship::TryAutoFire(float& timeSinceLastShot, float effectiveCooldown, Vector2 position, float direction,
                       float shotSpeed, int damage) const {
    if (!hasLasers) return;
    if (timeSinceLastShot < effectiveCooldown) return;

    timeSinceLastShot = 0.0f;
    FireSingleLaser(position, direction, shotSpeed, laserSize, laserOffsets, damage);
}

void Ship::ApplyControlMechanics(float& direction) const {
    if (!flipOnControl) return;
    if (!IsKeyPressed(KEY_LEFT_CONTROL) && !IsKeyPressed(KEY_RIGHT_CONTROL)) return;

    direction += 180.0f;
    if (direction >= 360.0f) direction -= 360.0f;
    else if (direction < 0.0f) direction += 360.0f;
}

ShipAllegiance Ship::GetAllegianceForType(int shipType) {
    switch (shipType) {
        case 0: case 1: case 4: case 5: case 8: return ShipAllegiance::REBELLION;
        case 2: case 3: case 6: case 7: return ShipAllegiance::EMPIRE;
        default: return ShipAllegiance::NEUTRAL;
    }
}

void Ship::boostLogic(bool isBraking, float deltaTime)
{
    if (IsBoostDisabled()) {
        boostStaller = false;
        boostResetTimer = 0.0f;
        boostTimer = 0.0f;
        return;
    }

    if (boostTimer < 1.0f) {
        boostStaller = true;
    }
    if (boostStaller) {
        if ((debugging) && (!debugMessage)) {
            printf("Boost staller active, waiting for boost to reset.\n");
            debugMessage = true;
        }
        boostResetTimer += deltaTime;
        if (boostResetTimer >= GetEffectiveBoostStallDuration()) {
            boostStaller = false;
            boostResetTimer = 0.0f;
            debugMessage = false;
        }
    }
    if (boostTimer >= maxBoost) {
        boostTimer = maxBoost;
    } else if (!IsKeyDown(KEY_LEFT_SHIFT) && !boostStaller && !isBraking) {
        boostTimer += 300.0f * runMods.boostRegenMult * deltaTime;
    }
}

// Globals
Ship XWingShip;
Ship YWingShip;
Ship TieFighterShip;
Ship DeathFighterShip;
Ship StarWarsFighterOneShip;
Ship ShlasherShip;
Ship FalconShip;
Ship TieBomberShip;
Ship ShipTwoShip;
Ship DeathStarShip;

// name(name), texture(texture), scale(scale), speed(speed), boost(boost),
//damage(damage), shotSpeed(shotSpeed), shotCooldown(shotCooldown),
//laserSize(laserSize), laserOffsets(laserOffsets), maxBoost(maxBoost)

void InitShips() {
    XWingShip = Ship("X-Wing", ShipAllegiance::REBELLION, xWing, 1.0f, 200.0f, 1.4f, 10, 600.0f, 0.4f,
        {4, 10}, { {-10 , 10}, {-10 , -10} }, 250.0f);

    YWingShip = Ship("Y-Wing", ShipAllegiance::REBELLION, yWing, 1.0f, 180.0f, 1.2f, 12, 500.0f, 0.5f,
        {3, 12}, { {-10 , 6}, {-10 , -6} }, 300.0f);

    TieFighterShip = Ship("TIE Fighter", ShipAllegiance::EMPIRE, tieFighter, 1.0f, 230.0f, 1.6f, 8, 750.0f, 0.25f,
        {3, 15}, { {-10 , 2 }, { -10 , -2 } }, 375.0f);

    DeathFighterShip = Ship("Death Fighter", ShipAllegiance::EMPIRE, deathFighter, 1.0f, 250.0f, 1.8f, 10, 800.0f, 0.225f,
        {5, 20}, { {-10 , 2 }, { -10 , -2 } , { 0, 0 }}, 500.0f);

    TieBomberShip = Ship("TIE Bomber", ShipAllegiance::EMPIRE, tieBomber, 1.0f, 175.0f, 1.3f, 14, 550.0f, 0.45f,
        {5, 14}, { {-10 , 8 }, { -10 , -8 } }, 350.0f);

    StarWarsFighterOneShip = Ship("Star Fighter", ShipAllegiance::REBELLION, starWarsFighterOne, 1.0f, 180.0f, 1.2f, 16, 600.0f, 0.5f,
        {4, 8}, { { -5 , 4 }, { -5 , -4 } }, 225.0f);
    
    ShlasherShip = Ship("Shlasher", ShipAllegiance::EMPIRE, shlasher, 1.0f, 210.0f, 1.5f, 10, 700.0f, 0.3f,
        {4, 12}, std::vector<Vector2>{ { -5, 0 } }, 400.0f, 180.0f, 180.0f, true, true, 0.8f, 0.8f,
        false, false, 1.35f);

    FalconShip = Ship("Falcon", ShipAllegiance::REBELLION, falcon, 1.0f, 160.0f, 2.2f, 20, 850.0f, 0.4f,
        {5, 20}, std::vector<Vector2>{ {5, 10} }, 99999.9f );

    ShipTwoShip = Ship("A-Wing", ShipAllegiance::REBELLION, shiptwo, 1.0f, 215.0f, 1.5f, 7, 720.0f, 0.12f,
        {3, 9}, { { -8 , 3 }, { -8 , -3 } }, 325.0f, 0.0f, 0.0f, true, true, 0.0f, 0.0f, true);

    DeathStarShip = Ship("Death Star", ShipAllegiance::EMPIRE, deathStar, 2.6f, 25.0f, 1.0f, 18, 500.0f, 4.5f,
        {8, 28}, { { 0, 35 }, { -22, 20 }, { 22, 20 } }, 99999.9f, 360.0f, 360.0f, true, true, 0.85f, 0.85f);
}
