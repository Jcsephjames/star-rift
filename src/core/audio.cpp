#include "audio.hpp"
#include "globals.hpp"
#include <cmath>
#include <cstring>

static const int SAMPLE_RATE = 44100;

static Sound sfx[(int)SfxId::COUNT] = {};
static bool sfxLoaded[(int)SfxId::COUNT] = {};

static Music musicTracks[(int)MusicId::COUNT] = {};
static bool musicLoaded[(int)MusicId::COUNT] = {};
static Sound ambientLoops[(int)MusicId::COUNT] = {};
static bool ambientLoaded[(int)MusicId::COUNT] = {};

static MusicId currentMusic = MusicId::COUNT;
static float laserSfxCooldown = 0.0f;
static float impactSfxCooldown = 0.0f;

static const char* SFX_PATHS[(int)SfxId::COUNT] = {
    "assets/audio/laser_fire.wav",
    "assets/audio/enemy_laser_fire.wav",
    "assets/audio/laser_impact.wav",
    "assets/audio/explosion_small.wav",
    "assets/audio/explosion_large.wav",
    "assets/audio/ship_explosion.wav",
    "assets/audio/player_hit.wav",
    "assets/audio/health_pickup.wav",
    "assets/audio/scrap_pickup.wav",
    "assets/audio/wave_announce.wav",
    "assets/audio/card_draft_open.wav",
    "assets/audio/upgrade_select.wav",
    "assets/audio/emp_pulse.wav",
    "assets/audio/game_over.wav",
    "assets/audio/menu_navigate.wav",
    "assets/audio/menu_confirm.wav",
};

static const char* MUSIC_PATHS[(int)MusicId::COUNT] = {
    "assets/audio/menu_music.ogg",
    "assets/audio/game_music.ogg",
};

static Wave MakeProceduralWave(int sampleCount, float duration, float (*sampleFn)(float t, float duration)) {
    Wave wave = { 0 };
    wave.frameCount = sampleCount;
    wave.sampleRate = SAMPLE_RATE;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = MemAlloc((unsigned int)sampleCount * sizeof(short));

    short* samples = (short*)wave.data;
    for (int i = 0; i < sampleCount; i++) {
        float t = (float)i / (float)SAMPLE_RATE;
        float value = sampleFn(t, duration);
        if (value > 1.0f) value = 1.0f;
        if (value < -1.0f) value = -1.0f;
        samples[i] = (short)(value * 32767.0f);
    }
    return wave;
}

static Wave GenLaserFireWave() {
    return MakeProceduralWave((int)(SAMPLE_RATE * 0.08f), 0.08f,
        [](float t, float duration) -> float {
            float env = 1.0f - (t / duration);
            float freq = 1800.0f - t * 6000.0f;
            return sinf(2.0f * PI * freq * t) * env * 0.35f;
        });
}

static Wave GenEnemyLaserFireWave() {
    return MakeProceduralWave((int)(SAMPLE_RATE * 0.07f), 0.07f,
        [](float t, float duration) -> float {
            float env = 1.0f - (t / duration);
            float freq = 900.0f - t * 2500.0f;
            return sinf(2.0f * PI * freq * t) * env * 0.28f;
        });
}

static float NoiseSample(float t, float duration) {
    (void)t;
    (void)duration;
    return (float)GetRandomValue(-1000, 1000) / 1000.0f;
}

static Wave GenLaserImpactWave() {
    return MakeProceduralWave((int)(SAMPLE_RATE * 0.05f), 0.05f,
        [](float t, float duration) -> float {
            float env = 1.0f - (t / duration);
            return NoiseSample(t, duration) * env * 0.25f;
        });
}

static Wave GenExplosionSmallWave() {
    return MakeProceduralWave((int)(SAMPLE_RATE * 0.18f), 0.18f,
        [](float t, float duration) -> float {
            float env = 1.0f - (t / duration);
            return NoiseSample(t, duration) * env * 0.45f;
        });
}

