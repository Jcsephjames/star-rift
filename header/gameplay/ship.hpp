#ifndef SHIP_HPP
#define SHIP_HPP

#include <string>
#include <vector>
#include <raylib.h>

enum class ShipAllegiance {
    REBELLION,
    EMPIRE,
    NEUTRAL
};

class Ship {
public:
    std::string name;
    ShipAllegiance allegiance = ShipAllegiance::NEUTRAL;
    Texture2D texture;
    float scale;
    float speed;
    float boost;
    int damage;
    float shotSpeed;
    float shotCooldown;
    float maxBoost;

    float boostTimer; // Amount of boost available
    float unlimitedBoost = 999999.9f;

    bool boostStaller = false;
    float boostResetTimer = 0.0f;

    Vector2 laserSize;
    std::vector<Vector2> laserOffsets;

    float playerCollisionReductionArc = 0.0f;
    float enemyCollisionReductionArc = 0.0f;
    bool playerCollisionReductionFromFront = true;
    bool enemyCollisionReductionFromFront = true;
    // Physical collisions only — laser damage is never reduced by these values.
    float playerCollisionDamageReduction = 0.0f;
    float enemyCollisionDamageReduction = 0.0f;
    bool flipOnControl = false;
    bool hasLasers = true;
    float meleeDamageMultiplier = 1.0f;

    Ship() = default;

    Ship(const std::string& name, ShipAllegiance allegiance, Texture2D texture, float scale, float speed, float boost,
         int damage, float shotSpeed, float shotCooldown,
         Vector2 laserSize, std::vector<Vector2> laserOffsets, float maxBoost,
         float playerCollisionReductionArc = 0.0f, float enemyCollisionReductionArc = 0.0f,
         bool playerCollisionReductionFromFront = true, bool enemyCollisionReductionFromFront = true,
         float playerCollisionDamageReduction = 0.0f, float enemyCollisionDamageReduction = 0.0f,
         bool flipOnControl = false, bool hasLasers = true, float meleeDamageMultiplier = 1.0f);

    static ShipAllegiance GetAllegianceForType(int shipType);

    float GetPlayerCollisionDamageMultiplier(float facingDirection, Vector2 targetPos, Vector2 hitterPos) const;
    float GetEnemyCollisionDamageMultiplier(float facingDirection, Vector2 targetPos, Vector2 hitterPos) const;
    float ComputeMeleeDamage(float approachSpeed, float tierMultiplier = 1.0f) const;
    void ApplyControlMechanics(float& direction) const;
    void TryAutoFire(float& timeSinceLastShot, float effectiveCooldown, Vector2 position, float direction,
                     float shotSpeed, int damage) const;
    
    void boostLogic(bool isBraking, float deltaTime);
};

extern Ship XWingShip;
extern Ship TieFighterShip;
extern Ship StarWarsFighterOneShip;
extern Ship FalconShip;
extern Ship YWingShip;
extern Ship DeathFighterShip;
extern Ship ShlasherShip;
extern Ship TieBomberShip;
extern Ship ShipTwoShip;
extern Ship DeathStarShip;

void InitShips();

#endif // SHIP_HPP
