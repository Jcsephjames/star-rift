#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <raylib.h>
#include <vector>
#include "globals.hpp"
#include "ship.hpp"

class Player {
    public:
        Vector2 position;
        Vector2 velocity;
        float direction; // in degrees or radians
        int starshipType;
        Texture2D texture;
        float size;
        float scale; // Texture scale factor
        float radius = 20.0f;
        bool bellowY = false;

        Ship ship;

        float timeSinceLastShot;
        float speed; // Speed of the player
        float boost; // Boost speed factor
        float life;
        float maxHealth = 100.0f;
        int damage;
        float damageCooldown = 0.0f;
        float shotSpeed; // Speed of the shot
        float shotCooldown; // Cooldown time for shooting

        float rotationRate = 0.0f;
        bool isBoosting = false;
        bool isBraking = false;

        // std::vector<Vector2> lasers; // Store laser positions
        
        Player();

        void InitPlayer();
        void playerRotation(float deltaTime);
        void Update(float deltaTime);
        void Draw(float gameWidth, float gameHeight);

        const char* GetStarshipName(int type) {
            switch(type) {
                case 0: return "X-Wing";
                case 1: return "Y-Wing";
                case 2: return "TIE Fighter";
                case 3: return "TIE Bomber";
                case 4: return "Star Fighter";
                case 5: return "A-Wing";
                case 6: return "Death Fighter";
                case 7: return "Shlasher";
                case 8: return "Falcon";
                default: return "Unknown";
            }
        }
        void SetStarshipType(int type);
    
        // maybe Load/Unload textures inside or externally
    };


#endif