static Wave GenExplosionLargeWave() {
    return MakeProceduralWave((int)(SAMPLE_RATE * 0.35f), 0.35f,
        [](float t, float duration) -> float {
            float env = 1.0f - (t / duration);
            float noise = ((float)GetRandomValue(-1000, 1000) / 1000.0f) * env;
            float bass = sinf(2.0f * PI * 80.0f * t) * env * 0.35f;
            return (noise * 0.55f + bass) * env;
        });
}

static Wave GenShipExplosionWave() {
    return MakeProceduralWave((int)(SAMPLE_RATE * 0.28f), 0.28f,
        [](float t, float duration) -> float {
            float env = 1.0f - (t / duration);
            float noise = ((float)GetRandomValue(-1000, 1000) / 1000.0f) * env;
            float tone = sinf(2.0f * PI * (140.0f - t * 120.0f) * t) * env * 0.3f;
            return (noise * 0.5f + tone) * env;
        });
}

static Wave GenPlayerHitWave() {
    return MakeProceduralWave((int)(SAMPLE_RATE * 0.15f), 0.15f,
        [](float t, float duration) -> float {
            float env = 1.0f - (t / duration);
            return sinf(2.0f * PI * 110.0f * t) * env * 0.55f;
        });
}

static Wave GenHealthPickupWave() {
    return MakeProceduralWave((int)(SAMPLE_RATE * 0.2f), 0.2f,
        [](float t, float duration) -> float {
            float env = 1.0f - (t / duration);
            float freq = 520.0f + t * 900.0f;
            return sinf(2.0f * PI * freq * t) * env * 0.35f;
        });
}

static Wave GenScrapPickupWave() {
    return MakeProceduralWave((int)(SAMPLE_RATE * 0.1f), 0.1f,
        [](float t, float duration) -> float {
            float env = 1.0f - (t / duration);
            return sinf(2.0f * PI * 760.0f * t) * env * 0.3f
                 + sinf(2.0f * PI * 1140.0f * t) * env * 0.15f;
        });
}

static Wave GenWaveAnnounceWave() {
    return MakeProceduralWave((int)(SAMPLE_RATE * 0.35f), 0.35f,
        [](float t, float duration) -> float {
            float env = 1.0f - (t / duration);
            float freq = (t < 0.12f) ? 440.0f : 660.0f;
            return sinf(2.0f * PI * freq * t) * env * 0.35f;
        });
}

static Wave GenCardDraftOpenWave() {
    return MakeProceduralWave((int)(SAMPLE_RATE * 0.3f), 0.3f,
        [](float t, float duration) -> float {
            float env = 1.0f - (t / duration);
            float freq = 300.0f + t * 1200.0f;
            return sinf(2.0f * PI * freq * t) * env * 0.32f;
        });
}

static Wave GenUpgradeSelectWave() {
    return MakeProceduralWave((int)(SAMPLE_RATE * 0.25f), 0.25f,
        [](float t, float duration) -> float {
            float env = 1.0f - (t / duration);
            float freq = 500.0f + t * 700.0f;
            return sinf(2.0f * PI * freq * t) * env * 0.38f;
        });
}

static Wave GenEmpPulseWave() {
    return MakeProceduralWave((int)(SAMPLE_RATE * 0.45f), 0.45f,
        [](float t, float duration) -> float {
            float env = 1.0f - (t / duration);
            float freq = 220.0f - t * 180.0f;
            return sinf(2.0f * PI * freq * t) * env * 0.42f;
        });
}

static Wave GenGameOverWave() {
    return MakeProceduralWave((int)(SAMPLE_RATE * 0.6f), 0.6f,
        [](float t, float duration) -> float {
            float env = 1.0f - (t / duration);
            float freq = 420.0f - t * 500.0f;
            return sinf(2.0f * PI * freq * t) * env * 0.4f;
        });
}

static Wave GenMenuNavigateWave() {
    return MakeProceduralWave((int)(SAMPLE_RATE * 0.04f), 0.04f,
        [](float t, float duration) -> float {
            float env = 1.0f - (t / duration);
            return sinf(2.0f * PI * 620.0f * t) * env * 0.2f;
        });
}

