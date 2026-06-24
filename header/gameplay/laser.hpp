#ifndef LASER_HPP
#define LASER_HPP

#include <raylib.h>
#include <vector>
#include "ship.hpp"

class Laser {
    public:
        Vector2 position;
        Vector2 direction;
        float speed;
        Vector2 size;

        int damage;
        bool fromEnemy;
        Color color;
    
        Laser(Vector2 pos, Vector2 dir, float shotSpeed, Vector2 laserSize, int dmg, bool enemyLaser = false)
        : position(pos), direction(dir), speed(shotSpeed), size(laserSize), damage(dmg),
          fromEnemy(enemyLaser), color(enemyLaser ? (Color){255, 140, 0, 255} : RED) {}
    
        void Update(float delta) {
            position.x += direction.x * speed * delta;
            position.y += direction.y * speed * delta;
        }
    
        void Draw() const {
            Rectangle destRec = {
                position.x - size.x / 2,
                position.y - size.y / 2,
                size.x,
                size.y
            };
            Vector2 origin = { size.x / 2, size.y / 2};
            float rotationDegrees = atan2f(direction.y, direction.x) * RAD2DEG - 90.0f;
        
            DrawRectanglePro(destRec, origin, rotationDegrees, color);
        }
    
        bool IsOffScreen() const {
            return position.x < 0 || position.x > GetScreenWidth() ||
                   position.y < 0 || position.y > GetScreenHeight();
        }
    };
    
#endif
