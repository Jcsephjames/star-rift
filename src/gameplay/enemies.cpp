#include "enemies.hpp"
#include "audio.hpp"
#include <cmath>
#include <vector>
#include <raymath.h>
#include "globals.hpp"
#include "player.hpp"
#include "laserManager.hpp"
#include "effects.hpp"
#include "pickups.hpp"
#include "scrap.hpp"
#include "runModifiers.hpp"
#include "asteroids.hpp"
#include "engineExhaust.hpp"
#include "difficulty.hpp"
#include "ship.hpp"

Enemy enemies[MAX_ENEMIES] = { 0 };

static bool falconSpawnedThisGame = false;
static bool slasherSpawnedThisGame = false;
static bool deathStarSpawnedThisGame = false;
static bool bossSpawnQueued = false;
static bool deathStarSpawnQueued = false;
static int lastAnnouncedWave = 0;

static const int BOSS_WAVE = RIFT_BOSS_WAVE;
static const int DEATH_STAR_WAVE = RIFT_FINALE_WAVE;
static const float BOSS_SPAWN_WARNING_DURATION = 3.0f;
static const float DEATH_STAR_SPAWN_WARNING_DURATION = 4.5f;

static const float GRUNT_HEALTH = 10.0f;
static const float GRUNT_DROP = 5.0f;
static const int GRUNT_SCRAP = 10;
static const float RARE_HEALTH = 20.0f;
static const float RARE_DROP = 10.0f;
static const int RARE_SCRAP = 22;
static const float ELITE_HEALTH = 25.0f;
static const float ELITE_DROP = 15.0f;
static const int ELITE_SCRAP = 38;
static const float FALCON_HEALTH = 50.0f;
static const float FALCON_DROP = 100.0f;
static const int FALCON_SCRAP = 92;
static const float DEATH_STAR_HEALTH = 220.0f;
static const float DEATH_STAR_DROP = 150.0f;
static const int DEATH_STAR_SCRAP = 200;

int GetEnemyScrapDrop(EnemyTier tier) {
    switch (tier) {
        case EnemyTier::GRUNT: return GRUNT_SCRAP;
        case EnemyTier::RARE: return RARE_SCRAP;
        case EnemyTier::ELITE: return ELITE_SCRAP;
        case EnemyTier::FALCON: return FALCON_SCRAP;
        case EnemyTier::DEATH_STAR: return DEATH_STAR_SCRAP;
    }
    return GRUNT_SCRAP;
}

static const float TELEGRAPH_DURATION = 0.25f;
static const float REPOSITION_DURATION = 2.0f;
static const float FIRING_RUN_DURATION = 3.5f;
static const float PURSUE_DURATION = 3.0f;
static const float WAVE_DURATION = 40.0f;
static const float ASTEROID_FIELD_WAVE_DURATION = 28.0f;
static const int ASTEROID_FIELD_ROLL_CHANCE = 15;

static int currentWave = 1;
static float waveElapsed = 0.0f;
static float currentWaveDuration = WAVE_DURATION;
static bool asteroidFieldActive = false;
static const float SPAWN_RATE_MULTIPLIER = 2.0f;
static const float SPAWN_WARNING_DURATION = 1.5f;
static const float WAVE_SPAWN_INTERVAL_REDUCTION = 0.30f;
static const float MIN_SPAWN_INTERVAL = 1.5f;
static const int LATE_GAME_ENEMY_CAP = 10;
static const int WAVE_ENEMY_CAP_BONUS_EVERY = 3;
static const float ASTEROID_AVOID_BASE_MARGIN = 50.0f;
static const float ASTEROID_AVOID_FORWARD_DOT = 0.42f;
static const float ASTEROID_AVOID_TURN_SPEED = 48.0f;
static const float ASTEROID_AVOID_MAX_BLEND = 0.42f;
static const float ASTEROID_AVOID_LOCKED_TURN_SPEED = 18.0f;
static const float ASTEROID_AVOID_LOCKED_MAX_BLEND = 0.08f;

static float NormalizeAngle(float angle) {
    while (angle >= 360.0f) angle -= 360.0f;
    while (angle < 0.0f) angle += 360.0f;
    return angle;
}

static float AngleDifference(float from, float to) {
    float diff = to - from;
    while (diff > 180.0f) diff -= 360.0f;
    while (diff < -180.0f) diff += 360.0f;
    return diff;
}

static Vector2 EnemyForwardVector(float direction) {
    float rad = (direction - 90.0f) * DEG2RAD;
    return { cosf(rad), sinf(rad) };
}

static bool IsTailshot(const Enemy& enemy, const Laser& laser) {
    Vector2 forward = EnemyForwardVector(enemy.direction);
    return Vector2DotProduct(forward, laser.direction) > 0.55f;
}

int GetEnemyWave() {
    if (asteroidsMode || !enemiesOn) {
        return 1 + (int)(currentRun.time / WAVE_DURATION);
    }
    return currentWave;
}

bool IsAsteroidFieldWave() {
    return asteroidFieldActive && !asteroidsMode && enemiesOn;
}

static int GetGruntPercentForWave(int wave) {
    int pct = 95 - (wave - 1) * 5;
    return pct < 75 ? 75 : pct;
}

static void BeginWave(int wave) {
    currentWave = wave;
    waveElapsed = 0.0f;
    asteroidFieldActive = false;
    currentWaveDuration = WAVE_DURATION;

    if (asteroidsOn && GetRandomValue(0, 99) < ASTEROID_FIELD_ROLL_CHANCE) {
        asteroidFieldActive = true;
        currentWaveDuration = ASTEROID_FIELD_WAVE_DURATION;
        SpawnFloatingText(
            { gameWidth / 2.0f, gameHeight * 0.22f },
            "Asteroid Field",
            (Color){ 180, 200, 255, 255 }
        );
    } else {
        SpawnFloatingText(
            { gameWidth / 2.0f, gameHeight * 0.22f },
            TextFormat("Wave %d", wave),
            (Color){ 255, 210, 80, 255 }
        );
    }

    PlaySfx(SfxId::WAVE_ANNOUNCE);
    lastAnnouncedWave = wave;

    if (wave == BOSS_WAVE) {
        ShipAllegiance playerAllegiance = Ship::GetAllegianceForType(player.starshipType);
        bool playerIsRebellion = playerAllegiance == ShipAllegiance::REBELLION;

        if (!playerIsRebellion && !falconSpawnedThisGame) {
            bossSpawnQueued = true;
            SpawnFloatingText(
                { gameWidth / 2.0f, gameHeight * 0.14f },
                TextFormat("BOSS INCOMING — %s", FalconShip.name.c_str()),
                (Color){ 255, 90, 60, 255 }
            );
        } else if (playerIsRebellion && !slasherSpawnedThisGame) {
            bossSpawnQueued = true;
            SpawnFloatingText(
                { gameWidth / 2.0f, gameHeight * 0.14f },
                TextFormat("BOSS INCOMING — %s", ShlasherShip.name.c_str()),
                (Color){ 255, 70, 110, 255 }
            );
        }
    }

    if (wave == DEATH_STAR_WAVE && !deathStarSpawnedThisGame) {
        deathStarSpawnQueued = true;
        SpawnFloatingText(
            { gameWidth / 2.0f, gameHeight * 0.14f },
            TextFormat("FINAL BOSS — %s", DeathStarShip.name.c_str()),
            (Color){ 180, 220, 255, 255 }
        );
    }
}

static void UpdateWaves(float deltaTime) {
    if (asteroidsMode || !enemiesOn) return;

    if (lastAnnouncedWave == 0) {
        BeginWave(1);
        return;
    }

    waveElapsed += deltaTime;
    if (waveElapsed >= currentWaveDuration) {
        BeginWave(currentWave + 1);
    }
}

struct SpawnChoice {
    const Ship* ship;
    EnemyTier tier;
    float health;
    float healthDrop;
};

