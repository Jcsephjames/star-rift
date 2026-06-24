#include "raylib.h"
#include <vector>
#include <stdio.h>
#include <cstring>

#include "game.hpp"
#include "globals.hpp"

#include "asteroids.hpp"
#include "enemies.hpp"
#include "saveData.hpp"
#include "effects.hpp"
#include "laserManager.hpp"
#include "scrap.hpp"
#include "cardDraft.hpp"
#include "pauseMenu.hpp"
#include "runModifiers.hpp"
#include "metaProgression.hpp"
#include "engineExhaust.hpp"
#include "audio.hpp"
#include "runSummary.hpp"
#include "runHistory.hpp"

Game::Game(){
}
void Game::InitGame()
{
    // Initialize player
    InitEffects();
    InitEngineExhaust();
    InitScrapPickups();
    InitRunModifiers();
    InitCardDraft();
    InitPauseMenu();
    InitRunSummarySnapshot();
    game.cardDraftOpen = false;
    game.runCreditsFinalized = false;
    lastRunCreditsEarned = 0;
    player.InitPlayer();
    
    // General game init
    gameOver = false;
    victory = false;
    endlessMode = false;
    victoryMessage[0] = '\0';
    returnToMainMenu = false;

    currentRun.score = 0;
    currentRun.scrap = 0;
    currentRun.time = 0.0f;

    if (asteroidsOn) {
        InitAsteroids();
    }
    if (enemiesOn) {
        InitEnemies();
    }
}
void Game::Update()
{
    float deltaTime = GetFrameTime();

    if (cardDraftOpen) {
        UpdateCardDraft();
        UpdateEngineExhaust(deltaTime);
        return;
    }

    if (pauseMenuOpen) {
        UpdatePauseMenu();
        return;
    }

    if (gameOver) {
        UpdateRunSummary();
        return;
    }

    if (IsKeyPressed(KEY_P)) {
        OpenPauseMenu();
        return;
    }

    // GAME UPDATES
    TimerLogic(deltaTime);

    if (asteroidsOn){
        AsteroidUpdateLoop();
    }
    if (enemiesOn) {
        EnemyUpdateLoop(deltaTime);
    }
    if (asteroidsOn || enemiesOn) {
        UpdateEffects(deltaTime);
        UpdateScrapPickups(deltaTime);
    }

    // PLAYER UPDATE
    player.Update(deltaTime);

    // Wrap player position around the screen Or deal with Player going below the screen
    player.position = WrapPosition(player.position, gameWidth, gameHeight);
    player.bellowY = (player.position.y > gameHeight + 50.0f) ? true : false;

    UpdateEngineExhaust(deltaTime);


    // BACKGROUND UPDATE
    stars.Update(deltaTime);

    if (!gameOver) {
        UpdateShipStats(deltaTime);
    }
    if (!gameOver) {
        UpdateHighScores();
    }
}

void Game::Draw() {

    // GAME OVER DRAW and Logic
    Game::GameOver();

    Camera2D shakeCamera = GetShakeCamera();
    BeginMode2D(shakeCamera);

    // BACKGROUND DRAW
    stars.Draw();

    DrawAllEngineExhaust();

    if (asteroidsOn) {
        AsteroidDrawAll();
    }
    if (enemiesOn) {
        EnemyDrawAll();
    }
    if (asteroidsOn || enemiesOn) {
        DrawEffects();
        DrawScrapPickups();
    }

    // PLAYER DRAW
    player.Draw(gameWidth, gameHeight);
    DrawLasers();

    EndMode2D();

    
    // OVERLAY DRAW
    DrawGameOverlay(player, player.life/ player.maxHealth, player.ship.boostTimer / player.ship.maxBoost, player.ship.boostStaller,
                    currentRun.score, currentRun.scrap, currentRun.time);

    if (cardDraftOpen) {
        DrawCardDraft();
    } else if (pauseMenuOpen) {
        DrawPauseMenu();
    } else if (enemiesOn && !asteroidsMode && !gameOver) {
        DrawActiveBuildSummary(10, 130);
    }

    if (gameOver) {
        DrawRunSummary();
    }

    if (debugging){
        DrawText("Game Started!", 5, 5, 15, GREEN);
    }
}

void Game::GameOver()
{
    if (player.life <= 0) {
        if (!gameOver) {
            if (endlessMode && victory) {
                UpdateLatestRiftRunHistory(
                    GetEnemyWave(),
                    currentRun.score,
                    currentRun.scrap,
                    currentRun.time
                );
                CaptureRunSummaryRecords();
                UpdateHighScores();
                SaveGameData();
            } else {
                FinalizeRunCredits();
                CaptureRunSummaryRecords();
            }
            PlaySfx(SfxId::GAME_OVER);
        }
        gameOver = true;
        return;
    }
}

void Game::TriggerVictory(const char* message)
{
    if (gameOver || victory) return;

    if (message && message[0] != '\0') {
        strncpy(victoryMessage, message, sizeof(victoryMessage) - 1);
        victoryMessage[sizeof(victoryMessage) - 1] = '\0';
    } else {
        strncpy(victoryMessage, "The Death Star has been destroyed!", sizeof(victoryMessage) - 1);
        victoryMessage[sizeof(victoryMessage) - 1] = '\0';
    }

    FinalizeRunCredits();
    CaptureRunSummaryRecords();
    PlaySfx(SfxId::EXPLOSION_LARGE);
    victory = true;
    gameOver = true;
}

void Game::ContinueEndless()
{
    if (!victory || !gameOver || endlessMode) return;
    if (asteroidsMode || !enemiesOn) return;

    endlessMode = true;
    gameOver = false;
    ContinueEndlessRift();
    PlaySfx(SfxId::WAVE_ANNOUNCE);
}

void Game::TimerLogic(float deltaTime)
{
    currentRun.time += deltaTime;
    if (asteroidsMode) {
        CheckAsteroidsVictory();
    }
    if (debugging) {
        if (time >= 10.0f) {
            printf("Game time: %.2f seconds\n", time);
        }
    }
}