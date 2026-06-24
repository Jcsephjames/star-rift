#pragma once
#ifndef AUDIO_HPP
#define AUDIO_HPP

enum class SfxId {
    LASER_FIRE,
    ENEMY_LASER_FIRE,
    LASER_IMPACT,
    EXPLOSION_SMALL,
    EXPLOSION_LARGE,
    SHIP_EXPLOSION,
    PLAYER_HIT,
    HEALTH_PICKUP,
    SCRAP_PICKUP,
    WAVE_ANNOUNCE,
    CARD_DRAFT_OPEN,
    UPGRADE_SELECT,
    EMP_PULSE,
    GAME_OVER,
    MENU_NAVIGATE,
    MENU_CONFIRM,
    COUNT
};

enum class MusicId {
    MENU,
    GAME,
    COUNT
};

void InitAudio();
void CloseAudio();
void UpdateAudio();

void PlaySfx(SfxId id);
void PlayMusic(MusicId id);
void StopMusic();

float GetEffectiveSfxVolume();
float GetEffectiveMusicVolume();

#endif
