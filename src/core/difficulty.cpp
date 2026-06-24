#include "difficulty.hpp"
#include "globals.hpp"

RiftDifficulty runDifficulty = RiftDifficulty::RIFT;

const char* GetDifficultyName(RiftDifficulty difficulty) {
    switch (difficulty) {
        case RiftDifficulty::RIFT: return "Rift";
        case RiftDifficulty::SURGE: return "Surge";
        case RiftDifficulty::COLLAPSE: return "Collapse";
        default: return "Rift";
    }
}

const char* GetDifficultyTag(RiftDifficulty difficulty) {
    switch (difficulty) {
        case RiftDifficulty::RIFT: return "";
        case RiftDifficulty::SURGE: return "+";
        case RiftDifficulty::COLLAPSE: return "++";
        default: return "";
    }
}

float GetDifficultySpawnMultiplier(RiftDifficulty difficulty) {
    switch (difficulty) {
        case RiftDifficulty::RIFT: return 1.0f;
        case RiftDifficulty::SURGE: return 1.4f;
        case RiftDifficulty::COLLAPSE: return 1.8f;
        default: return 1.0f;
    }
}

int GetDifficultyEnemyCapBonus(RiftDifficulty difficulty) {
    switch (difficulty) {
        case RiftDifficulty::RIFT: return 0;
        case RiftDifficulty::SURGE: return 1;
        case RiftDifficulty::COLLAPSE: return 2;
        default: return 0;
    }
}

void ApplyDifficultySpawnSettings() {
    float mult = GetDifficultySpawnMultiplier();
    if (asteroidsMode) {
        asteroidSpawnInterval = 3.0f / mult;
    } else {
        asteroidSpawnInterval = 7.0f / mult;
        enemySpawnInterval = 7.0f / mult;
    }
}
