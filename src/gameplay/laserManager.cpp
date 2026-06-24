#include "laserManager.hpp"
#include "globals.hpp"
#include "audio.hpp"

std::vector<Laser*> activeLasers;

void InitLasers() {
    activeLasers.clear();
}
void FireSingleLaser(Vector2 shipCenter, float directionDegrees, float shotSpeed, Vector2 laserSize, const std::vector<Vector2>& laserOffsets, int damage, bool fromEnemy) {
    float angleRad = (directionDegrees - 90.0f) * DEG2RAD;
    Vector2 dir = { cosf(angleRad), sinf(angleRad) };

    for (Vector2 offset : laserOffsets) {
        float rotatedX = offset.x * cosf(angleRad) - offset.y * sinf(angleRad);
        float rotatedY = offset.x * sinf(angleRad) + offset.y * cosf(angleRad);
        Vector2 rotatedOffset = { rotatedX, rotatedY };

        Vector2 spawnPos = {
            shipCenter.x + rotatedOffset.x,
            shipCenter.y + rotatedOffset.y
        };
        activeLasers.push_back(new Laser(spawnPos, dir, shotSpeed, laserSize, damage, fromEnemy));
    }

    PlaySfx(fromEnemy ? SfxId::ENEMY_LASER_FIRE : SfxId::LASER_FIRE);
}
void UpdateLasers(float deltaTime) {
    for (int i = 0; i < activeLasers.size(); ) {
        Laser* laser = activeLasers[i];
        laser->Update(deltaTime);

        if (laser->IsOffScreen()) {
            delete laser;
            activeLasers.erase(activeLasers.begin() + i);
        } else {
            ++i;
        }
    }
}

void DrawLasers() {
    for (Laser* laser : activeLasers) {
        laser->Draw();

        if (debugging) {
            DrawCircleLines((int)laser->position.x, (int)laser->position.y, 2, RED);
        }
    }

    if (debugging) {
        DrawText(TextFormat("Active Lasers: %d", activeLasers.size()), gameWidth - MeasureText("Active Lasers: 10", 20), 40, 20, GREEN);
    }
}

void DestroyEnemyLasers() {
    for (int i = 0; i < (int)activeLasers.size(); ) {
        if (activeLasers[i]->fromEnemy) {
            delete activeLasers[i];
            activeLasers.erase(activeLasers.begin() + i);
        } else {
            ++i;
        }
    }
}
