#pragma once
#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include "raylib.h"
#include "globals.hpp"
#include "textures.hpp"
#include "saveData.hpp"

extern Player player;

class Settings {
    private:
        static constexpr int settingsCount = 11; // Total number of settings options (indices 0–10)
        int settingsSelection = 2; // Default to first player-facing option (Y Wrapping)

        static bool IsDevSetting(int index) {
            return index == 0 || index == 1 || index == 4 || index == 5 || index == 6;
        }

        static bool IsSettingVisible(int index) {
            if (IsDevSetting(index) && !debugging) return false;
            if (index == 5 && !asteroidsOn) return false;
            return true;
        }

        void EnsureSelectionVisible() {
            if (IsSettingVisible(settingsSelection)) return;
            for (int i = 0; i < settingsCount; i++) {
                if (IsSettingVisible(i)) {
                    settingsSelection = i;
                    return;
                }
            }
        }

        void MoveSelection(int direction) {
            int next = settingsSelection;
            for (int step = 0; step < settingsCount; step++) {
                next += direction;
                if (next >= settingsCount) next = 0;
                if (next < 0) next = settingsCount - 1;
                if (IsSettingVisible(next)) {
                    settingsSelection = next;
                    return;
                }
            }
        }

    public:
        Settings() {
            EnsureSelectionVisible();
        }

