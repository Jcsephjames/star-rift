#include <raylib.h>
#include <vector>
#include <math.h>
#include <stdio.h>

#include "player.hpp"
#include "laser.hpp"
#include "textures.hpp"
#include "globals.hpp"
#include "ship.hpp"
#include "laserManager.hpp"
#include "runModifiers.hpp"
#include "metaProgression.hpp"
#include "engineExhaust.hpp"

Player::Player() {

}
void Player::InitPlayer()
{
    position = {500.0f, 400.0f};
    direction = 0.0f;
    player.maxHealth = GetEffectiveMaxHealth();
    life = player.maxHealth;

    if (starshipType < 0 || starshipType >= MAX_SHIPS) {
        starshipType = 0;
    }


    ship.boostTimer = ship.maxBoost;
    ship.boostStaller = false;
    ship.boostResetTimer = 0.0f;
    timeSinceLastShot = 0.0f;

    // Init Lasers
    InitLasers();
}
void Player::playerRotation(float deltaTime)
{
    const float rotationSpeed = 400.0f; // degrees per second
    const float lightrotation = 175.0f; // degrees per second for light rotation
    float prevDirection = direction;

        // Heavy Rotate
        if (IsKeyDown(KEY_Q)) {
            direction -= rotationSpeed * deltaTime; // Rotate left
        }
        if (IsKeyDown(KEY_E)) {
            direction += rotationSpeed * deltaTime; // Rotate right
        }
        // Light Rotate
        if (IsKeyDown(KEY_A)){
            direction -= lightrotation * deltaTime; // light Rotate left
        }
        if (IsKeyDown(KEY_D)){
            direction += lightrotation * deltaTime; // light Rotate right
        }
    
        // Keep direction in [0, 360)
        if (direction >= 360.0f) direction -= 360.0f;
        else if (direction < 0.0f) direction += 360.0f;

        float delta = direction - prevDirection;
        if (delta > 180.0f) delta -= 360.0f;
        if (delta < -180.0f) delta += 360.0f;
        rotationRate = delta / deltaTime;
    
}
void Player::Update(float deltaTime)
{
    playerRotation(deltaTime);
    ship.ApplyControlMechanics(direction);

    isBraking = IsKeyDown(KEY_S);
    isBoosting = !IsBoostDisabled() && IsKeyDown(KEY_LEFT_SHIFT) && ship.boostTimer > 0.0f && !ship.boostStaller;
    ship.boostLogic(isBraking, deltaTime);
    timeSinceLastShot += deltaTime;
    const float moveSpeed = GetEffectiveSpeed();
    const float brakeFactor = 0.5f; // Hold S to move at 50% speed
    float speedMultiplier = isBraking ? brakeFactor : 1.0f;
    float effectiveCooldown = GetEffectiveShotCooldown();

    float rad = (direction - 90.0f) * (PI / 180.0f);
    if ((position.y < 0.0 + 30) && (!yWrapping)) {
        position.y = 0.0 + 30;
    }

    if (isBraking && ship.boostTimer > 0.0f && !ship.boostStaller && !IsBoostDisabled()) {
        ship.boostTimer -= 60.0f * deltaTime;
    }

    if (isBoosting) {
        ship.boostTimer -= 120.0f * deltaTime;
        position.x += cosf(rad) * moveSpeed * boost * speedMultiplier * deltaTime;
        position.y += sinf(rad) * moveSpeed * boost * speedMultiplier * deltaTime;
    } else {
        position.x += cosf(rad) * moveSpeed * speedMultiplier * deltaTime;
        position.y += sinf(rad) * moveSpeed * speedMultiplier * deltaTime;
    }

    if (!isBraking) {
        ship.TryAutoFire(timeSinceLastShot, effectiveCooldown, position, direction, shotSpeed, GetEffectiveDamage());
    }

    UpdateLasers(deltaTime);
    UpdatePassiveLaser(deltaTime);

    float intensity = isBraking ? 0.35f : 0.85f;
    float boostFactor = isBoosting ? 1.75f : 1.0f;
    EngineExhaustParams exhaust = {
        .position = position,
        .direction = direction,
        .scale = scale,
        .intensity = intensity,
        .boostFactor = boostFactor,
        .turnRate = rotationRate,
        .neonColors = IsMetaEnabled(META_NEON_TRAIL) && !IsMetaEnabled(META_ION_TRAIL),
        .ionColors = IsMetaEnabled(META_ION_TRAIL),
        .time = (float)GetTime()
    };
    EmitEngineExhaust(exhaust);

    float halfWidth = (texture.width * scale) / 2.0f;
    if (position.x < halfWidth) {
        exhaust.position = { position.x + gameWidth, position.y };
        EmitEngineExhaust(exhaust);
    } else if (position.x > gameWidth - halfWidth) {
        exhaust.position = { position.x - gameWidth, position.y };
        EmitEngineExhaust(exhaust);
    }
}