static Wave GenMenuConfirmWave() {
    return MakeProceduralWave((int)(SAMPLE_RATE * 0.12f), 0.12f,
        [](float t, float duration) -> float {
            float env = 1.0f - (t / duration);
            return sinf(2.0f * PI * 520.0f * t) * env * 0.3f
                 + sinf(2.0f * PI * 780.0f * t) * env * 0.2f;
        });
}

static float AmbientSample(float t, float duration, float baseFreq) {
    float env = 0.5f + 0.5f * sinf(2.0f * PI * (1.0f / duration) * t);
    float tone = sinf(2.0f * PI * baseFreq * t) * 0.12f;
    float pad = sinf(2.0f * PI * (baseFreq * 1.5f) * t) * 0.06f;
    return (tone + pad) * env;
}

static Wave GenMenuAmbientWave() {
    return MakeProceduralWave((int)(SAMPLE_RATE * 4.0f), 4.0f,
        [](float t, float duration) -> float {
            return AmbientSample(t, duration, 110.0f);
        });
}

static Wave GenGameAmbientWave() {
    return MakeProceduralWave((int)(SAMPLE_RATE * 4.0f), 4.0f,
        [](float t, float duration) -> float {
            return AmbientSample(t, duration, 82.0f);
        });
}

static Wave (*PROCEDURAL_GENERATORS[(int)SfxId::COUNT])() = {
    GenLaserFireWave,
    GenEnemyLaserFireWave,
    GenLaserImpactWave,
    GenExplosionSmallWave,
    GenExplosionLargeWave,
    GenShipExplosionWave,
    GenPlayerHitWave,
    GenHealthPickupWave,
    GenScrapPickupWave,
    GenWaveAnnounceWave,
    GenCardDraftOpenWave,
    GenUpgradeSelectWave,
    GenEmpPulseWave,
    GenGameOverWave,
    GenMenuNavigateWave,
    GenMenuConfirmWave,
};

static void LoadSfx(SfxId id) {
    int idx = (int)id;
    if (sfxLoaded[idx]) return;

    if (FileExists(SFX_PATHS[idx])) {
        sfx[idx] = LoadSound(SFX_PATHS[idx]);
    } else {
        Wave wave = PROCEDURAL_GENERATORS[idx]();
        sfx[idx] = LoadSoundFromWave(wave);
        UnloadWave(wave);
    }
    sfxLoaded[idx] = true;
}

static void LoadMusicTrack(MusicId id) {
    int idx = (int)id;
    if (musicLoaded[idx] || ambientLoaded[idx]) return;

    if (FileExists(MUSIC_PATHS[idx])) {
        musicTracks[idx] = LoadMusicStream(MUSIC_PATHS[idx]);
        musicLoaded[idx] = true;
    } else {
        Wave wave = (id == MusicId::MENU) ? GenMenuAmbientWave() : GenGameAmbientWave();
        ambientLoops[idx] = LoadSoundFromWave(wave);
        UnloadWave(wave);
        ambientLoaded[idx] = true;
    }
}

float GetEffectiveSfxVolume() {
    if (audioMuted) return 0.0f;
    return masterVolume * sfxVolume;
}

float GetEffectiveMusicVolume() {
    if (audioMuted) return 0.0f;
    return masterVolume * musicVolume;
}

void InitAudio() {
    InitAudioDevice();

    for (int i = 0; i < (int)SfxId::COUNT; i++) {
        LoadSfx((SfxId)i);
        SetSoundVolume(sfx[i], GetEffectiveSfxVolume());
    }

    for (int i = 0; i < (int)MusicId::COUNT; i++) {
        LoadMusicTrack((MusicId)i);
        if (musicLoaded[i]) {
            SetMusicVolume(musicTracks[i], GetEffectiveMusicVolume());
        } else if (ambientLoaded[i]) {
            SetSoundVolume(ambientLoops[i], GetEffectiveMusicVolume() * 0.6f);
        }
    }
}

