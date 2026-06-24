#include <stdio.h>
#include "textures.hpp"

// global textures
Texture2D falcon;
Texture2D xWing;
Texture2D yWing;
Texture2D tieFighter;
Texture2D deathFighter;
Texture2D starWarsFighterOne;
Texture2D shlasher;
Texture2D tieBomber;
Texture2D shiptwo;
Texture2D deathStar;

Texture2D starRift;
Texture2D collections;
Texture2D settings;
Texture2D selection;

void LoadAllTextures() {
    falcon = LoadTexture("texture/falcon.png");
    xWing = LoadTexture("texture/xWing.png");
    yWing = LoadTexture("texture/YWing.png");
    tieFighter = LoadTexture("texture/tieFighter.png");
    deathFighter = LoadTexture("texture/DarthFighter.png");
    starWarsFighterOne = LoadTexture("texture/starFighterShipOne.png");
    shlasher = LoadTexture("texture/Shlasher.png");
    tieBomber = LoadTexture("texture/TIE Bomber.png");
    shiptwo = LoadTexture("texture/SHIPTWO.png");
    deathStar = LoadTexture("texture/deathStar.png");

    starRift = LoadTexture("texture/StarRift.png");
    collections = LoadTexture("texture/Collections300.png");
    settings = LoadTexture("texture/Settings300.png");
    selection = LoadTexture("texture/Selection300.png");

#ifdef DEV_BUILD
    printf("Falcon: %d x %d\n", falcon.width, falcon.height);
    printf("XWing: %d x %d\n", xWing.width, xWing.height);
    printf("YWing: %d x %d\n", yWing.width, yWing.height);
    printf("TieFighter: %d x %d\n", tieFighter.width, tieFighter.height);
    printf("DeathFighter: %d x %d\n", deathFighter.width, deathFighter.height);
    printf("StarWarsFighterOne: %d x %d\n", starWarsFighterOne.width, starWarsFighterOne.height);
    printf("Shlasher: %d x %d\n", shlasher.width, shlasher.height);
    printf("TieBomber: %d x %d\n", tieBomber.width, tieBomber.height);
    printf("ShipTwo: %d x %d\n", shiptwo.width, shiptwo.height);
    printf("DeathStar: %d x %d\n", deathStar.width, deathStar.height);

    printf("StarRift: %d x %d\n", starRift.width, starRift.height);
    printf("Collections: %d x %d\n", collections.width, collections.height);
    printf("Settings: %d x %d\n", settings.width, settings.height);
    printf("Selection: %d x %d\n", selection.width, selection.height);
#endif
}

void UnloadAllTextures() {
    UnloadTexture(falcon);
    UnloadTexture(xWing);
    UnloadTexture(yWing);
    UnloadTexture(tieFighter);
    UnloadTexture(deathFighter);
    UnloadTexture(starWarsFighterOne);
    UnloadTexture(shlasher);
    UnloadTexture(tieBomber);
    UnloadTexture(shiptwo);
    UnloadTexture(deathStar);

    UnloadTexture(starRift);
    UnloadTexture(collections);
    UnloadTexture(settings);
    UnloadTexture(selection);
#ifdef DEV_BUILD
    printf("All textures unloaded successfully.\n");
#endif
}