struct PendingEnemySpawn {
    bool active;
    bool isBossSpawn;
    bool spawnAtTopCenter;
    float timer;
    float warningDuration;
    bool spawnLeft;
    float spawnY;
    SpawnChoice choice;
    EnemyPlaystyle playstyle;
    float direction;
    float breakDirection;
};

static PendingEnemySpawn pendingSpawn = { false };

static SpawnChoice GetBossSpawnChoice() {
    ShipAllegiance playerAllegiance = Ship::GetAllegianceForType(player.starshipType);
    bool playerIsRebellion = playerAllegiance == ShipAllegiance::REBELLION;

    if (!playerIsRebellion) {
        falconSpawnedThisGame = true;
        return { &FalconShip, EnemyTier::FALCON, FALCON_HEALTH, FALCON_DROP };
    }

    slasherSpawnedThisGame = true;
    return { &ShlasherShip, EnemyTier::FALCON, FALCON_HEALTH, FALCON_DROP };
}

static SpawnChoice GetDeathStarSpawnChoice() {
    return { &DeathStarShip, EnemyTier::DEATH_STAR, DEATH_STAR_HEALTH, DEATH_STAR_DROP };
}

static EnemyPlaystyle PickBossPlaystyle(const SpawnChoice& choice) {
    if (choice.tier == EnemyTier::DEATH_STAR) return EnemyPlaystyle::BOSS_STATION;
    if (choice.ship == &FalconShip) return EnemyPlaystyle::BOSS_GUNNER;
    return EnemyPlaystyle::BOSS_BLADE;
}

static SpawnChoice PickOpposingShip() {
    ShipAllegiance playerAllegiance = Ship::GetAllegianceForType(player.starshipType);
    bool playerIsRebellion = playerAllegiance == ShipAllegiance::REBELLION;
    int wave = GetEnemyWave();
    int gruntThreshold = GetGruntPercentForWave(wave) * 10;
    int roll = GetRandomValue(0, 999);

    if (roll < gruntThreshold) {
        const Ship* ship = playerIsRebellion ? &TieFighterShip : &XWingShip;
        return { ship, EnemyTier::GRUNT, GRUNT_HEALTH, GRUNT_DROP };
    }

    int subRoll = GetRandomValue(0, 999);
    if (subRoll < 700) {
        const Ship* ship = playerIsRebellion ? &TieBomberShip : &YWingShip;
        return { ship, EnemyTier::RARE, RARE_HEALTH, RARE_DROP };
    }

    if (playerIsRebellion) {
        const Ship* ship = GetRandomValue(0, 1) == 0 ? &DeathFighterShip : &ShipTwoShip;
        return { ship, EnemyTier::ELITE, ELITE_HEALTH, ELITE_DROP };
    }

    return { &StarWarsFighterOneShip, EnemyTier::ELITE, ELITE_HEALTH, ELITE_DROP };
}

static int GetActiveEnemyCap() {
    float time = currentRun.time;
    int baseCap;
    if (time < 18.0f) baseCap = 2;
    else if (time < 36.0f) baseCap = 5;
    else if (time < 55.0f) baseCap = 8;
    else baseCap = LATE_GAME_ENEMY_CAP;

    int wave = GetEnemyWave();
    int waveBonus = (wave - 1) / WAVE_ENEMY_CAP_BONUS_EVERY;
    if (waveBonus > 4) waveBonus = 4;

    int cap = baseCap + waveBonus + GetDifficultyEnemyCapBonus();
    return cap < LATE_GAME_ENEMY_CAP ? cap : LATE_GAME_ENEMY_CAP;
}

static float GetSpawnInterval() {
    float time = currentRun.time;
    float ramp = fminf(time / 90.0f, 1.0f);
    float interval = (enemySpawnInterval - ramp * 3.0f) / (SPAWN_RATE_MULTIPLIER * SPAWN_RATE_BOOST);

    int wave = GetEnemyWave();
    interval -= (wave - 1) * WAVE_SPAWN_INTERVAL_REDUCTION;
    return interval < MIN_SPAWN_INTERVAL ? MIN_SPAWN_INTERVAL : interval;
}

static EnemyPlaystyle PickPlaystyle(EnemyTier tier) {
    int stalkerMax;
    int firingRunMax;

    switch (tier) {
        case EnemyTier::RARE:
            // TIE Bomber / Y-Wing — slightly more firing runs
            stalkerMax = 27;
            firingRunMax = 69;
            break;
        case EnemyTier::ELITE:
            // Death Fighter / Ship Two / Star Fighter — pursue more
            stalkerMax = 27;
            firingRunMax = 58;
            break;
        default:
            // Grunts and Falcon keep the baseline mix
            stalkerMax = 40;
            firingRunMax = 70;
            break;
    }

    int roll = GetRandomValue(0, 99);
    if (roll < stalkerMax) return EnemyPlaystyle::STALKER;
    if (roll < firingRunMax) return EnemyPlaystyle::FIRING_RUN;
    return EnemyPlaystyle::PURSUE;
}

static float GetEnemyRadius(EnemyTier tier) {
    switch (tier) {
        case EnemyTier::DEATH_STAR: return 78.0f;
        case EnemyTier::FALCON: return 34.0f;
        default: return 28.0f;
    }
}

static int SpawnEnemyAt(const SpawnChoice& choice, EnemyPlaystyle playstyle, Vector2 position,
                        float direction, float breakDirection) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) continue;

        enemies[i] = {
            .active = true,
            .ship = *choice.ship,
            .tier = choice.tier,
            .playstyle = playstyle,
            .healthDrop = choice.healthDrop,
            .position = position,
            .direction = direction,
            .health = choice.health,
            .maxHealth = choice.health,
            .hitFlashTimer = 0.0f,
            .timeSinceLastShot = (float)GetRandomValue(0, 30) / 10.0f,
            .breakDirection = breakDirection,
            .radius = GetEnemyRadius(choice.tier),
            .telegraphTimer = 0.0f,
            .pendingFireDirection = 0.0f,
            .repositionTimer = 0.0f,
            .commitTimer = 0.0f,
            .runHeading = direction,
            .collisionDamageCooldown = 0.0f,
            .enteringScreen = choice.tier != EnemyTier::DEATH_STAR,
            .prevDirection = direction,
            .rotationRate = 0.0f,
            .thrustIntensity = 0.7f,
            .thrustBoost = 1.0f
        };
        return i;
    }
    return -1;
}

static void BeginPendingSpawn(bool isBoss) {
    bool spawnLeft = GetRandomValue(0, 1) == 0;
    float angleBase = spawnLeft ? 0.0f : 180.0f;

    SpawnChoice choice = isBoss ? GetBossSpawnChoice() : PickOpposingShip();
    float warningDuration = isBoss ? BOSS_SPAWN_WARNING_DURATION : SPAWN_WARNING_DURATION;
    float spawnY = isBoss
        ? gameHeight * 0.5f
        : (float)GetRandomValue(80, gameHeight - 80);

    pendingSpawn = {
        .active = true,
        .isBossSpawn = isBoss,
        .spawnAtTopCenter = false,
        .timer = warningDuration,
        .warningDuration = warningDuration,
        .spawnLeft = spawnLeft,
        .spawnY = spawnY,
        .choice = choice,
        .playstyle = isBoss ? PickBossPlaystyle(choice) : PickPlaystyle(choice.tier),
        .direction = angleBase + (float)GetRandomValue(-25, 25),
        .breakDirection = GetRandomValue(0, 1) == 0 ? -1.0f : 1.0f
    };
}

static void BeginDeathStarPendingSpawn() {
    deathStarSpawnQueued = false;
    deathStarSpawnedThisGame = true;

    SpawnChoice choice = GetDeathStarSpawnChoice();
    pendingSpawn = {
        .active = true,
        .isBossSpawn = true,
        .spawnAtTopCenter = true,
        .timer = DEATH_STAR_SPAWN_WARNING_DURATION,
        .warningDuration = DEATH_STAR_SPAWN_WARNING_DURATION,
        .spawnLeft = false,
        .spawnY = gameHeight * 0.14f,
        .choice = choice,
        .playstyle = EnemyPlaystyle::BOSS_STATION,
        .direction = 180.0f,
        .breakDirection = 1.0f
    };
}