void CloseAudio() {
    StopMusic();

    for (int i = 0; i < (int)SfxId::COUNT; i++) {
        if (sfxLoaded[i]) {
            UnloadSound(sfx[i]);
            sfxLoaded[i] = false;
        }
    }

    for (int i = 0; i < (int)MusicId::COUNT; i++) {
        if (musicLoaded[i]) {
            UnloadMusicStream(musicTracks[i]);
            musicLoaded[i] = false;
        }
        if (ambientLoaded[i]) {
            UnloadSound(ambientLoops[i]);
            ambientLoaded[i] = false;
        }
    }

    CloseAudioDevice();
}

void UpdateAudio() {
    float deltaTime = GetFrameTime();
    if (laserSfxCooldown > 0.0f) laserSfxCooldown -= deltaTime;
    if (impactSfxCooldown > 0.0f) impactSfxCooldown -= deltaTime;

    float sfxVol = GetEffectiveSfxVolume();
    float musicVol = GetEffectiveMusicVolume();

    for (int i = 0; i < (int)SfxId::COUNT; i++) {
        if (sfxLoaded[i]) {
            SetSoundVolume(sfx[i], sfxVol);
        }
    }

    if (currentMusic != MusicId::COUNT) {
        int idx = (int)currentMusic;
        if (musicLoaded[idx]) {
            SetMusicVolume(musicTracks[idx], musicVol);
            UpdateMusicStream(musicTracks[idx]);
        } else if (ambientLoaded[idx]) {
            SetSoundVolume(ambientLoops[idx], musicVol * 0.6f);
            if (!IsSoundPlaying(ambientLoops[idx])) {
                PlaySound(ambientLoops[idx]);
            }
        }
    }
}

static void PlaySfxInternal(SfxId id, float pitch, float cooldown) {
    if (audioMuted || GetEffectiveSfxVolume() <= 0.0f) return;

    if (cooldown > 0.0f) {
        if (id == SfxId::LASER_FIRE || id == SfxId::ENEMY_LASER_FIRE) {
            if (laserSfxCooldown > 0.0f) return;
            laserSfxCooldown = cooldown;
        } else if (id == SfxId::LASER_IMPACT) {
            if (impactSfxCooldown > 0.0f) return;
            impactSfxCooldown = cooldown;
        }
    }

    int idx = (int)id;
    if (!sfxLoaded[idx]) LoadSfx(id);
    SetSoundPitch(sfx[idx], pitch);
    PlaySound(sfx[idx]);
}

void PlaySfx(SfxId id) {
    float pitch = 0.95f + (float)GetRandomValue(0, 10) / 100.0f;
    float cooldown = 0.0f;

    switch (id) {
        case SfxId::LASER_FIRE:
        case SfxId::ENEMY_LASER_FIRE:
            cooldown = 0.04f;
            break;
        case SfxId::LASER_IMPACT:
            cooldown = 0.03f;
            pitch = 0.9f + (float)GetRandomValue(0, 20) / 100.0f;
            break;
        default:
            break;
    }

    PlaySfxInternal(id, pitch, cooldown);
}

void PlayMusic(MusicId id) {
    if (id == currentMusic) return;

    StopMusic();
    currentMusic = id;

    int idx = (int)id;
    LoadMusicTrack(id);

    if (musicLoaded[idx]) {
        SetMusicVolume(musicTracks[idx], GetEffectiveMusicVolume());
        PlayMusicStream(musicTracks[idx]);
    } else if (ambientLoaded[idx]) {
        SetSoundVolume(ambientLoops[idx], GetEffectiveMusicVolume() * 0.6f);
        PlaySound(ambientLoops[idx]);
    }
}

void StopMusic() {
    if (currentMusic == MusicId::COUNT) return;

    int idx = (int)currentMusic;
    if (musicLoaded[idx]) {
        StopMusicStream(musicTracks[idx]);
    }
    if (ambientLoaded[idx]) {
        StopSound(ambientLoops[idx]);
    }
    currentMusic = MusicId::COUNT;
}