        void Update() {
            if (IsKeyPressed(KEY_GRAVE)) {
                debugging = !debugging;
                EnsureSelectionVisible();
                SaveGameData();
            }

            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                MoveSelection(1);
            }
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                MoveSelection(-1);
            }
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_D)) {
                switch (settingsSelection)
                {
                // Debugging toggle
                case 0:
                    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                        debugging = false;
                    } else {
                        debugging = true;
                    }
                    EnsureSelectionVisible();
                    SaveGameData();
                    break;
                // Ship type selection
                case 1:
                    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                        player.starshipType--;
                        if (player.starshipType < 0) player.starshipType = MAX_SHIPS - 1;
                        player.SetStarshipType(player.starshipType);
                    } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                        player.starshipType++;
                        if (player.starshipType >= MAX_SHIPS) player.starshipType = 0;
                        player.SetStarshipType(player.starshipType);
                    }
                    SaveGameData();
                    break;
                case 2:
                    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                        yWrapping = false;
                    } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                        yWrapping = true;
                    }
                    SaveGameData();
                    break;
                case 3:
                    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                        directionPointerOn = false;
                    } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                        directionPointerOn = true;
                    }
                    SaveGameData();
                    break;
                case 4:
                    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                        asteroidsOn = !asteroidsOn;
                    } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                        asteroidsOn = !asteroidsOn;
                    }
                    EnsureSelectionVisible();
                    SaveGameData();
                    break;
                case 5:
                    if (asteroidsOn){
                        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                            asteroidSpawnInterval -= 0.5f;
                            if (asteroidSpawnInterval < 0.5f) asteroidSpawnInterval = 0.5f;
                        } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                            asteroidSpawnInterval += 0.5f;
                            if (asteroidSpawnInterval > 10.0f) asteroidSpawnInterval = 10.0f;
                        }
                        SaveGameData();
                    }
                    break;
                case 6:
                    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                        enemyDebugging = false;
                    } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                        enemyDebugging = true;
                    }
                    SaveGameData();
                    break;
                case 7:
                    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                        masterVolume -= 0.1f;
                        if (masterVolume < 0.0f) masterVolume = 0.0f;
                    } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                        masterVolume += 0.1f;
                        if (masterVolume > 1.0f) masterVolume = 1.0f;
                    }
                    SaveGameData();
                    break;
                case 8:
                    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                        sfxVolume -= 0.1f;
                        if (sfxVolume < 0.0f) sfxVolume = 0.0f;
                    } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                        sfxVolume += 0.1f;
                        if (sfxVolume > 1.0f) sfxVolume = 1.0f;
                    }
                    SaveGameData();
                    break;
                case 9:
                    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                        musicVolume -= 0.1f;
                        if (musicVolume < 0.0f) musicVolume = 0.0f;
                    } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                        musicVolume += 0.1f;
                        if (musicVolume > 1.0f) musicVolume = 1.0f;
                    }
                    SaveGameData();
                    break;
                case 10:
                    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                        audioMuted = false;
                    } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                        audioMuted = true;
                    }
                    SaveGameData();
                    break;
                }
            }
        }
        void Draw() {

            Color selectionColor = WHITE;
            Color nonSelectionColor = DARKGRAY;

            Color debuggingColor = (settingsSelection == 0) ? selectionColor : nonSelectionColor;
            Color starshipColor = (settingsSelection == 1) ? selectionColor : nonSelectionColor;
            Color yWrappingColor = (settingsSelection == 2) ? selectionColor : nonSelectionColor;
            Color directionPointerColor = (settingsSelection == 3) ? selectionColor : nonSelectionColor;
            Color asteroidsOnColor = (settingsSelection == 4) ? selectionColor : nonSelectionColor;
            Color asteroidSpawnIntervalColor = (settingsSelection == 5) ? selectionColor : nonSelectionColor;
            Color enemyDebuggingColor = (settingsSelection == 6) ? selectionColor : nonSelectionColor;
            Color masterVolumeColor = (settingsSelection == 7) ? selectionColor : nonSelectionColor;
            Color sfxVolumeColor = (settingsSelection == 8) ? selectionColor : nonSelectionColor;
            Color musicVolumeColor = (settingsSelection == 9) ? selectionColor : nonSelectionColor;
            Color audioMutedColor = (settingsSelection == 10) ? selectionColor : nonSelectionColor;

            DrawTexture(settings, gameWidth/2 - settings.width/2, 40, WHITE);

            int y = 150;
            const int lineHeight = 30;

            if (debugging) {
                const char* debuggingText = TextFormat("Debugging: %s", debugging ? "ON" : "OFF");
                DrawText(debuggingText, gameWidth/2 - MeasureText("Debugging: OFF", 20)/2, y, 20, debuggingColor);
                y += lineHeight;

                const char* starshipText = TextFormat("Starship Type: %s", player.GetStarshipName(player.starshipType));
                DrawText(starshipText, gameWidth/2 - MeasureText("Starship Type: Falcon", 20)/2, y, 20, starshipColor);
                y += lineHeight;
            }

            const char* yWrappingText = TextFormat("Y Wrapping: %s", yWrapping ? "ON" : "OFF");
            DrawText(yWrappingText, gameWidth/2 - MeasureText("Y Wrapping: OFF", 20)/2, y, 20, yWrappingColor);
            y += lineHeight;

            const char* directionPointerText = TextFormat("Direction Pointer: %s", directionPointerOn ? "ON" : "OFF");
            DrawText(directionPointerText, gameWidth/2 - MeasureText("Direction Pointer: OFF", 20)/2, y, 20, directionPointerColor);
            y += lineHeight;

            if (debugging) {
                const char* asteroidsText = TextFormat("Asteroids: %s", asteroidsOn ? "ON" : "OFF");
                DrawText(asteroidsText, gameWidth/2 - MeasureText("Asteroids: OFF", 20)/2, y, 20, asteroidsOnColor);
                y += lineHeight;

                if (asteroidsOn) {
                    const char* spawnText = TextFormat("Asteroid Spawn Interval: %.1f seconds", asteroidSpawnInterval);
                    DrawText(spawnText, gameWidth/2 - MeasureText("Asteroid Spawn Interval: 10.0 seconds", 18)/2, y, 18, asteroidSpawnIntervalColor);
                    y += lineHeight;
                }

                const char* enemyDebugText = TextFormat("Enemy Debugging: %s", enemyDebugging ? "ON" : "OFF");
                DrawText(enemyDebugText, gameWidth/2 - MeasureText("Enemy Debugging: OFF", 20)/2, y, 20, enemyDebuggingColor);
                y += lineHeight;
            }

            const char* masterVolumeText = TextFormat("Master Volume: %d%%", (int)(masterVolume * 100.0f));
            DrawText(masterVolumeText, gameWidth/2 - MeasureText("Master Volume: 100%", 20)/2, y, 20, masterVolumeColor);
            y += lineHeight;

            const char* sfxVolumeText = TextFormat("SFX Volume: %d%%", (int)(sfxVolume * 100.0f));
            DrawText(sfxVolumeText, gameWidth/2 - MeasureText("SFX Volume: 100%", 20)/2, y, 20, sfxVolumeColor);
            y += lineHeight;

            const char* musicVolumeText = TextFormat("Music Volume: %d%%", (int)(musicVolume * 100.0f));
            DrawText(musicVolumeText, gameWidth/2 - MeasureText("Music Volume: 100%", 20)/2, y, 20, musicVolumeColor);
            y += lineHeight;

            const char* audioMutedText = TextFormat("Mute Audio: %s", audioMuted ? "ON" : "OFF");
            DrawText(audioMutedText, gameWidth/2 - MeasureText("Mute Audio: OFF", 20)/2, y, 20, audioMutedColor);
        }
};

#endif // SETTINGS_HPP