static void BeginBossPendingSpawn() {
    bossSpawnQueued = false;
    BeginPendingSpawn(true);
}

static void CompletePendingSpawn() {
    if (!pendingSpawn.active) return;

    Vector2 position;
    if (pendingSpawn.spawnAtTopCenter) {
        position = { gameWidth / 2.0f, pendingSpawn.spawnY };
    } else {
        position = {
            pendingSpawn.spawnLeft ? -40.0f : (float)gameWidth + 40.0f,
            pendingSpawn.spawnY
        };
    }

    SpawnEnemyAt(
        pendingSpawn.choice,
        pendingSpawn.playstyle,
        position,
        pendingSpawn.direction,
        pendingSpawn.breakDirection
    );
    pendingSpawn.active = false;
}

static void UpdatePendingSpawn(float deltaTime) {
    if (!pendingSpawn.active) return;

    pendingSpawn.timer -= deltaTime;
    if (pendingSpawn.timer <= 0.0f) {
        CompletePendingSpawn();
    }
}

static int SpawnEnemy() {
    SpawnChoice choice = PickOpposingShip();
    bool spawnLeft = GetRandomValue(0, 1) == 0;
    Vector2 position = {
        spawnLeft ? -40.0f : (float)gameWidth + 40.0f,
        (float)GetRandomValue(80, gameHeight - 80)
    };
    float angleBase = spawnLeft ? 0.0f : 180.0f;
    float direction = angleBase + (float)GetRandomValue(-25, 25);
    float breakDirection = GetRandomValue(0, 1) == 0 ? -1.0f : 1.0f;

    return SpawnEnemyAt(choice, PickPlaystyle(choice.tier), position, direction, breakDirection);
}

static SpawnChoice GetGruntSpawnChoice() {
    ShipAllegiance playerAllegiance = Ship::GetAllegianceForType(player.starshipType);
    bool playerIsRebellion = playerAllegiance == ShipAllegiance::REBELLION;
    const Ship* ship = playerIsRebellion ? &TieFighterShip : &XWingShip;
    return { ship, EnemyTier::GRUNT, GRUNT_HEALTH, GRUNT_DROP };
}

static void SpawnInitialGrunts() {
    SpawnChoice grunt = GetGruntSpawnChoice();

    for (int i = 0; i < 2; i++) {
        bool spawnLeft = (i == 0);
        Vector2 position = {
            spawnLeft ? -40.0f : (float)gameWidth + 40.0f,
            (float)GetRandomValue(120, gameHeight - 120)
        };
        float angleBase = spawnLeft ? 0.0f : 180.0f;
        float direction = angleBase + (float)GetRandomValue(-20, 20);
        float breakDirection = GetRandomValue(0, 1) == 0 ? -1.0f : 1.0f;

        SpawnEnemyAt(grunt, PickPlaystyle(EnemyTier::GRUNT), position, direction, breakDirection);
    }
}

void InitEnemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i] = { 0 };
    }
    enemySpawnTimer = 0.0f;
    falconSpawnedThisGame = false;
    slasherSpawnedThisGame = false;
    deathStarSpawnedThisGame = false;
    bossSpawnQueued = false;
    deathStarSpawnQueued = false;
    lastAnnouncedWave = 0;
    currentWave = 1;
    waveElapsed = 0.0f;
    currentWaveDuration = WAVE_DURATION;
    asteroidFieldActive = false;
    pendingSpawn.active = false;
    InitPickups();
    SpawnInitialGrunts();
}

void ContinueEndlessRift() {
    if (asteroidsMode || !enemiesOn) return;

    SpawnFloatingText(
        { gameWidth / 2.0f, gameHeight * 0.14f },
        "ENDLESS MODE",
        (Color){ 255, 210, 80, 255 }
    );
    BeginWave(currentWave + 1);
}

int CountActiveEnemies() {
    int count = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) count++;
    }
    return count;
}

static void EnemySpawnTimer(float deltaTime) {
    if (!enemiesOn || asteroidsMode) return;

    UpdateWaves(deltaTime);

    if (pendingSpawn.active) return;

    if (bossSpawnQueued) {
        BeginBossPendingSpawn();
        return;
    }

    if (deathStarSpawnQueued) {
        BeginDeathStarPendingSpawn();
        return;
    }

    enemySpawnTimer += deltaTime;
    if (enemySpawnTimer < GetSpawnInterval()) return;
    enemySpawnTimer = 0.0f;

    if (CountActiveEnemies() >= GetActiveEnemyCap()) return;
    BeginPendingSpawn(false);
}

static Vector2 AngleToForward(float angleDeg) {
    float rad = (angleDeg - 90.0f) * DEG2RAD;
    return { cosf(rad), sinf(rad) };
}

static float ForwardToAngle(Vector2 forward) {
    float len = Vector2Length(forward);
    if (len < 0.001f) return 0.0f;
    return NormalizeAngle(atan2f(forward.y, forward.x) * RAD2DEG + 90.0f);
}

static bool IsEnemyAttackLocked(const Enemy* enemy) {
    return enemy->commitTimer > 0.0f || enemy->telegraphTimer > 0.0f;
}

static bool ComputeAsteroidAvoidance(const Enemy* enemy, float desiredAngle, float* outAngle, float* outUrgency, float* outTurnSpeed, int* outThreatAsteroidIdx = nullptr) {
    if (!asteroidsOn) return false;

    bool locked = IsEnemyAttackLocked(enemy);
    float maxBlend = locked ? ASTEROID_AVOID_LOCKED_MAX_BLEND : ASTEROID_AVOID_MAX_BLEND;
    float urgencyScale = locked ? 0.12f : 0.55f;
    float urgencyThreshold = locked ? 0.15f : 0.06f;
    *outTurnSpeed = locked ? ASTEROID_AVOID_LOCKED_TURN_SPEED : ASTEROID_AVOID_TURN_SPEED;

    Vector2 forward = AngleToForward(enemy->direction);
    Vector2 avoidSum = { 0.0f, 0.0f };
    float maxUrgency = 0.0f;
    int threatAsteroidIdx = -1;

    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        const Asteroid& asteroid = asteroids[i];
        if (!asteroid.active) continue;

        Vector2 toAsteroid = Vector2Subtract(asteroid.position, enemy->position);
        float dist = Vector2Length(toAsteroid);
        if (dist < 0.001f) continue;

        Vector2 toAsteroidDir = Vector2Scale(toAsteroid, 1.0f / dist);
        if (Vector2DotProduct(forward, toAsteroidDir) < ASTEROID_AVOID_FORWARD_DOT) continue;

        float astRadius = GetAsteroidRadius(asteroid.size);
        float detectRange = ASTEROID_AVOID_BASE_MARGIN + astRadius + enemy->radius;
        if (dist >= detectRange) continue;

        Vector2 away = Vector2Normalize(Vector2Subtract(enemy->position, asteroid.position));
        float urgency = 1.0f - (dist / detectRange);
        urgency *= urgency;

        avoidSum = Vector2Add(avoidSum, Vector2Scale(away, urgency));
        if (urgency > maxUrgency) {
            maxUrgency = urgency;
            threatAsteroidIdx = i;
        }
    }

    if (outThreatAsteroidIdx) *outThreatAsteroidIdx = threatAsteroidIdx;

    if (maxUrgency < urgencyThreshold) return false;

    Vector2 avoidDir = Vector2Length(avoidSum) > 0.001f
        ? Vector2Normalize(avoidSum)
        : AngleToForward(desiredAngle);
    Vector2 desiredDir = AngleToForward(desiredAngle);
    float blend = fminf(maxUrgency * urgencyScale, maxBlend);
    Vector2 blended = Vector2Normalize(Vector2Add(
        Vector2Scale(desiredDir, 1.0f - blend),
        Vector2Scale(avoidDir, blend)
    ));

    *outAngle = ForwardToAngle(blended);
    *outUrgency = maxUrgency;
    return true;
}