void Player::Draw(float gameWidth, float gameHeight) {
    Rectangle sourceRec = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
    Rectangle destRec = {
        position.x,
        position.y,
        texture.width * scale,
        texture.height * scale
    };
    Vector2 origin = { (texture.width * scale) / 2.0f, (texture.height * scale) / 2.0f };

    Color shipTint = WHITE;
    if (IsMetaEnabled(META_GOLD_HULL)) {
        shipTint = (Color){ 255, 215, 80, 255 };
    } else if (IsMetaEnabled(META_CRIMSON_HULL)) {
        shipTint = (Color){ 255, 90, 90, 255 };
    }

    DrawTexturePro(texture, sourceRec, destRec, origin, direction, shipTint);
    //debugging
    if (debugging) {
        DrawText(TextFormat("Position: (%.2f, %.2f)", position.x, position.y), 10, 20, 20, WHITE);
        DrawText(TextFormat("Direction: %.2f", direction), 10, 40, 20, WHITE);
        DrawText(TextFormat("Speed: %.2f", speed), 10, 60, 20, WHITE);
        DrawCircle((int)position.x, (int)position.y, 4, RED);
        DrawRectangleLines(
            position.x - origin.x,
            position.y - origin.y,
            destRec.width,
            destRec.height,
            GREEN
        );
    }
    //               WRAPPING DRAW LOGIC
    float halfWidth = (texture.width * scale) / 2.0f;
    // If close to left edge, draw ghost on right side
    if (position.x < halfWidth) {
        Rectangle ghostDestRec = destRec;
        ghostDestRec.x += gameWidth;
        DrawTexturePro(texture, sourceRec, ghostDestRec, origin, direction, shipTint);
    }
    // If close to right edge, draw ghost on left side
    else if (position.x > gameWidth - halfWidth) {
        Rectangle ghostDestRec = destRec;
        ghostDestRec.x -= gameWidth;
        DrawTexturePro(texture, sourceRec, ghostDestRec, origin, direction, shipTint);
    }
    if (bellowY) {
        life -= 0.1f; // Decrease life if below Y threshold

        // Clamp arrow X so it stays inside screen bounds
        float arrowX = position.x;
        if (arrowX < 20) arrowX = 20;
        else if (arrowX > gameWidth - 20) arrowX = gameWidth - 20;
    
        Vector2 arrowPos = { arrowX, gameHeight - 30.0f };
    
        // Triangle points relative to arrowPos, pointing upwards by default
        Vector2 point1 = { 0, -27 };  // Top tip of arrow
        Vector2 point2 = { -10, 0 };  // Bottom left
        Vector2 point3 = { 10, 0 };   // Bottom right
    
        // Rotate points by player's direction (in degrees)
        float rad = (direction) * (PI / 180.0f);
    
        // Function to rotate a point around origin (0,0)
        auto RotatePoint = [rad](Vector2 p) {
            return Vector2{
                p.x * cosf(rad) - p.y * sinf(rad),
                p.x * sinf(rad) + p.y * cosf(rad)
            };
        };
    
        Vector2 rPoint1 = RotatePoint(point1);
        Vector2 rPoint2 = RotatePoint(point2);
        Vector2 rPoint3 = RotatePoint(point3);
    
        // Translate rotated points to arrowPos on screen
        rPoint1.x += arrowPos.x;
        rPoint1.y += arrowPos.y;
    
        rPoint2.x += arrowPos.x;
        rPoint2.y += arrowPos.y;
    
        rPoint3.x += arrowPos.x;
        rPoint3.y += arrowPos.y;
    
        DrawTriangle(rPoint1, rPoint2, rPoint3, RED);
    }
}
void Player::SetStarshipType(int type)
{
    if (type < 0 || type >= MAX_SHIPS) {
        type = 0;
    }
    starshipType = type;
    ::starshipType = type;

    switch(type) {
        case 0: ship = XWingShip; break;
        case 1: ship = YWingShip; break;
        case 2: ship = TieFighterShip; break;
        case 3: ship = TieBomberShip; break;
        case 4: ship = StarWarsFighterOneShip; break;
        case 5: ship = ShipTwoShip; break;
        case 6: ship = DeathFighterShip; break;
        case 7: ship = ShlasherShip; break;
        case 8: ship = FalconShip; break;
        default: ship = XWingShip; break;
    }

    texture = ship.texture;
    scale = ship.scale;
    speed = ship.speed;
    boost = ship.boost;
    damage = ship.damage;
    shotSpeed = ship.shotSpeed;
    shotCooldown = ship.shotCooldown;

    ship.boostTimer = ship.maxBoost;
    ship.boostStaller = false;
    ship.boostResetTimer = 0.0f;
}
