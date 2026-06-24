#ifndef LASER_MANAGER_HPP
#define LASER_MANAGER_HPP

#include <vector>
#include "laser.hpp"
#include "ship.hpp"

#pragma once
#include "raylib.h"
#include "laser.hpp"
extern std::vector<Laser*> activeLasers;

void InitLasers();

void FireSingleLaser(Vector2 shipCenter, float directionDegrees, float shotSpeed, Vector2 laserSize, const std::vector<Vector2>& laserOffsets, int damage, bool fromEnemy = false);
void UpdateLasers(float deltaTime);
void DrawLasers();
void DestroyEnemyLasers();

#endif
