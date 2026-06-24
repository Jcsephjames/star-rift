#pragma once
#ifndef DIFFICULTY_HPP
#define DIFFICULTY_HPP

enum class RiftDifficulty {
    RIFT = 0,
    SURGE = 1,
    COLLAPSE = 2
};

constexpr int RIFT_DIFFICULTY_COUNT = 3;

extern RiftDifficulty runDifficulty;

const char* GetDifficultyName(RiftDifficulty difficulty);
const char* GetDifficultyTag(RiftDifficulty difficulty);
float GetDifficultySpawnMultiplier(RiftDifficulty difficulty = runDifficulty);
int GetDifficultyEnemyCapBonus(RiftDifficulty difficulty = runDifficulty);
void ApplyDifficultySpawnSettings();

#endif
