# Star Rift

A top-down space shooter built with [raylib](https://www.raylib.com/) (C++17). Fight through escalating rift waves, draft run upgrades from scrap, survive asteroid fields, defeat faction bosses and the Death Star, then spend credits on ships and hangar upgrades.

## Requirements

- macOS
- Homebrew
- Xcode command-line tools (`g++`)

## Build & Run

```bash
brew install raylib
make build    # compiles to ./game
make run      # builds if needed, then launches
make clean    # removes the binary
```

The build script looks for raylib via `brew --prefix raylib`. If the build fails, confirm raylib is installed and try `make build` again.

**Release build** (strips dev-only settings from the binary):

```bash
STAR_RIFT_RELEASE=1 make build
```

**Dev mode at launch** (enables debug settings immediately):

```bash
./game --dev
```

## Project Layout

```
main.cpp              Entry point and screen routing
header/               Headers grouped by core, ui, gameplay, meta
src/core/             Save data, audio, textures, difficulty
src/ui/               Menus, overlays, run summary
src/gameplay/         Player, enemies, lasers, asteroids, game loop
src/meta/             Credits, hangar upgrades, card draft, run history
.venv/bin/            Build, run, and clean scripts invoked by make
```

## Controls

Press **C** on the main menu (or **P → Controls** in-game) for the full controls overlay.

| Context | Keys | Action |
|---------|------|--------|
| Main menu | W / S | Navigate options |
| Main menu | Enter | Select |
| Main menu | C | Toggle controls overlay |
| Main menu | Esc | Quit |
| Ship selection | A / D, arrows | Browse ships |
| Ship selection | W / S | Change difficulty |
| Ship selection | Enter | Confirm ship |
| Ship selection | Backspace | Back to menu |
| In-game | Q / E | Heavy rotate |
| In-game | A / D | Light rotate |
| In-game | S | Brake |
| In-game | Left Shift | Boost |
| In-game | Space | Fire lasers |
| In-game | P | Pause |
| In-game | Backspace | Return to menu |
| Pause menu | W / S | Navigate options |
| Pause menu | Enter | Select |
| Pause menu | P | Resume |
| Card draft | A / D, arrows | Browse cards |
| Card draft | 1–4 / Enter | Pick upgrade |
| Run summary | R | Restart run |
| Run summary | M | Return to menu |
| Run summary (victory) | C / Enter | Continue endless |
| Hangar | 1–8 | Purchase or toggle upgrade |
| Collections | A / D | Browse ships |
| Collections | Enter / Space | Purchase ship |
| Run history | A / D | Switch Rift / Asteroids tab |
| Run history | W / S | Browse entries |
| Settings | W / S | Navigate options |
| Settings | A / D | Adjust value |
| Settings | ` (grave) | Toggle debug mode |
| Sub-screens | Backspace | Back to menu |

## Game Modes & Screens

### Main Menu

Six options: **Rift Fight**, **Asteroids Mode**, **Hangar**, **Collections**, **Run History**, and **Settings**. Your credit balance and a **Rift Victories** counter (Death Star wins) are shown above the menu. A credit-goal nudge points toward the next affordable hangar or ship unlock.

### Rift Fight

The main survival mode. Enemies and asteroids spawn in escalating waves (~40 seconds each). Score is kills; survival time is tracked separately for records. Pick a difficulty before launch:

| Difficulty | Effect |
|------------|--------|
| **Rift** | Standard spawn rate and enemy cap |
| **Surge** | ~1.4× spawn rate, +1 enemy cap |
| **Collapse** | ~1.8× spawn rate, +2 enemy cap |

**Asteroid fields** — Each wave has a ~15% chance to become an asteroid field instead of a combat wave. Fields last ~28 seconds, spawn heavier asteroid pressure, and show an "Asteroid Field" HUD label with boss/finale milestones.

**Card draft** — Collect scrap during a run. Every 100 scrap (then every +10 after each draft), the game pauses and offers four upgrade cards. One slot is always a full hull repair; the other three are random upgrades from Common through Legendary rarity. Picking a card clears your scrap and triggers a brief EMP pulse.

**Boss waves**

| Wave | Boss | Notes |
|------|------|-------|
| 10 | Mini-boss | Rebellion pilots face the **Shlasher**; Empire pilots face the **Falcon** |
| 20 | **Death Star** | Final boss — destroy it to win |

After victory, the run summary offers **Continue endless** (waves keep climbing past 20 with escalating pressure), **Restart**, or **Return to menu**. Victory credits are locked in when you win; endless play is for glory and high scores.

Runs earn credits and are logged in Run History with wave, kills, scrap, upgrades picked, victory status, and credit breakdown.

### Asteroids Mode

Classic asteroid-dodging mode with screen wrap on the Y axis. No enemy ships — just asteroids at a faster spawn rate. Difficulty still affects asteroid spawn speed. High score is points-based. No card draft in this mode.

**Victory** — End a run in victory (with credits) by reaching any one of:

- 5 minutes survived
- 5,000 points
- 100 asteroids cleared

### Hangar

Spend credits on eight permanent upgrades. Press the numbered key to purchase; owned cosmetic items can be toggled on or off.

| # | Upgrade | Cost | Effect |
|---|---------|------|--------|
| 1 | Neon Trail | 500 | Cosmetic engine exhaust |
| 2 | Gold Hull | 750 | Cosmetic gold ship tint |
| 3 | Reinforced Frame | 1,000 | Permanent +5 max hull |
| 4 | Crimson Hull | 850 | Cosmetic crimson ship tint |
| 5 | Ion Trail | 650 | Cosmetic ion engine exhaust |
| 6 | Overcharged Cells | 1,250 | Permanent +1 laser damage |
| 7 | Magnetic Scoop | 1,100 | Permanent +30% pickup radius |
| 8 | Bounty Scanner | 1,500 | Permanent +15% run credits |

### Collections

Browse all nine ships, view per-ship stats and records, and unlock new ships with credits. X-Wing and TIE Fighter are free starters; others range from 3,000 to 10,000 credits.

| Tier | Ships | Cost |
|------|-------|------|
| Starter | X-Wing, TIE Fighter | Free |
| Mid | Y-Wing, TIE Bomber | 3,000 |
| Elite | Star Fighter, Ship Two, Death Fighter | 6,000 |
| Boss | Shlasher, Falcon | 10,000 |

### Run History

Review your last 10 runs for Rift Fight and Asteroids (separate tabs). Rift entries show ship, wave, kills, scrap, upgrades, victory status, and credits earned. Asteroids entries show ship, score, scrap, time, and credits.

### Settings

Adjust Y wrapping, direction pointer, master/SFX/music volume, and mute. Press **`** (grave) to toggle debug mode, which reveals dev-only options (spawn toggles, ship picker, enemy debug).

## Credits

Credits are earned at the end of each run. Runs shorter than 5 seconds earn nothing; otherwise the minimum payout is 5 credits.

**Rift Fight formula** (before Bounty Scanner bonus):

```
kills × 3  +  scrap ÷ 4  +  time ÷ 2  +  wave × 8  [+ victory bonus if applicable]
```

Victory bonus: `500 + wave × 25` (awarded when the Death Star is destroyed, or when an Asteroids victory condition is met).

**Bounty Scanner** (hangar upgrade #8) multiplies the total by 1.15×.

The run summary screen shows a full credit breakdown and nudges you toward the next affordable unlock.

## Save File

Progress is stored in **`starrift_save.txt`** in the project root (same directory you run `./game` from).

The save includes credits, unlocked ships, hangar upgrades, high scores, per-ship stats, audio settings, difficulty preference, rift victory count, and run history. Delete the file to reset all progress.
