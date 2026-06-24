#include <raylib.h>
#include <cstring>
#include "globals.hpp"
#include "mainMenu.hpp"
#include "textures.hpp"
#include "settings.hpp"
#include "shipSelection.hpp"
#include "collections.hpp"
#include "hangar.hpp"
#include "runHistoryScreen.hpp"
#include "saveData.hpp"
#include "metaProgression.hpp"
#include "enemies.hpp"
#include "audio.hpp"
#include "difficulty.hpp"

// #include "player.hpp"
// #include "laser.hpp"
// #include "game.hpp"
// #include "background.hpp"
// #include "ship.hpp"
// #include "laserManager.hpp"
// #include "asteroids.hpp"

int main(int argc, char* argv[]) {
    bool devMode = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--dev") == 0) {
            devMode = true;
        }
    }

    // Initialization
    InitWindow(screenWidth, screenHeight, "Star Rift");
    SetTargetFPS(60);
    InitAudio();

    LoadAllTextures();
    InitShips();
    LoadGameData();
    if (devMode) {
        debugging = true;
    }
    player.SetStarshipType(starshipType);

    enum GameScreen { MAIN_MENU, SHIP_SELECTION, GAME, ASTEROIDS, COLLECTIONS, HANGAR, RUN_HISTORY, SETTINGS };

    GameScreen currentScreen = MAIN_MENU;
    MainMenu mainMenu;
    Settings settings;
    Collections collections;
    Hangar hangar;
    RunHistoryScreen runHistoryScreen;
    shipSelection.InitShips();

    // Main Game Loop
    while (!WindowShouldClose()) {
        UpdateAudio();
        Color spaceBlack = (Color){ 15, 15, 20, 255 };
        Color blueTingedBlack = (Color){ 10, 10, 30, 255 };
        Color purpleNebula = (Color){ 20, 10, 25, 255 };
        Color warmDark = (Color){ 18, 12, 10, 255 };

        BeginDrawing();
        ClearBackground(spaceBlack);

        switch (currentScreen) {
            case MAIN_MENU:
            PlayMusic(MusicId::MENU);
            mainMenu.Update();
            mainMenu.Draw();

            if (mainMenu.ShouldStartGame()) {
                shipSelection.SetInitialShip(starshipType);
                currentScreen = SHIP_SELECTION;
                mainMenu.startGame = false;
            }
            if (mainMenu.ShouldOpenCollections()) {
                currentScreen = COLLECTIONS;
                mainMenu.collections = false;
            }
            if (mainMenu.ShouldOpenHangar()) {
                currentScreen = HANGAR;
                mainMenu.hangar = false;
            }
            if (mainMenu.ShouldOpenRunHistory()) {
                currentScreen = RUN_HISTORY;
                mainMenu.runHistory = false;
            }
            if (mainMenu.ShouldOpenSettings()) {
                currentScreen = SETTINGS;
                mainMenu.settings = false;
            }
            break;
            case SHIP_SELECTION:
            PlayMusic(MusicId::MENU);
            shipSelection.Update();
            shipSelection.Draw();
        
            if (IsKeyPressed(KEY_ENTER) && shipSelection.IsCurrentShipUnlocked()) {
                player.SetStarshipType(shipSelection.GetSelectedShip());
                runDifficulty = shipSelection.GetDifficulty();
                ApplyDifficultySpawnSettings();
                game.InitGame();
                currentScreen = asteroidsMode ? ASTEROIDS : GAME;
                SaveGameData();
            }
        
            if (IsKeyPressed(KEY_BACKSPACE)) {
                mainMenu.startGame = false;
                asteroidsMode = false;
                currentScreen = MAIN_MENU;
            }
            
            break;
        
            case GAME:
                PlayMusic(MusicId::GAME);
                game.Update();
                game.Draw();
                if (IsKeyPressed(KEY_BACKSPACE) || game.returnToMainMenu) {
                    FinalizeRunCredits();
                    UpdateHighScores();
                    SaveGameData();
                    mainMenu.startGame = false;
                    game.returnToMainMenu = false;
                    currentScreen = MAIN_MENU;
                }
                break;
            case ASTEROIDS:
                PlayMusic(MusicId::GAME);
                game.Update();
                game.Draw();
                if (IsKeyPressed(KEY_BACKSPACE) || game.returnToMainMenu) {
                    FinalizeRunCredits();
                    UpdateHighScores();
                    SaveGameData();
                    mainMenu.startGame = false;
                    asteroidsMode = false;
                    game.returnToMainMenu = false;
                    currentScreen = MAIN_MENU;
                }
                break;
            case COLLECTIONS:
            PlayMusic(MusicId::MENU);
                collections.Update();
                collections.Draw();
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    mainMenu.collections = false;
                    currentScreen = MAIN_MENU;
                }
                break;
            case HANGAR:
            PlayMusic(MusicId::MENU);
                hangar.Update();
                hangar.Draw();
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    SaveGameData();
                    mainMenu.hangar = false;
                    currentScreen = MAIN_MENU;
                }
                break;
            case RUN_HISTORY:
            PlayMusic(MusicId::MENU);
                runHistoryScreen.Update();
                runHistoryScreen.Draw();
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    mainMenu.runHistory = false;
                    currentScreen = MAIN_MENU;
                }
                break;
            case SETTINGS:
            PlayMusic(MusicId::MENU);
                settings.Update();
                settings.Draw();
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    SaveGameData();
                    mainMenu.settings = false;
                    currentScreen = MAIN_MENU;
                }
                break;
        }

        EndDrawing();
    }
    SaveGameData();
    CloseAudio();
    UnloadAllTextures();
    CloseWindow();
    return 0;
}