static void TurnToward(Enemy* enemy, float desiredAngle, float turnSpeed, float deltaTime) {
    float angleDiff = AngleDifference(enemy->direction, desiredAngle);
    if (fabsf(angleDiff) <= turnSpeed * deltaTime) {
        enemy->direction = desiredAngle;
    } else {
        enemy->direction += (angleDiff > 0.0f ? turnSpeed : -turnSpeed) * deltaTime;
    }
    enemy->direction = NormalizeAngle(enemy->direction);
}

static void FireEnemyLaser(Enemy* enemy, float fireDirection, bool triggerReposition) {
    FireSingleLaser(
        enemy->position,
        fireDirection,
        enemy->ship.shotSpeed,
        enemy->ship.laserSize,
        enemy->ship.laserOffsets,
        enemy->ship.damage,
        true
    );
    enemy->timeSinceLastShot = 0.0f;
    if (triggerReposition) {
        enemy->repositionTimer = REPOSITION_DURATION + (float)GetRandomValue(0, 6) / 10.0f;
    }
}

static void TryFireAtThreateningAsteroid(Enemy* enemy, int asteroidIdx, float urgency) {
    if (asteroidIdx < 0 || urgency < 0.18f) return;
    if (enemy->telegraphTimer > 0.0f) return;
    if (enemy->timeSinceLastShot < enemy->ship.shotCooldown * 0.7f) return;

    const Asteroid& asteroid = asteroids[asteroidIdx];
    float fireDirection = atan2f(
        asteroid.position.y - enemy->position.y,
        asteroid.position.x - enemy->position.x
    ) * RAD2DEG + 90.0f;
    fireDirection = NormalizeAngle(fireDirection);

    float aimDiff = fabsf(AngleDifference(enemy->direction, fireDirection));
    if (aimDiff > 45.0f) return;

    int skipChance = (int)(25.0f + urgency * 35.0f);
    if (GetRandomValue(0, 100) > skipChance) return;

    FireEnemyLaser(enemy, fireDirection + (float)GetRandomValue(-6, 6), false);
}

static void MoveEnemy(Enemy* enemy, float desiredAngle, float deltaTime, float thrustScale = 1.0f) {
    const float normalTurnSpeed = 125.0f;
    float turnSpeed = normalTurnSpeed;
    float urgency = 0.0f;
    float adjustedAngle = desiredAngle;

    int threatAsteroidIdx = -1;
    float avoidTurnSpeed = ASTEROID_AVOID_TURN_SPEED;
    if (ComputeAsteroidAvoidance(enemy, desiredAngle, &adjustedAngle, &urgency, &avoidTurnSpeed, &threatAsteroidIdx)) {
        desiredAngle = adjustedAngle;
        if (IsEnemyAttackLocked(enemy)) {
            turnSpeed = avoidTurnSpeed;
        } else {
            turnSpeed = avoidTurnSpeed +
                (normalTurnSpeed - avoidTurnSpeed) * (1.0f - urgency);
        }
        TryFireAtThreateningAsteroid(enemy, threatAsteroidIdx, urgency);
    }

    TurnToward(enemy, desiredAngle, turnSpeed, deltaTime);

    float angleDiff = AngleDifference(enemy->direction, desiredAngle);
    float rad = (enemy->direction - 90.0f) * DEG2RAD;
    float thrust = enemy->ship.speed * 0.72f * thrustScale;
    float turnPenalty = 1.0f - fminf(fabsf(angleDiff) / 160.0f, 1.0f) * 0.2f;

    enemy->thrustIntensity = fmaxf(enemy->thrustIntensity, 0.55f + 0.3f * thrustScale);
    enemy->thrustBoost = fmaxf(enemy->thrustBoost, thrustScale > 1.0f ? thrustScale : 1.0f);

    enemy->position.x += cosf(rad) * thrust * turnPenalty * deltaTime;
    enemy->position.y += sinf(rad) * thrust * turnPenalty * deltaTime;
}

static void MoveEnemyStraight(Enemy* enemy, float deltaTime, float thrustScale = 1.0f) {
    float heading = enemy->runHeading;
    float urgency = 0.0f;
    float adjustedHeading = heading;
    float avoidTurnSpeed = ASTEROID_AVOID_LOCKED_TURN_SPEED;

    int threatAsteroidIdx = -1;
    if (ComputeAsteroidAvoidance(enemy, heading, &adjustedHeading, &urgency, &avoidTurnSpeed, &threatAsteroidIdx)) {
        float blend = fminf(urgency * 0.12f, ASTEROID_AVOID_LOCKED_MAX_BLEND);
        Vector2 runDir = AngleToForward(heading);
        Vector2 avoidDir = AngleToForward(adjustedHeading);
        Vector2 blended = Vector2Normalize(Vector2Add(
            Vector2Scale(runDir, 1.0f - blend),
            Vector2Scale(avoidDir, blend)
        ));
        heading = ForwardToAngle(blended);
        TurnToward(enemy, heading, avoidTurnSpeed, deltaTime);
        enemy->runHeading = enemy->direction;
        TryFireAtThreateningAsteroid(enemy, threatAsteroidIdx, urgency);
    } else {
        enemy->direction = heading;
    }

    float rad = (enemy->runHeading - 90.0f) * DEG2RAD;
    float thrust = enemy->ship.speed * 0.85f * thrustScale;
    enemy->thrustIntensity = fmaxf(enemy->thrustIntensity, 0.65f + 0.35f * thrustScale);
    enemy->thrustBoost = fmaxf(enemy->thrustBoost, thrustScale > 1.0f ? thrustScale : 1.0f);
    enemy->position.x += cosf(rad) * thrust * deltaTime;
    enemy->position.y += sinf(rad) * thrust * deltaTime;
}

static void UpdateReposition(Enemy* enemy, float deltaTime, float angleToPlayer) {
    enemy->repositionTimer -= deltaTime;
    float peelAmount = 130.0f + (1.0f - fmaxf(enemy->repositionTimer, 0.0f) / REPOSITION_DURATION) * 40.0f;
    float desiredAngle = NormalizeAngle(angleToPlayer + enemy->breakDirection * peelAmount);
    MoveEnemy(enemy, desiredAngle, deltaTime, 1.05f);
}

static void TryStartTelegraph(Enemy* enemy, float fireDirection) {
    if (enemy->telegraphTimer > 0.0f) return;

    enemy->telegraphTimer = TELEGRAPH_DURATION;
    enemy->pendingFireDirection = fireDirection;
}

static void EndCommitPhase(Enemy* enemy) {
    enemy->commitTimer = 0.0f;
    enemy->repositionTimer = REPOSITION_DURATION + (float)GetRandomValue(0, 8) / 10.0f;
}

static void UpdateEnemyTelegraph(Enemy* enemy, float deltaTime) {
    if (enemy->playstyle == EnemyPlaystyle::FIRING_RUN) return;
    if (enemy->telegraphTimer <= 0.0f) return;

    enemy->telegraphTimer -= deltaTime;
    if (enemy->telegraphTimer <= 0.0f) {
        enemy->telegraphTimer = 0.0f;
        bool peelAfter = enemy->playstyle == EnemyPlaystyle::STALKER;
        FireEnemyLaser(enemy, enemy->pendingFireDirection, peelAfter);
    }
}

static void StartFiringRun(Enemy* enemy) {
    enemy->commitTimer = FIRING_RUN_DURATION + (float)GetRandomValue(0, 15) / 10.0f;
    enemy->runHeading = enemy->direction;
}

static void StartPursue(Enemy* enemy) {
    enemy->commitTimer = PURSUE_DURATION + (float)GetRandomValue(0, 12) / 10.0f;
}

