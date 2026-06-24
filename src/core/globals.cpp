#include "globals.hpp"
#include "player.hpp"
#include "shipSelection.hpp"
#include <cstring>

Player player;
Game game;
Game currentRun;
ShipSelection shipSelection;

int starshipType = 0; // Default starship type

const int screenWidth = 1000;
const int screenHeight = 800;

const int gameWidth = 1000;
const int gameHeight = 800;

// ASTEROIDS
bool asteroidsMode = false;
bool asteroidsOn = true;
float asteroidSpawnInterval = 5.0f;
float asteroidSpawnTimer = 0.0f;

// ENEMIES
bool enemiesOn = false;
float enemySpawnInterval = 7.0f;
float enemySpawnTimer = 0.0f;
int maxActiveEnemies = 10;


// SETTINGS
#ifdef DEV_BUILD
bool debugging = true;
#else
bool debugging = false;
#endif
bool enemyDebugging = false;
bool yWrapping = false;
bool directionPointerOn = true;
float debugTimer = 0;

// AUDIO
float masterVolume = 0.8f;
float sfxVolume = 0.8f;
float musicVolume = 0.5f;
bool audioMuted = false;

// HIGH SCORES
int highScoreQuickPlay = 0;
int highScoreAsteroids = 0;
int totalShipsDestroyed = 0;

// SHIP COLLECTION
bool shipUnlocked[MAX_SHIPS] = {};
int shipHighScores[MAX_SHIPS] = {};
float shipPlayTime[MAX_SHIPS] = {};
int shipKills[MAX_SHIPS] = {};

Vector2 WrapPosition(const Vector2& pos, float gameWidth, float gameHeight) {
    Vector2 wrappedPos = pos;

    if (pos.x < 0.0f){ 
        wrappedPos.x = gameWidth;
#ifdef DEV_BUILD
        if (debugging) {
            printf("Wrapped X position from %f to %f\n", pos.x, wrappedPos.x);
        }
#endif
    }
    else if (pos.x > gameWidth){
        wrappedPos.x = 0.0f;
#ifdef DEV_BUILD
        if (debugging) {
            printf("Wrapped X position from %f to %f\n", pos.x, wrappedPos.x);
        }
#endif
    }
    // Optionally add Y wrapping:
    if (yWrapping) {
        if (pos.y < 0.0f){
            wrappedPos.y = gameHeight;
#ifdef DEV_BUILD
            if (debugging) {
                printf("Wrapped Y position from %f to %f\n", pos.y, wrappedPos.y);
            }
#endif
        }
        else if (pos.y > gameHeight){
            wrappedPos.y = 0.0f;
#ifdef DEV_BUILD
            if (debugging) {
                printf("Wrapped Y position from %f to %f\n", pos.y, wrappedPos.y);
            }
#endif
        }
    }

    return wrappedPos;
}