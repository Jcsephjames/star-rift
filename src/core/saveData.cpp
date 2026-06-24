#include "saveData.hpp"
#include "globals.hpp"
#include "unlocks.hpp"
#include "player.hpp"
#include "metaProgression.hpp"
#include "runHistory.hpp"
#include "difficulty.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

static const char* SAVE_FILE = "starrift_save.txt";
static int shipLayoutVersion = 0;

static int UnlockedShipsBitmask() {
    int mask = 0;
    for (int i = 0; i < MAX_SHIPS; i++) {
        if (shipUnlocked[i]) {
            mask |= (1 << i);
        }
    }
    return mask;
}

static void ApplyUnlockedBitmask(int mask) {
    for (int i = 0; i < MAX_SHIPS; i++) {
        shipUnlocked[i] = (mask & (1 << i)) != 0;
    }
}

static void MigrateShipLayoutV1() {
    if (shipLayoutVersion >= 1) return;

    bool unlocked[4];
    int scores[4];
    float times[4];
    int kills[4];

    for (int i = 0; i < 4; i++) {
        unlocked[i] = shipUnlocked[5 + i];
        scores[i] = shipHighScores[5 + i];
        times[i] = shipPlayTime[5 + i];
        kills[i] = shipKills[5 + i];
    }

    // Old: 5 Death Fighter, 6 Slasher, 7 Falcon, 8 Ship Two
    shipUnlocked[5] = unlocked[3];
    shipUnlocked[6] = unlocked[0];
    shipUnlocked[7] = unlocked[1];
    shipUnlocked[8] = unlocked[2];

    shipHighScores[5] = scores[3];
    shipHighScores[6] = scores[0];
    shipHighScores[7] = scores[1];
    shipHighScores[8] = scores[2];

    shipPlayTime[5] = times[3];
    shipPlayTime[6] = times[0];
    shipPlayTime[7] = times[1];
    shipPlayTime[8] = times[2];

    shipKills[5] = kills[3];
    shipKills[6] = kills[0];
    shipKills[7] = kills[1];
    shipKills[8] = kills[2];

    switch (starshipType) {
        case 5: starshipType = 6; break;
        case 6: starshipType = 7; break;
        case 7: starshipType = 8; break;
        case 8: starshipType = 5; break;
        default: break;
    }

    shipLayoutVersion = 1;
    SaveGameData();
}

void LoadGameData() {
    InitDefaultUnlocks();
    InitRunHistory();

    if (!FileExists(SAVE_FILE)) {
        return;
    }

    char* text = LoadFileText(SAVE_FILE);
    if (!text) {
        return;
    }

    char* line = strtok(text, "\n");
    while (line) {
        char key[64];
        char value[64];
        if (sscanf(line, "%63[^=]=%63s", key, value) == 2) {
            if (strcmp(key, "debugging") == 0) {
                debugging = (strcmp(value, "1") == 0);
            } else if (strcmp(key, "enemyDebugging") == 0) {
                enemyDebugging = (strcmp(value, "1") == 0);
            } else if (strcmp(key, "starshipType") == 0) {
                starshipType = atoi(value);
            } else if (strcmp(key, "yWrapping") == 0) {
                yWrapping = (strcmp(value, "1") == 0);
            } else if (strcmp(key, "directionPointerOn") == 0) {
                directionPointerOn = (strcmp(value, "1") == 0);
            } else if (strcmp(key, "asteroidsOn") == 0) {
                asteroidsOn = (strcmp(value, "1") == 0);
            } else if (strcmp(key, "asteroidSpawnInterval") == 0) {
                asteroidSpawnInterval = (float)atof(value);
            } else if (strcmp(key, "runDifficulty") == 0) {
                int diff = atoi(value);
                if (diff >= 0 && diff < RIFT_DIFFICULTY_COUNT) {
                    runDifficulty = static_cast<RiftDifficulty>(diff);
                }
            } else if (strcmp(key, "highScoreQuickPlay") == 0) {
                highScoreQuickPlay = atoi(value);
            } else if (strcmp(key, "highScoreAsteroids") == 0) {
                highScoreAsteroids = atoi(value);
            } else if (strcmp(key, "totalShipsDestroyed") == 0) {
                totalShipsDestroyed = atoi(value);
            } else if (strcmp(key, "playerCredits") == 0) {
                playerCredits = atoi(value);
            } else if (strcmp(key, "riftVictories") == 0) {
                riftVictories = atoi(value);
            } else if (strcmp(key, "asteroidsVictories") == 0) {
                asteroidsVictories = atoi(value);
            } else if (strcmp(key, "metaOwned") == 0) {
                ApplyMetaOwnedBitmask(atoi(value));
            } else if (strcmp(key, "metaEnabled") == 0) {
                ApplyMetaEnabledBitmask(atoi(value));
            } else if (strcmp(key, "unlockedShips") == 0) {
                ApplyUnlockedBitmask(atoi(value));
            } else if (strcmp(key, "shipLayoutVersion") == 0) {
                shipLayoutVersion = atoi(value);
            } else if (strcmp(key, "masterVolume") == 0) {
                masterVolume = (float)atof(value);
            } else if (strcmp(key, "sfxVolume") == 0) {
                sfxVolume = (float)atof(value);
            } else if (strcmp(key, "musicVolume") == 0) {
                musicVolume = (float)atof(value);
            } else if (strcmp(key, "audioMuted") == 0) {
                audioMuted = (strcmp(value, "1") == 0);
            } else if (TryParseRunHistoryKey(key, value)) {
                // handled by run history
            } else {
                for (int i = 0; i < MAX_SHIPS; i++) {
                    char highKey[32];
                    char playKey[32];
                    char killsKey[32];
                    snprintf(highKey, sizeof(highKey), "ship%dHighScore", i);
                    snprintf(playKey, sizeof(playKey), "ship%dPlayTime", i);
                    snprintf(killsKey, sizeof(killsKey), "ship%dKills", i);
                    if (strcmp(key, highKey) == 0) {
                        shipHighScores[i] = atoi(value);
                    } else if (strcmp(key, playKey) == 0) {
                        shipPlayTime[i] = (float)atof(value);
                    } else if (strcmp(key, killsKey) == 0) {
                        shipKills[i] = atoi(value);
                    }
                }
            }
        }
        line = strtok(nullptr, "\n");
    }

    UnloadFileText(text);

    MigrateShipLayoutV1();

    if (starshipType < 0 || starshipType >= MAX_SHIPS) {
        starshipType = 0;
    }
    if (asteroidSpawnInterval < 0.5f) {
        asteroidSpawnInterval = 0.5f;
    } else if (asteroidSpawnInterval > 10.0f) {
        asteroidSpawnInterval = 10.0f;
    }
    if (masterVolume < 0.0f) masterVolume = 0.0f;
    else if (masterVolume > 1.0f) masterVolume = 1.0f;
    if (sfxVolume < 0.0f) sfxVolume = 0.0f;
    else if (sfxVolume > 1.0f) sfxVolume = 1.0f;
    if (musicVolume < 0.0f) musicVolume = 0.0f;
    else if (musicVolume > 1.0f) musicVolume = 1.0f;

    if (!IsShipUnlocked(starshipType)) {
        for (int i = 0; i < MAX_SHIPS; i++) {
            if (IsShipUnlocked(i)) {
                starshipType = i;
                break;
            }
        }
    }

#ifndef DEV_BUILD
    debugging = false;
    enemyDebugging = false;
    asteroidsOn = true;
    asteroidSpawnInterval = 5.0f;
#endif
}