static void UpdateStalkerAI(Enemy* enemy, float deltaTime, float distance, float angleToPlayer) {
    if (enemy->telegraphTimer > 0.0f) return;

    enemy->timeSinceLastShot += deltaTime;

    if (enemy->repositionTimer > 0.0f) {
        UpdateReposition(enemy, deltaTime, angleToPlayer);
        return;
    }

    float desiredAngle = angleToPlayer;

    if (distance < 200.0f) {
        float breakAmount = 70.0f + 40.0f * (1.0f - distance / 200.0f);
        desiredAngle = NormalizeAngle(angleToPlayer + enemy->breakDirection * breakAmount);
    } else if (distance < 340.0f) {
        desiredAngle = NormalizeAngle(angleToPlayer + enemy->breakDirection * 50.0f);
    } else {
        float playerRad = (player.direction - 90.0f) * DEG2RAD;
        Vector2 leadPos = {
            player.position.x + cosf(playerRad) * player.speed * 0.25f,
            player.position.y + sinf(playerRad) * player.speed * 0.25f
        };
        desiredAngle = atan2f(leadPos.y - enemy->position.y, leadPos.x - enemy->position.x) * RAD2DEG + 90.0f;
        desiredAngle = NormalizeAngle(desiredAngle);
    }

    MoveEnemy(enemy, desiredAngle, deltaTime);

    float aimDiff = fabsf(AngleDifference(enemy->direction, angleToPlayer));
    float angleDiff = AngleDifference(enemy->direction, desiredAngle);
    float aimPenalty = fminf(fabsf(angleDiff) / 30.0f, 1.0f);
    if (aimDiff < 24.0f && distance < 400.0f && distance > 120.0f &&
        enemy->timeSinceLastShot >= enemy->ship.shotCooldown * 1.5f &&
        GetRandomValue(0, 100) > (int)(20.0f + aimPenalty * 30.0f)) {
        float fireDirection = enemy->direction + (float)GetRandomValue(-12, 12);
        TryStartTelegraph(enemy, fireDirection);
    }
}

static void UpdateFiringRunAI(Enemy* enemy, float deltaTime, float distance, float angleToPlayer) {
    enemy->timeSinceLastShot += deltaTime;

    if (enemy->repositionTimer > 0.0f) {
        UpdateReposition(enemy, deltaTime, angleToPlayer);
        return;
    }

    if (enemy->commitTimer > 0.0f) {
        enemy->commitTimer -= deltaTime;
        MoveEnemyStraight(enemy, deltaTime, 1.1f);

        Vector2 toPlayer = Vector2Subtract(player.position, enemy->position);
        Vector2 runForward = EnemyForwardVector(enemy->runHeading);
        bool playerStillAhead = Vector2DotProduct(runForward, Vector2Normalize(toPlayer)) > 0.2f;

        if (playerStillAhead && enemy->timeSinceLastShot >= enemy->ship.shotCooldown * 1.1f) {
            FireEnemyLaser(enemy, enemy->runHeading + (float)GetRandomValue(-8, 8), false);
        } else if (!playerStillAhead && enemy->commitTimer > 1.0f) {
            enemy->commitTimer = 1.0f;
        }

        if (enemy->commitTimer <= 0.0f) {
            EndCommitPhase(enemy);
        }
        return;
    }

    float desiredAngle = angleToPlayer;
    if (distance > 280.0f) {
        float playerRad = (player.direction - 90.0f) * DEG2RAD;
        Vector2 leadPos = {
            player.position.x + cosf(playerRad) * player.speed * 0.35f,
            player.position.y + sinf(playerRad) * player.speed * 0.35f
        };
        desiredAngle = atan2f(leadPos.y - enemy->position.y, leadPos.x - enemy->position.x) * RAD2DEG + 90.0f;
        desiredAngle = NormalizeAngle(desiredAngle);
    }

    MoveEnemy(enemy, desiredAngle, deltaTime, 0.95f);

    float aimDiff = fabsf(AngleDifference(enemy->direction, angleToPlayer));
    if (aimDiff < 18.0f && distance < 520.0f && distance > 100.0f) {
        StartFiringRun(enemy);
    }
}

static void UpdatePursueAI(Enemy* enemy, float deltaTime, float distance, float angleToPlayer) {
    enemy->timeSinceLastShot += deltaTime;

    if (enemy->repositionTimer > 0.0f) {
        UpdateReposition(enemy, deltaTime, angleToPlayer);
        return;
    }

    if (enemy->commitTimer > 0.0f) {
        enemy->commitTimer -= deltaTime;

        float playerRad = (player.direction - 90.0f) * DEG2RAD;
        Vector2 leadPos = {
            player.position.x + cosf(playerRad) * player.speed * 0.2f,
            player.position.y + sinf(playerRad) * player.speed * 0.2f
        };
        float desiredAngle = atan2f(leadPos.y - enemy->position.y, leadPos.x - enemy->position.x) * RAD2DEG + 90.0f;
        desiredAngle = NormalizeAngle(desiredAngle);
        MoveEnemy(enemy, desiredAngle, deltaTime, 1.05f);

        float aimDiff = fabsf(AngleDifference(enemy->direction, angleToPlayer));
        if (enemy->telegraphTimer <= 0.0f &&
            aimDiff < 32.0f && distance < 280.0f && distance > 70.0f &&
            enemy->timeSinceLastShot >= enemy->ship.shotCooldown * 1.35f &&
            GetRandomValue(0, 100) > 35) {
            float fireDirection = enemy->direction + (float)GetRandomValue(-10, 10);
            TryStartTelegraph(enemy, fireDirection);
        }

        if (enemy->commitTimer <= 0.0f) {
            EndCommitPhase(enemy);
        }
        return;
    }

    float flankOffset = 35.0f;
    if (distance < 320.0f) {
        flankOffset = fmaxf(0.0f, 35.0f * (distance - 130.0f) / 190.0f);
    }
    float desiredAngle = NormalizeAngle(angleToPlayer + enemy->breakDirection * flankOffset);
    MoveEnemy(enemy, desiredAngle, deltaTime, 0.85f);

    if (distance < 300.0f && distance > 130.0f && GetRandomValue(0, 100) > 50) {
        StartPursue(enemy);
    }
}

static void UpdateBossGunnerAI(Enemy* enemy, float deltaTime, float distance, float angleToPlayer) {
    enemy->timeSinceLastShot += deltaTime;

    if (enemy->repositionTimer > 0.0f) {
        UpdateReposition(enemy, deltaTime, angleToPlayer);
        return;
    }

    float desiredAngle = angleToPlayer;
    float thrustScale = 0.9f;

    if (distance < 220.0f) {
        float breakAmount = 95.0f + 35.0f * (1.0f - distance / 220.0f);
        desiredAngle = NormalizeAngle(angleToPlayer + enemy->breakDirection * breakAmount);
        thrustScale = 1.15f;
    } else if (distance > 390.0f) {
        float playerRad = (player.direction - 90.0f) * DEG2RAD;
        Vector2 leadPos = {
            player.position.x + cosf(playerRad) * player.speed * 0.35f,
            player.position.y + sinf(playerRad) * player.speed * 0.35f
        };
        desiredAngle = atan2f(leadPos.y - enemy->position.y, leadPos.x - enemy->position.x) * RAD2DEG + 90.0f;
        desiredAngle = NormalizeAngle(desiredAngle);
        thrustScale = 1.05f;
    } else {
        float orbitAmount = 62.0f + 18.0f * sinf((float)GetTime() * 1.4f);
        desiredAngle = NormalizeAngle(angleToPlayer + enemy->breakDirection * orbitAmount);
    }

    MoveEnemy(enemy, desiredAngle, deltaTime, thrustScale);

    float aimDiff = fabsf(AngleDifference(enemy->direction, angleToPlayer));
    if (aimDiff < 20.0f && distance > 190.0f && distance < 430.0f &&
        enemy->timeSinceLastShot >= enemy->ship.shotCooldown * 1.1f) {
        float fireDirection = enemy->direction + (float)GetRandomValue(-6, 6);
        FireEnemyLaser(enemy, fireDirection, false);
        FireEnemyLaser(enemy, fireDirection + (float)GetRandomValue(-4, 4), true);
    }
}

