#pragma once
#ifndef SHIP_SELECTION_HPP
#define SHIP_SELECTION_HPP

#include "raylib.h"
#include "difficulty.hpp"
#include <string>

const int MAX_SHIPS = 9;

struct ShipStats {
    std::string name;
    int shotPower;      // 1 - 10 for each stat
    int shotCooldown;
    int speed;
    int boostMultiplier;
    Texture2D image;
};

class ShipSelection {
private:
    float shipSlideOffset;
    float slideSpeed = 10.0f; // smoothness

    ShipStats* ships[MAX_SHIPS];

    ShipStats* CreateShip(const char* name, Texture2D &texture, int shotPower, int shotCooldown, int speed, int boostMultiplier);

public:
    ShipSelection();
    ~ShipSelection();

    int shipSelection;
    int targetShipIndex;
    int difficultySelection;

    void InitShips();
    void SetInitialShip(int index);
    void Update();
    void Draw() const;

    int GetSelectedShip() const { return shipSelection; }
    RiftDifficulty GetDifficulty() const { return static_cast<RiftDifficulty>(difficultySelection); }
    const ShipStats* GetShip(int index) const;
    bool IsCurrentShipUnlocked() const;
};

#endif // SHIP_SELECTION_HPP
