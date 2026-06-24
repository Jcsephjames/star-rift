#pragma once
#include "background.hpp"
#include "player.hpp"
#include "laser.hpp"
#include "ship.hpp"
#include "overlay.hpp"

class Player;
extern Player player;

class Game {
public:
    bool gameOver = false;
    bool victory = false;
    bool endlessMode = false;
    bool returnToMainMenu = false;
    bool cardDraftOpen = false;
    bool pauseMenuOpen = false;
    bool runCreditsFinalized = false;
    char victoryMessage[128] = { 0 };
    int score;
    int scrap;
    float time;
    void InitGame();
    void Update();
    void Draw();
    void GameOver();
    void TriggerVictory(const char* message = nullptr);
    void ContinueEndless();
    void TimerLogic(float deltaTime);
    Game();
private:
    Stars stars;
};