static void UpdateBossBladeAI(Enemy* enemy, float deltaTime, float distance, float angleToPlayer) {
    enemy->timeSinceLastShot += deltaTime;

    if (enemy->repositionTimer > 0.0f) {
        UpdateReposition(enemy, deltaTime, angleToPlayer);
        return;
    }

    if (enemy->commitTimer > 0.0f) {
        enemy->commitTimer -= deltaTime;

        float playerRad = (player.direction - 90.0f) * DEG2RAD;
        Vector2 leadPos = {
            player.position.x + cosf(playerRad) * player.speed * 0.15f,
            player.position.y + sinf(playerRad) * player.speed * 0.15f
        };
        float chargeAngle = atan2f(leadPos.y - enemy->position.y, leadPos.x - enemy->position.x) * RAD2DEG + 90.0f;
        chargeAngle = NormalizeAngle(chargeAngle);
        enemy->runHeading = chargeAngle;
        MoveEnemyStraight(enemy, deltaTime, 1.4f);

        float aimDiff = fabsf(AngleDifference(enemy->direction, angleToPlayer));
        if (distance < 190.0f && aimDiff < 36.0f &&
            enemy->timeSinceLastShot >= enemy->ship.shotCooldown * 0.85f) {
            FireEnemyLaser(enemy, enemy->direction + (float)GetRandomValue(-10, 10), false);
        }

        if (enemy->commitTimer <= 0.0f) {
            enemy->repositionTimer = 1.2f + (float)GetRandomValue(0, 4) / 10.0f;
        }
        return;
    }

    float desiredAngle = angleToPlayer;
    float thrustScale = 1.1f;
    if (distance > 260.0f) {
        float playerRad = (player.direction - 90.0f) * DEG2RAD;
        Vector2 leadPos = {
            player.position.x + cosf(playerRad) * player.speed * 0.3f,
            player.position.y + sinf(playerRad) * player.speed * 0.3f
        };
        desiredAngle = atan2f(leadPos.y - enemy->position.y, leadPos.x - enemy->position.x) * RAD2DEG + 90.0f;
        desiredAngle = NormalizeAngle(desiredAngle);
    } else if (distance > 150.0f) {
        desiredAngle = NormalizeAngle(angleToPlayer + enemy->breakDirection * 18.0f);
    }

    MoveEnemy(enemy, desiredAngle, deltaTime, thrustScale);

    float aimDiff = fabsf(AngleDifference(enemy->direction, angleToPlayer));
    if (distance < 230.0f && aimDiff < 28.0f) {
        enemy->commitTimer = 2.8f + (float)GetRandomValue(0, 8) / 10.0f;
        enemy->runHeading = enemy->direction;
    }
}

static void UpdateBossStationAI(Enemy* enemy, float deltaTime, float distance, float angleToPlayer) {
    enemy->timeSinceLastShot += deltaTime;
    enemy->thrustIntensity = 0.2f;
    enemy->thrustBoost = 0.4f;

    float driftX = sinf((float)GetTime() * 0.35f) * 18.0f;
    float anchorY = gameHeight * 0.14f + sinf((float)GetTime() * 0.22f) * 6.0f;
    enemy->position.x = gameWidth / 2.0f + driftX;
    enemy->position.y = anchorY;
    enemy->direction = angleToPlayer;

    if (enemy->telegraphTimer > 0.0f) return;

    if (distance < 920.0f && enemy->timeSinceLastShot >= enemy->ship.shotCooldown) {
        enemy->telegraphTimer = 0.65f;
        enemy->pendingFireDirection = angleToPlayer;
    }
}

static void UpdateEnemyAI(Enemy* enemy, float deltaTime) {
    Vector2 toPlayer = Vector2Subtract(player.position, enemy->position);
    float distance = Vector2Length(toPlayer);
    float angleToPlayer = atan2f(toPlayer.y, toPlayer.x) * RAD2DEG + 90.0f;
    angleToPlayer = NormalizeAngle(angleToPlayer);

    if (enemy->playstyle == EnemyPlaystyle::BOSS_STATION) {
        UpdateBossStationAI(enemy, deltaTime, distance, angleToPlayer);
    } else if (enemy->playstyle == EnemyPlaystyle::BOSS_GUNNER) {
        UpdateBossGunnerAI(enemy, deltaTime, distance, angleToPlayer);
    } else if (enemy->playstyle == EnemyPlaystyle::BOSS_BLADE) {
        UpdateBossBladeAI(enemy, deltaTime, distance, angleToPlayer);
    } else if (enemy->playstyle == EnemyPlaystyle::FIRING_RUN) {
        UpdateFiringRunAI(enemy, deltaTime, distance, angleToPlayer);
    } else if (enemy->playstyle == EnemyPlaystyle::PURSUE) {
        UpdatePursueAI(enemy, deltaTime, distance, angleToPlayer);
    } else {
        UpdateStalkerAI(enemy, deltaTime, distance, angleToPlayer);
    }
}

static void HandleEnemyDeath(Enemy& enemy) {
    if (enemy.tier == EnemyTier::DEATH_STAR) {
        SpawnExplosion(enemy.position, LARGE, false);
        SpawnFloatingText(
            { gameWidth / 2.0f, gameHeight * 0.32f },
            "DEATH STAR DESTROYED!",
            (Color){ 120, 255, 160, 255 }
        );
        game.TriggerVictory("The Death Star has been destroyed!");
    } else {
        SpawnShipExplosion(enemy.position);
    }

    float driftRad = (enemy.direction - 90.0f) * DEG2RAD;
    Vector2 driftVelocity = {
        cosf(driftRad) * enemy.ship.speed * 0.4f,
        sinf(driftRad) * enemy.ship.speed * 0.4f
    };
    SpawnHealthPickup(enemy.position, enemy.healthDrop, driftVelocity);
    SpawnScrapPickup(enemy.position, GetEnemyScrapDrop(enemy.tier), driftVelocity);
    enemy.active = false;
    currentRun.score += 1;
    totalShipsDestroyed++;
    int shipIdx = player.starshipType;
    if (shipIdx >= 0 && shipIdx < MAX_SHIPS) {
        shipKills[shipIdx]++;
    }
}

static void CheckLaserEnemyCollisions() {
    for (int i = 0; i < (int)activeLasers.size(); ) {
        Laser* laser = activeLasers[i];
        if (laser->fromEnemy) {
            ++i;
            continue;
        }

        bool laserHit = false;
        for (int j = 0; j < MAX_ENEMIES; j++) {
            Enemy& enemy = enemies[j];
            if (!enemy.active) continue;

            if (CheckCollisionPointCircle(laser->position, enemy.position, enemy.radius)) {
                float damage = (float)laser->damage;
                if (IsTailshot(enemy, *laser)) {
                    damage *= runMods.tailshotMult;
                    SpawnFloatingText(enemy.position, "Tailshot!", (Color){ 80, 255, 120, 255 });
                }

                enemy.health -= damage;
                enemy.hitFlashTimer = 0.15f;
                SpawnLaserImpact(laser->position);

                if (enemy.health <= 0.0f) {
                    HandleEnemyDeath(enemy);
                }

                laserHit = true;
                break;
            }
        }

        if (laserHit) {
            delete laser;
            activeLasers.erase(activeLasers.begin() + i);
        } else {
            ++i;
        }
    }
}

static void CheckEnemyLaserPlayerCollisions(float deltaTime) {
    player.damageCooldown -= deltaTime;
    if (player.damageCooldown < 0.0f) player.damageCooldown = 0.0f;

    for (int i = 0; i < (int)activeLasers.size(); ) {
        Laser* laser = activeLasers[i];
        if (!laser->fromEnemy) {
            ++i;
            continue;
        }

        if (CheckCollisionPointCircle(laser->position, player.position, player.radius)) {
            float reflectRoll = (float)GetRandomValue(0, 9999) / 10000.0f;
            if (reflectRoll < GetEffectiveReflectChance()) {
                float reflectDeg = atan2f(laser->direction.y, laser->direction.x) * RAD2DEG + 90.0f + 180.0f;
                FireSingleLaser(
                    player.position,
                    reflectDeg,
                    player.shotSpeed,
                    player.ship.laserSize,
                    player.ship.laserOffsets,
                    GetEffectiveDamage(),
                    false
                );
                SpawnFloatingText(player.position, "Reflect!", (Color){ 120, 200, 255, 255 });
                delete laser;
                activeLasers.erase(activeLasers.begin() + i);
                continue;
            }

            if (player.damageCooldown <= 0.0f) {
                player.life -= GetEffectiveLaserDamageTaken((float)laser->damage);
                player.damageCooldown = 0.6f;
                SpawnLaserImpact(laser->position);
                TriggerPlayerHitShake((float)laser->damage);
            }
            delete laser;
            activeLasers.erase(activeLasers.begin() + i);
        } else {
            ++i;
        }
    }
}

