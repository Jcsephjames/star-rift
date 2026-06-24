#ifndef BACKGROUND_HPP
#define BACKGROUND_HPP

#include "raylib.h"
#include <vector>

struct Star {
    float x, y;
    float speed;
    float size;
};

class Stars {
private:
    std::vector<Star> stars;
    const int starCount = 200;

public:
    Stars();
    void Update(float deltaTime);
    void Draw();
};

#endif