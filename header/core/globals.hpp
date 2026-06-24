#ifndef GLOBALS_H
#define GLOBALS_H
#include <raylib.h>
#include <vector>
#include "player.hpp"
#include "game.hpp"
#include "laser.hpp"
#include "shipSelection.hpp"


class Player;
class Game;
class ShipSelection;

extern Player player; // Player object
extern Game game; // Game object
extern Game currentRun; // Current run score/scrap/time (shared by both modes)
extern ShipSelection shipSelection;

extern int starshipType;

extern std::vector<Laser> lasers;

extern const int screenWidth;
extern const int screenHeight;

extern const int gameWidth;
extern const int gameHeight;

#define SPAWN_RATE_BOOST 1.3f

// ASTEROIDS
extern bool asteroidsMode;
extern bool asteroidsOn;
extern float asteroidSpawnInterval;
extern float asteroidSpawnTimer;

// ENEMIES
extern bool enemiesOn;
extern float enemySpawnInterval;
extern float enemySpawnTimer;
extern int maxActiveEnemies;


// SETTINGS
extern bool debugging;
extern bool enemyDebugging;
extern bool yWrapping;
extern bool directionPointerOn;
extern float debugTimer;

// AUDIO
extern float masterVolume;
extern float sfxVolume;
extern float musicVolume;
extern bool audioMuted;

// HIGH SCORES
extern int highScoreQuickPlay;
extern int highScoreAsteroids;
extern int totalShipsDestroyed;

// SHIP COLLECTION
extern bool shipUnlocked[MAX_SHIPS];
extern int shipHighScores[MAX_SHIPS];
extern float shipPlayTime[MAX_SHIPS];
extern int shipKills[MAX_SHIPS];

Vector2 WrapPosition(const Vector2& pos, float gameWidth, float gameHeight);

#endif
// GLOBALS_H