static void DestroyEnemyFromCollision(Enemy& enemy) {
    HandleEnemyDeath(enemy);
}

static float GetTierCollisionMultiplier(EnemyTier tier) {
    switch (tier) {
        case EnemyTier::RARE: return 1.1f;
        case EnemyTier::ELITE: return 1.2f;
        case EnemyTier::FALCON: return 1.35f;
        case EnemyTier::DEATH_STAR: return 0.6f;
        default: return 1.0f;
    }
}

static float GetPlayerApproachSpeed() {
    float moveSpeed = player.speed;
    if (player.isBoosting) moveSpeed *= player.boost;
    else if (player.isBraking) moveSpeed *= 0.5f;
    return fmaxf(moveSpeed, Vector2Length(player.velocity));
}

static float GetEnemyApproachSpeed(const Enemy& enemy) {
    return enemy.ship.speed * fmaxf(enemy.thrustBoost, 1.0f);
}

static void CheckEnemyPlayerCollisions() {
    const float minRamSpeed = 35.0f;
    float playerApproach = GetPlayerApproachSpeed();

    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy& enemy = enemies[i];
        if (!enemy.active) continue;

        float totalRadius = enemy.radius + player.radius;
        if (Vector2Distance(enemy.position, player.position) >= totalRadius) continue;

        Vector2 collisionNormal = Vector2Normalize(Vector2Subtract(player.position, enemy.position));
        player.velocity = Vector2Add(player.velocity, Vector2Scale(collisionNormal, 40.0f));

        float tierMult = GetTierCollisionMultiplier(enemy.tier);
        float enemyApproach = GetEnemyApproachSpeed(enemy);

        if (player.damageCooldown <= 0.0f) {
            float baseDamage = enemy.ship.ComputeMeleeDamage(enemyApproach, tierMult);
            float damageMult = player.ship.GetPlayerCollisionDamageMultiplier(
                player.direction, player.position, enemy.position);
            float damage = baseDamage * damageMult;
            if (damage > 0.0f) {
                player.life -= GetEffectiveMeleeDamageTaken(damage);
                player.damageCooldown = 0.8f;
                TriggerPlayerHitShake(damage);
            }
        }

        if (enemy.collisionDamageCooldown <= 0.0f && playerApproach >= minRamSpeed) {
            float ramDamage = player.ship.ComputeMeleeDamage(playerApproach, 1.0f);
            ramDamage = GetEffectiveOutgoingMeleeDamage(ramDamage);
            ramDamage *= enemy.ship.GetEnemyCollisionDamageMultiplier(
                enemy.direction, enemy.position, player.position);
            if (ramDamage > 0.0f) {
                enemy.health -= ramDamage;
                enemy.hitFlashTimer = 0.15f;
                enemy.collisionDamageCooldown = 0.8f;
                SpawnLaserImpact(enemy.position);

                if (enemy.health <= 0.0f) {
                    DestroyEnemyFromCollision(enemy);
                }
            }
        }
    }
}

static void DrawEnemyTelegraph(const Enemy& enemy) {
    if (enemy.playstyle == EnemyPlaystyle::FIRING_RUN) return;
    if (enemy.telegraphTimer <= 0.0f) return;

    float progress = 1.0f - (enemy.telegraphTimer / TELEGRAPH_DURATION);
    Vector2 forward = EnemyForwardVector(enemy.pendingFireDirection);
    Vector2 muzzle = Vector2Add(enemy.position, Vector2Scale(forward, 18.0f));
    Vector2 end = Vector2Add(muzzle, Vector2Scale(forward, 70.0f + progress * 40.0f));
    Color telegraphColor = (Color){ 255, 90, 40, (unsigned char)(120 + progress * 135) };

    DrawLineEx(muzzle, end, 2.0f + progress * 2.0f, telegraphColor);
    DrawCircleV(muzzle, 4.0f + progress * 3.0f, Fade(ORANGE, 0.35f + progress * 0.45f));
}

static void DrawSpawnPortal(float edgeX, float centerY, bool fromLeft, float progress, float pulse) {
    float alpha = (0.25f + progress * 0.7f) * pulse;
    float glowAlpha = alpha * 0.5f;
    float portalHeight = 55.0f + progress * 35.0f;
    float halfH = portalHeight * 0.5f;
    float topY = centerY - halfH;
    float botY = centerY + halfH;

    Color coreColor = fromLeft ? (Color){ 200, 240, 255, 255 } : (Color){ 255, 220, 170, 255 };
    Color glowColor = fromLeft ? (Color){ 70, 150, 255, 255 } : (Color){ 255, 110, 50, 255 };

    float rimX = edgeX;
    float glowDepth = 30.0f + progress * 37.5f;

    Rectangle glowRect = fromLeft
        ? Rectangle{ rimX, topY - 6.0f, glowDepth, portalHeight + 12.0f }
        : Rectangle{ rimX - glowDepth, topY - 6.0f, glowDepth, portalHeight + 12.0f };
    DrawRectangleGradientH(
        (int)glowRect.x, (int)glowRect.y, (int)glowRect.width, (int)glowRect.height,
        fromLeft ? Fade(glowColor, glowAlpha) : Fade(BLANK, 0.0f),
        fromLeft ? Fade(BLANK, 0.0f) : Fade(glowColor, glowAlpha)
    );

    float pipeWidth = 2.5f + progress * 2.0f;
    Rectangle pipeRect = fromLeft
        ? Rectangle{ rimX, topY, pipeWidth, portalHeight }
        : Rectangle{ rimX - pipeWidth, topY, pipeWidth, portalHeight };
    DrawRectangleGradientV(
        (int)pipeRect.x, (int)pipeRect.y, (int)pipeRect.width, (int)pipeRect.height,
        Fade(coreColor, alpha * 0.55f),
        Fade(glowColor, alpha * 0.75f)
    );

    float seamX = fromLeft ? rimX + pipeWidth * 0.5f : rimX - pipeWidth * 0.5f;
    DrawLineEx({ seamX, topY }, { seamX, botY }, 1.0f + progress * 0.75f, Fade(WHITE, alpha * 0.95f));
}

static void DrawTopSpawnPortal(float centerX, float centerY, float progress, float pulse) {
    float alpha = (0.3f + progress * 0.65f) * pulse;
    float portalWidth = 120.0f + progress * 80.0f;
    float portalHeight = 40.0f + progress * 25.0f;
    float halfW = portalWidth * 0.5f;
    float halfH = portalHeight * 0.5f;

    Color coreColor = (Color){ 180, 220, 255, 255 };
    Color glowColor = (Color){ 90, 140, 220, 255 };

    Rectangle glowRect = { centerX - halfW - 10.0f, centerY - halfH - 14.0f, portalWidth + 20.0f, portalHeight + 28.0f };
    DrawRectangleRounded(glowRect, 0.35f, 8, Fade(glowColor, alpha * 0.35f));

    Rectangle coreRect = { centerX - halfW, centerY - halfH, portalWidth, portalHeight };
    DrawRectangleRounded(coreRect, 0.25f, 6, Fade(coreColor, alpha * 0.55f));
    DrawRectangleRoundedLines(coreRect, 0.25f, 6, Fade(WHITE, alpha * 0.85f));
}

