#pragma once
#ifndef COLLECTIONS_HPP
#define COLLECTIONS_HPP

#include "raylib.h"
#include <cstddef>

class Collections {
private:
    int selectedShipIndex;

    void DrawShipThumbnail(int index, int x, int y, int size) const;
    void DrawShipPreview(int centerX, int centerY) const;
    static void FormatPlayTime(float seconds, char* buffer, size_t bufferSize);

public:
    Collections();

    void Update();
    void Draw() const;
};

#endif