void SaveGameData() {
    starshipType = player.starshipType;

#ifndef DEV_BUILD
    const int saveDebugging = 0;
    const int saveEnemyDebugging = 0;
    const int saveAsteroidsOn = 1;
    const float saveAsteroidSpawnInterval = 5.0f;
#else
    const int saveDebugging = debugging ? 1 : 0;
    const int saveEnemyDebugging = enemyDebugging ? 1 : 0;
    const int saveAsteroidsOn = asteroidsOn ? 1 : 0;
    const float saveAsteroidSpawnInterval = asteroidSpawnInterval;
#endif

    char text[8192];
    int offset = snprintf(text, sizeof(text),
        "debugging=%d\n"
        "enemyDebugging=%d\n"
        "starshipType=%d\n"
        "yWrapping=%d\n"
        "directionPointerOn=%d\n"
        "asteroidsOn=%d\n"
        "asteroidSpawnInterval=%.1f\n"
        "runDifficulty=%d\n"
        "highScoreQuickPlay=%d\n"
        "highScoreAsteroids=%d\n"
        "totalShipsDestroyed=%d\n"
        "playerCredits=%d\n"
        "riftVictories=%d\n"
        "asteroidsVictories=%d\n"
        "metaOwned=%d\n"
        "metaEnabled=%d\n"
        "unlockedShips=%d\n"
        "shipLayoutVersion=%d\n"
        "masterVolume=%.1f\n"
        "sfxVolume=%.1f\n"
        "musicVolume=%.1f\n"
        "audioMuted=%d\n",
        saveDebugging,
        saveEnemyDebugging,
        starshipType,
        yWrapping ? 1 : 0,
        directionPointerOn ? 1 : 0,
        saveAsteroidsOn,
        saveAsteroidSpawnInterval,
        static_cast<int>(runDifficulty),
        highScoreQuickPlay,
        highScoreAsteroids,
        totalShipsDestroyed,
        playerCredits,
        riftVictories,
        asteroidsVictories,
        GetMetaOwnedBitmask(),
        GetMetaEnabledBitmask(),
        UnlockedShipsBitmask(),
        shipLayoutVersion,
        masterVolume,
        sfxVolume,
        musicVolume,
        audioMuted ? 1 : 0
    );

    for (int i = 0; i < MAX_SHIPS; i++) {
        offset += snprintf(text + offset, sizeof(text) - (size_t)offset,
            "ship%dHighScore=%d\n"
            "ship%dPlayTime=%.1f\n"
            "ship%dKills=%d\n",
            i, shipHighScores[i],
            i, shipPlayTime[i],
            i, shipKills[i]
        );
    }

    AppendRunHistorySaveLines(text, &offset, sizeof(text));

    SaveFileText(SAVE_FILE, text);
}

void UpdateHighScores() {
    bool updated = false;

    if (asteroidsMode) {
        if (currentRun.score > highScoreAsteroids) {
            highScoreAsteroids = currentRun.score;
            updated = true;
        }
        int shipIdx = player.starshipType;
        if (shipIdx >= 0 && shipIdx < MAX_SHIPS) {
            if (currentRun.score > shipHighScores[shipIdx]) {
                shipHighScores[shipIdx] = currentRun.score;
                updated = true;
            }
        }
    } else if (currentRun.time > 0.0f) {
        int survivalSeconds = (int)currentRun.time;
        if (survivalSeconds > highScoreQuickPlay) {
            highScoreQuickPlay = survivalSeconds;
            updated = true;
        }
    }

    if (updated) {
        SaveGameData();
    }
}

void UpdateShipStats(float deltaTime) {
    int shipIdx = player.starshipType;
    if (shipIdx >= 0 && shipIdx < MAX_SHIPS) {
        shipPlayTime[shipIdx] += deltaTime;
    }
}