static void DrawSpawnWarning() {
    if (!pendingSpawn.active) return;

    float progress = 1.0f - (pendingSpawn.timer / pendingSpawn.warningDuration);
    float pulse = 0.6f + 0.4f * sinf((float)GetTime() * 7.0f);

    if (pendingSpawn.spawnAtTopCenter) {
        DrawTopSpawnPortal(gameWidth / 2.0f, pendingSpawn.spawnY, progress, pulse);
    } else {
        float edgeX = pendingSpawn.spawnLeft ? 0.0f : (float)gameWidth;
        DrawSpawnPortal(edgeX, pendingSpawn.spawnY, pendingSpawn.spawnLeft, progress, pulse);
    }

    if (!pendingSpawn.isBossSpawn) return;

    const char* bossLabel = pendingSpawn.choice.ship->name.c_str();
    const char* subtitle = pendingSpawn.spawnAtTopCenter ? "FINAL BOSS INCOMING" : "BOSS INCOMING";
    int titleSize = 30;
    int subtitleSize = 18;
    float bannerAlpha = 0.55f + progress * 0.45f;
    Color titleColor = pendingSpawn.choice.ship == &DeathStarShip
        ? (Color){ 140, 200, 255, 255 }
        : (pendingSpawn.choice.ship == &FalconShip
            ? (Color){ 255, 120, 70, 255 }
            : (Color){ 255, 80, 120, 255 });
    Color subtitleColor = (Color){ 255, 220, 120, 255 };

    int titleWidth = MeasureText(bossLabel, titleSize);
    int subtitleWidth = MeasureText(subtitle, subtitleSize);
    int bannerWidth = titleWidth > subtitleWidth ? titleWidth : subtitleWidth;
    bannerWidth += 48;
    int bannerHeight = 72;
    int bannerX = gameWidth / 2 - bannerWidth / 2;
    int bannerY = (int)(gameHeight * 0.1f);

    DrawRectangleRounded(
        { (float)bannerX, (float)bannerY, (float)bannerWidth, (float)bannerHeight },
        0.2f, 6, Fade((Color){ 20, 12, 28, 255 }, bannerAlpha * 0.9f)
    );
    DrawRectangleRoundedLines(
        { (float)bannerX, (float)bannerY, (float)bannerWidth, (float)bannerHeight },
        0.2f, 6, Fade(titleColor, bannerAlpha)
    );

    DrawText(
        subtitle,
        gameWidth / 2 - subtitleWidth / 2,
        bannerY + 10,
        subtitleSize,
        Fade(subtitleColor, bannerAlpha)
    );
    DrawText(
        bossLabel,
        gameWidth / 2 - titleWidth / 2,
        bannerY + 34,
        titleSize,
        Fade(titleColor, bannerAlpha)
    );
}

static void DrawFiringRunIndicator(const Enemy& enemy) {
    if (enemy.playstyle != EnemyPlaystyle::FIRING_RUN) return;
    if (enemy.commitTimer <= 0.0f) return;

    Vector2 forward = EnemyForwardVector(enemy.runHeading);
    Vector2 tail = Vector2Subtract(enemy.position, Vector2Scale(forward, 24.0f));
    Vector2 tip = Vector2Add(enemy.position, Vector2Scale(forward, 90.0f));
    DrawLineEx(tail, tip, 2.0f, Fade((Color){ 255, 160, 60, 255 }, 0.55f));
}

static void DrawPursueIndicator(const Enemy& enemy) {
    if (enemy.playstyle != EnemyPlaystyle::PURSUE) return;
    if (enemy.commitTimer <= 0.0f) return;

    DrawCircleLines((int)enemy.position.x, (int)enemy.position.y, enemy.radius + 6, Fade(RED, 0.45f));
}

void EnemyUpdateLoop(float deltaTime) {
    if (!enemiesOn || asteroidsMode) return;

    if (debugging && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        SpawnEnemy();
    }

    EnemySpawnTimer(deltaTime);
    UpdatePendingSpawn(deltaTime);
    UpdatePickups(deltaTime);
    CheckLaserEnemyCollisions();
    CheckEnemyLaserPlayerCollisions(deltaTime);
    CheckEnemyPlayerCollisions();

    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* enemy = &enemies[i];
        if (!enemy->active) continue;

        if (enemy->hitFlashTimer > 0.0f) {
            enemy->hitFlashTimer -= deltaTime;
            if (enemy->hitFlashTimer < 0.0f) enemy->hitFlashTimer = 0.0f;
        }

        if (enemy->collisionDamageCooldown > 0.0f) {
            enemy->collisionDamageCooldown -= deltaTime;
            if (enemy->collisionDamageCooldown < 0.0f) enemy->collisionDamageCooldown = 0.0f;
        }

        UpdateEnemyTelegraph(enemy, deltaTime);
        enemy->thrustIntensity = 0.55f;
        enemy->thrustBoost = 1.0f;
        UpdateEnemyAI(enemy, deltaTime);

        float deltaDir = AngleDifference(enemy->prevDirection, enemy->direction);
        enemy->rotationRate = deltaDir / deltaTime;
        enemy->prevDirection = enemy->direction;

        if (enemy->enteringScreen) {
            if (enemy->position.x >= 0.0f && enemy->position.x <= (float)gameWidth) {
                enemy->enteringScreen = false;
            }
        } else if (enemy->tier != EnemyTier::DEATH_STAR) {
            enemy->position = WrapPosition(enemy->position, gameWidth, gameHeight);
        }

        EngineExhaustParams exhaust = {
            .position = enemy->position,
            .direction = enemy->direction,
            .scale = enemy->ship.scale,
            .intensity = enemy->thrustIntensity,
            .boostFactor = enemy->thrustBoost,
            .turnRate = enemy->rotationRate,
            .neonColors = false,
            .ionColors = false,
            .time = (float)GetTime()
        };
        EmitEngineExhaust(exhaust);
    }
}

void EnemyDrawAll() {
    DrawSpawnWarning();

    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy& enemy = enemies[i];
        if (!enemy.active) continue;

        if (enemyDebugging) {
            DrawFiringRunIndicator(enemy);
            DrawPursueIndicator(enemy);
            DrawEnemyTelegraph(enemy);
        }

        Texture2D texture = enemy.ship.texture;
        float scale = enemy.ship.scale;
        Rectangle sourceRec = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
        Rectangle destRec = {
            enemy.position.x,
            enemy.position.y,
            texture.width * scale,
            texture.height * scale
        };
        Vector2 origin = { (texture.width * scale) / 2.0f, (texture.height * scale) / 2.0f };

        Color tint = WHITE;
        if (enemy.hitFlashTimer > 0.0f) {
            float flashStrength = enemy.hitFlashTimer / 0.15f;
            tint = Fade(WHITE, 0.4f + flashStrength * 0.6f);
        }

        DrawTexturePro(texture, sourceRec, destRec, origin, enemy.direction, tint);

        if (enemy.maxHealth > GRUNT_HEALTH + 0.5f) {
            float barWidth = enemy.tier == EnemyTier::DEATH_STAR ? 96.0f
                : (enemy.tier == EnemyTier::FALCON ? 48.0f : 36.0f);
            float barHeight = enemy.tier == EnemyTier::DEATH_STAR ? 7.0f
                : (enemy.tier == EnemyTier::FALCON ? 5.0f : 4.0f);
            float healthRatio = enemy.health / enemy.maxHealth;
            float barX = enemy.position.x - barWidth / 2.0f;
            float barY = enemy.position.y - origin.y - (enemy.tier == EnemyTier::DEATH_STAR ? 18.0f : 12.0f);
            DrawRectangle((int)barX, (int)barY, (int)barWidth, (int)barHeight, (Color){ 30, 30, 30, 180 });
            Color barColor = enemy.tier == EnemyTier::DEATH_STAR
                ? (Color){ 120, 200, 255, 255 }
                : (enemy.tier == EnemyTier::FALCON
                    ? (Color){ 255, 90, 50, 255 }
                    : (healthRatio > 0.5f ? MAROON : (Color){ 200, 60, 40, 255 }));
            DrawRectangle((int)barX, (int)barY, (int)(barWidth * healthRatio), (int)barHeight, barColor);
        }
    }

    DrawPickups();
}
