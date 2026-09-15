# GAME_ARCHITECTURE.md

Technical documentation for **Joan of Arc : Banner of Orleans**
C++ / iGraphics / Visual Studio 2013 (toolset v120, Win32, MultiByte)

This document describes the code **as it exists right now**. Anything that is a
stub, unused, or absent is marked `[PLACEHOLDER]` or `[NOT IMPLEMENTED]`.
Nothing here is taken from the project proposal — only from the code.

---

## 1. PROJECT STRUCTURE

### 1.1 Game source files (all in `demo/demo/`)

| File | Type | Purpose | Key contents | Depends on | Depended on by |
|---|---|---|---|---|---|
| `iMain.cpp` | Source (the **only** `.cpp` in the build) | Framework callbacks and entry point. Pure dispatch — contains no game rules. | `iDraw()`, `gameUpdate()`, `animationTick()`, `fixedUpdate()`, `iMouse()`, `iMouseMove()`, `iPassiveMouseMove()`, `main()` | everything | — (top of the tree) |
| `Config.hpp` | Header | Every tunable constant and the colour palette. No logic. | `SCREEN_WIDTH/HEIGHT`, `LOGIC_TICK_MS`, `ANIM_TICK_MS`, `KEY_POLL_MS`, `FADE_TICKS`, `TOTAL_LEVELS`, `START_LIVES`, physics/player/unit/scoring constants, `struct Color`, `COL_*`, `FONT_*` | nothing | all game modules |
| `Utils.hpp` | Header | Geometry and maths helpers. | `struct Rect`, `makeRect()`, `rectsOverlap()`, `pointInRect()`, `clampi/clampd`, `lerpd()`, `randRange()`, `getEuclideanDistance()` | `Config.hpp` | all game modules |
| `GameState.hpp` | Header | The state machine and the globals that outlive a screen. | `enum GameStateId`, `gameState`, `fadeTimer`, `uiTime`, `animFrame`, `currentLevel`, `score`, `highScore`, `lives`, `heroPath`, `changeState()`, `resetRun()`, `stateName()`, `levelName()` | `Config.hpp` | almost everything |
| `Input.hpp` | Header | Converts the framework's held-key arrays into single key presses. | `keyHeld()`, `keyTapped()`, `keyTappedCh()`, `specialHeld()`, `specialTapped()`, `inputEndFrame()`, `prevKey[]`, `prevSpecial[]`, `KEY_ENTER/ESCAPE/SPACE` | `Config.hpp`, iGraphics globals | `Screens.hpp`, `Player.hpp` |
| `AssetList.hpp` | Header, **GENERATED** | Machine-written list of exported artwork. Never edited by hand. | `ASSET_SPRITE_COUNT`, `enum SpriteId`, `ASSET_TABLE[]`, `enum AnimId`, `ANIM_TABLE[]` | nothing | `Assets.hpp` |
| `Assets.hpp` | Header | Texture loading and sprite drawing. | `struct Sprite`, `gSprites[]`, `loadAllAssets()`, `fileExists()`, `spriteAspect()`, `drawSpriteTint()`, `drawSpriteEx()`, `drawMissingBox()` (+ unused helpers, see §16) | `Config.hpp`, `Utils.hpp`, `AssetList.hpp`, iGraphics | `UI.hpp`, `World.hpp`, `Player.hpp`, `Units.hpp`, `HUD.hpp` |
| `UI.hpp` | Header | Reusable 2D drawing: text, panels, buttons, the menu backdrop. | `setColor()`, `setColorMix()`, `beginBlend()/endBlend()`, `fillRectAlpha()`, `textWidth()`, `drawText*()`, `drawFrame()`, `drawPanel()`, `drawDiamond()`, `drawDivider()`, `drawButton()`, `drawHangingBanner()`, `drawBackdrop()`, `drawFadeOverlay()`, `drawHintBar()` | `Config.hpp`, `Utils.hpp`, `GameState.hpp` | `World.hpp`, `Effects.hpp`, `HUD.hpp`, `Game.hpp`, `Screens.hpp` |
| `Audio.hpp` | Header | Music and sound effects (iGraphics has none). | `musicPlay()`, `musicStop()`, `musicToggle()`, `sfx()`, `gMusicOn`, `gSoundOn`, `gMusicAlias` | `windows.h`, `mmsystem.h`, `Config.hpp` | `World.hpp`, `Player.hpp`, `Units.hpp`, `Combat.hpp`, `Game.hpp`, `Screens.hpp` |
| `World.hpp` | Header | The level: camera, background, barricades, pickups, level construction. | `cameraX`, `worldToScreenX()`, `onScreen()`, `struct Obstacle`/`obstacles[]`, `struct Pickup`/`pickups[]`, `levelLength`, `levelKills`, `bossAlive`, `gateX`, `setLevelMood()`, `drawParallaxBackground()`, `drawGround()`, `drawObstacles()`, `drawPickups()`, `drawGate()`, `buildLevel()` | `Config`, `Utils`, `GameState`, `Assets`, `Audio`, (uses `UI.hpp` helpers) | `Effects`, `Projectiles`, `Player`, `Units`, `Combat`, `HUD`, `Game` |
| `Effects.hpp` | Header | Short-lived visual feedback. Never affects gameplay. | `struct Effect`, `effects[]`, `screenShake`, `fxSpark()`, `fxNumber()`, `fxText()`, `fxRing()`, `fxDust()`, `shakeScreen()`, `updateEffects()`, `drawEffects()` | `Config`, `Utils`, `World` | `Projectiles`, `Player`, `Units`, `Combat`, `Game` |
| `Projectiles.hpp` | Header | Arrows and the boss shockwave, for both sides. | `enum Faction`, `enum ProjType`, `struct Projectile`, `projectiles[]`, `spawnArrow()`, `spawnShock()`, `projRect()`, `updateProjectiles()`, `drawProjectiles()`, `clearProjectiles()` | `Config`, `Utils`, `World`, `Effects` | `Player`, `Units`, `Combat`, `Game` |
| `Player.hpp` | Header | Joan: movement, gravity, jump, sword, bow, health, hero stages. | `struct Player`, `player`, `playerRect()`, `swordRect()`, `playerSwordDamage()`, `playerArrowDamage()`, `playerSpeed()`, `resetPlayer()`, `newRunPlayer()`, `setHeroStage()`, `damagePlayer()`, `healPlayer()`, `readPlayerInput()`, `updatePlayer()`, `drawPlayer()` | `Config`, `Utils`, `GameState`, `Input`, `Assets`, `Audio`, `World`, `Effects`, `Projectiles` | `Units`, `Combat`, `HUD`, `Game`, `iMain.cpp` |
| `Units.hpp` | Header | Every walking thing that is not Joan: enemies, companions, boss — one struct, one loop. | `enum UnitType/UnitState`, `struct Unit`, `units[]`, `unitMaxHp()`, `unitSpeedOf()`, `unitScaleOf()`, `unitDamageOf()`, `unitScoreOf()`, `unitReachOf()`, `damageMultiplier()`, `unitName()`, `spawnUnit()`, `unitRect()`, `countLiveEnemies()`, `damageUnit()`, `findTarget()`, `updateUnits()`, `unitTint()`, `drawUnits()` | `Config`, `Utils`, `Assets`, `Audio`, `World`, `Effects`, `Projectiles`, `Player` | `Combat`, `HUD`, `Game` |
| `Combat.hpp` | Header | **Every collision in the game**, plus the cross-system half of the abilities. | `resolveSword()`, `resolveProjectiles()`, `resolveUnitAttacks()`, `resolveContact()`, `resolvePickups()`, `resolveAbilities()`, `updateCamera()`, `resolveAllCollisions()` | `Config`, `Utils`, `GameState`, `Audio`, `World`, `Effects`, `Projectiles`, `Player`, `Units` | `Game.hpp` |
| `HUD.hpp` | Header | Everything drawn on top of the battlefield. | `drawAbilityIcon()`, `drawHUD()` | `Config`, `Utils`, `GameState`, `UI`, `Assets`, `Player`, `Units`, `World` | `Game.hpp` |
| `Game.hpp` | Header | Level flow: build a level, tick it, render one frame of it. | `levelIntroTimer`, `bossWoke`, `populateLevel()`, `startLevel()`, `onPlayerDeath()`, `updateGame()`, `drawGameplay()` | all gameplay modules | `Screens.hpp` |
| `Screens.hpp` | Header | One draw function and one input handler per state, plus three dispatchers. | `drawMenuScreen()`, `drawInstructionsScreen()`, `drawCreditsScreen()`, `drawStoryScreen()`, `drawPlayingScreen()`, `drawPauseScreen()`, `drawPathChoiceScreen()`, `drawLevelCompleteScreen()`, `drawGameOverScreen()`, `drawVictoryScreen()`, matching `handle*Keys()`, `drawCurrentScreen()`, `handleCurrentKeys()`, `updateCurrentHover()`, `handleCurrentClick()` | `Config`, `Utils`, `GameState`, `Input`, `UI`, `Game` | `iMain.cpp` |

### 1.2 Framework and library files — **never modified**

| File | Purpose |
|---|---|
| `iGraphics.h` | The course framework (v4.0). Window creation, timers, drawing primitives, texture loading, GLUT callback wiring. |
| `glut.h`, `glaux.h`, `stb_image.h` | GLUT headers, glaux headers, the PNG/JPG decoder used by `iLoadImage`. |
| `glut32.lib`, `Glaux.lib`, `glui32.lib`, `OPENGL32.LIB`, `GLU32.LIB`, `GLUT32.DLL` | Libraries and the GLUT runtime DLL. |

### 1.3 Data and tooling

| Path | Purpose |
|---|---|
| `demo/demo/Assets/player|enemy|background|ui/` | Generated PNGs actually loaded by the game. `boss/`, `items/`, `effects/` exist but are empty. |
| `demo/demo/Audios/background.mp3`, `gameover.mp3` | Music, played through MCI. |
| `demo/demo/Audios/sfx/*.wav` | 11 generated effects: `swing, hit, arrow, jump, coin, heal, hurt, death, summon, ult, levelup`. |
| `tools/prepare_assets.py` | The asset pipeline. Not part of the build. |
| `tools/assets.json` | Job list driving the pipeline. |
| `tools/source_art/` | Raw artwork input (`joan_turnaround.jpg`, `enemy_brute.jpg`, `battlefield.jpg`). |
| `tools/preview/` | Checkerboard previews of each cutout. |
| `demo/demo.sln`, `demo/demo/demo.vcxproj` | Solution and project actually built. |

### 1.4 Legacy files still present but **not part of the game**

| File | Status |
|---|---|
| `Balloon.hpp` | Old balloon demo struct. Listed in the project, included by nothing. |
| `bitmap_loader.h` | Alternate BMP loader. Never included. |
| `Images/balloon*.png` | Old demo art. Unused. |
| `iMain_boxdemo_backup.txt` | Backup of the original box demo `iMain.cpp`. |
| `BubblePop Game.vcxproj`, `demo.vcxproj.bak` | Orphan project files, not in the solution. **Do not open these** — they lack the include/library paths. |

---

## 2. FILE-BY-FILE EXPLANATION

### `iMain.cpp` — the framework boundary
**Responsibility:** be the only place the framework talks to the game, and do nothing else.

The framework requires five functions to exist: `iDraw`, `fixedUpdate`, `iMouse`, `iMouseMove`, `iPassiveMouseMove`. All five live here and immediately delegate.

- `main()` seeds `rand`, registers two timers **before** the window exists (`iSetTimer(20, gameUpdate)`, `iSetTimer(120, animationTick)`), calls `iInitialize(1200, 675, title, KEY_POLL_MS)`, then `loadAllAssets()`, then `iStart()`. The asset call sits between `iInitialize` and `iStart` because a texture cannot be created before the OpenGL context exists, and `iStart()` never returns.
- `iDraw()` calls `iClear()`, `updateCurrentHover()`, `drawCurrentScreen()`.
- `gameUpdate()` advances `uiTime`, decrements `fadeTimer`, and returns early unless `gameState == STATE_PLAYING`; otherwise calls `updateGame()`.
- `animationTick()` increments `animFrame`. **`animFrame` is currently read by nothing** — see §16.
- `fixedUpdate()` calls `handleCurrentKeys()` then `inputEndFrame()`.
- `iMouse()` behaves differently by state: during `STATE_PLAYING` left button swings the sword and right button fires an arrow (stage ≥ 2); in every other state it forwards to `handleCurrentClick()`.
- `iMouseMove()` and `iPassiveMouseMove()` are **empty** `[PLACEHOLDER]`. Hover is read from the framework's own `iMouseX`/`iMouseY` globals inside `iDraw`.

### `Config.hpp` — the numbers
Exists so tuning never requires hunting through logic. Contains only constants, `struct Color`, and `#pragma warning(disable:4996)` which suppresses the VS2013 "unsafe function" warnings for `sprintf`/`fopen` used across the project. Included first by everything.

### `Utils.hpp` — geometry
`Rect` plus `rectsOverlap()` are the entire collision vocabulary of the game: every collision in `Combat.hpp` is an AABB overlap test. `getEuclideanDistance()` survives from the original template and is currently unused by the game.

### `GameState.hpp` — the spine
Holds `gameState` and the run-scoped values that must survive screen changes: `currentLevel`, `score`, `highScore`, `lives`, `heroPath`. `changeState()` is the only sanctioned way to move between screens because it also arms `fadeTimer`. `resetRun()` returns the run to level 1 with zero score. `levelName()` maps 1/2/3 to the three mission titles.

### `Input.hpp` — press detection
Exists because this iGraphics version has **no key event callback at all**. It only maintains `keyPressed[]`/`specialKeyPressed[]` while a key is held. `Input.hpp` snapshots those arrays each input tick into `prevKey[]`/`prevSpecial[]`, so `keyTapped()` can return true only on the 0→1 transition. Without it, one tap on a menu would fire ~125 times per second. `inputEndFrame()` **must** be called exactly once at the end of `fixedUpdate()`.

### `AssetList.hpp` — generated manifest
Written by `tools/prepare_assets.py`. Gives C++ the sprite ID enum, each asset's path, real pixel size, and power-of-two canvas size. Editing it by hand is pointless because the next pipeline run overwrites it.

### `Assets.hpp` — textures
`loadAllAssets()` walks `ASSET_TABLE`, checks each file exists with `fopen` (because `stbi_load` fails *silently* inside `iLoadImage`), calls `iLoadImage`, stores the texture ID plus the UV fraction `u1/v1` the artwork occupies inside its canvas, and prints a load report to the console.

`drawSpriteTint()` is the real drawing function; `drawSpriteEx()` is a white-tint wrapper. They exist instead of the framework's `iShowImage()` because that function has no alpha blending (soft PNG edges become halos), always samples the whole texture (our art sits in the corner of a padded canvas), and cannot mirror a sprite. A missing asset draws a magenta crossed box instead of failing silently.

### `UI.hpp` — the look
Everything that is not a sprite. `fillRectAlpha()` wraps `glEnable(GL_BLEND)` around a `glColor4f` quad, because the framework's `iSetColor` is RGB only. `drawTextBold()` fakes weight by overdrawing at 1px offsets since GLUT has no bold font. `drawBackdrop()` is the night battlefield used by every non-gameplay screen. `drawFadeOverlay()` reads `fadeTimer` and paints a black rectangle whose alpha falls to zero, producing the fade-in after every state change.

### `Audio.hpp` — sound
Music is MP3 through `mciSendStringA` (`open … type mpegvideo alias …` then `play … repeat`); effects are WAV through `PlaySoundA(..., SND_ASYNC)`. They are separate subsystems, so an effect does not interrupt the music. If MCI has no MP3 decoder the open call fails and the game continues silently. No project settings were needed: `winmm.lib` is linked by a `#pragma` inside `glut.h`, and `mmsystem.h` arrives with `windows.h`.

### `World.hpp` — the level
Owns `cameraX` and the coordinate convention: everything is stored in world space, and only drawing subtracts `cameraX`. Holds the barricade and pickup arrays and their draw routines. `buildLevel()` sets the level's length, mood tint and gate position, then scatters barricades, coins and three flasks. It does **not** place enemies — that is `populateLevel()` in `Game.hpp`.

`drawParallaxBackground()` draws the battlefield painting at 0.35× the camera speed, mirroring every second copy so the non-tileable image meets itself edge to edge instead of showing a seam.

### `Effects.hpp` — feedback
A fixed pool of 96 effects (`FX_SPARK`, `FX_NUMBER`, `FX_RING`, `FX_DUST`; `FX_SHOCK` is drawn but never spawned). Also owns `screenShake`, which `drawGameplay()` converts into a `glTranslatef` offset. Nothing here influences simulation.

### `Projectiles.hpp` — arrows
One pool of 72, each tagged with the side that fired it, so player arrows and enemy bolts share one update loop. Projectiles die on barricades, on the ground, on a life timer, or when far off screen. `PROJ_SHOCK` is the boss's ground wave.

### `Player.hpp` — Joan
Holds the whole player state including the four-stage progression. Deliberately split in two halves:
- `readPlayerInput()` runs on the **input** clock and only sets intentions (`vx`, `swingTimer`, and the `castHeal`/`castSummon`/`castUlt` flags).
- `updatePlayer()` runs on the **simulation** clock: timers, horizontal move, barricade push-out, gravity, landing, then animation state.

Abilities only raise a flag here; `Combat.hpp` performs the part that touches other systems. That keeps this file free of any dependency on the unit array.

### `Units.hpp` — everything else that walks
One `Unit` struct with a `side` field serves enemies, companions and the boss, which is why the summon ability was nearly free to implement. `damageMultiplier()` implements the counter triangle. `damageUnit()` is the single entry point for hurting a unit and is also where score, kill counting, boss death and coin drops happen. `updateUnits()` is the AI: sleep until Joan is within `AGGRO_RANGE`, pick a target, approach until inside weapon reach, then wind up and strike. Boss phases escalate at 2/3 and 1/3 health.

### `Combat.hpp` — all collisions in one place
Deliberately the only file that knows about more than one gameplay system, so that "what hit what" is answerable by reading one file. Runs in a fixed order each tick: sword → projectiles → unit melee → body contact → pickups → abilities. `updateCamera()` also lives here because it depends on the player.

### `HUD.hpp` — the overlay
Portrait, health bar, stage/path text, lives, score, gold, objective bar with a marker, five ability icons with cooldown sweeps, the boss bar, and a red vignette below 30% health. Reads state; changes nothing.

### `Game.hpp` — level flow
`startLevel()` clears the pools, builds the level, places the army, resets Joan, sets her stage from the level number, and starts the music. `updateGame()` is one simulation tick and the win/lose checks. `onPlayerDeath()` spends a life and either respawns her 260px back or ends the run. `drawGameplay()` renders one frame in a fixed order.

### `Screens.hpp` — one file per screen, three dispatchers
Every state has `drawXxxScreen()` and `handleXxxKeys()`. The three dispatchers (`drawCurrentScreen`, `handleCurrentKeys`, `handleCurrentClick`) are `switch` statements on `gameState`. Adding a screen means adding an enum value and two functions and three switch cases.

---

## 3. GAME ARCHITECTURE

```
        Windows / GLUT message loop  (inside iStart -> glutMainLoop)
                 |
   +-------------+--------------------+--------------------+
   |             |                    |                    |
 keyboard     Win32 timer          Win32 timer         glutIdleFunc
 & mouse       every 8 ms          every 20 ms         (uncapped)
   |             |                    |                    |
   v             v                    v                    v
keyPressed[]  fixedUpdate()       gameUpdate()          iDraw()
specialKey[]      |                    |                    |
   |              v                    v                    v
   +------> handleCurrentKeys()   uiTime++, fade--     updateCurrentHover()
                  |                    |                    |
                  v                    | (only if PLAYING)  v
            readPlayerInput()          v               drawCurrentScreen()
            sets vx / flags        updateGame()             |
                                       |                    v
                                       |            per-state draw fn
                    +------------------+---------+          |
                    |        |        |          |          v
              updatePlayer  updateUnits  updateProjectiles  drawGameplay()
                    |        |        |          |     (world -> HUD -> fade)
                    +--------+---+----+----------+
                                 |
                       resolveAllCollisions()   <-- Combat.hpp
                                 |
                          updateCamera()
                                 |
                          changeState(...) on win / loss
```

**Where each concern lives**

| Concern | Location |
|---|---|
| Keyboard input | `fixedUpdate()` → `handleCurrentKeys()` → per-state handler; gameplay movement in `readPlayerInput()` (`Player.hpp`) |
| Mouse click | `iMouse()` in `iMain.cpp` → `handleCurrentClick()` (`Screens.hpp`), or direct attack during `STATE_PLAYING` |
| Mouse hover | `updateCurrentHover()` called from `iDraw()`, reading `iMouseX`/`iMouseY` |
| Game simulation | `gameUpdate()` → `updateGame()` (`Game.hpp`) and the `update*` functions it calls |
| Collision | `resolveAllCollisions()` (`Combat.hpp`) only |
| Animation | `animationTick()` increments `animFrame` `[currently unused]`; visible motion is driven by `uiTime` and `player.runPhase` |
| Rendering | `iDraw()` → `drawCurrentScreen()` → per-state draw function |
| Asset loading | `loadAllAssets()` once in `main()`, between `iInitialize()` and `iStart()` |
| Sound | `sfx()` at the moment of the event; `musicPlay()`/`musicStop()` in `startLevel()`, `onPlayerDeath()`, and screen transitions |
| State changes | `changeState()` only — called from `Screens.hpp` handlers and from `updateGame()`/`onPlayerDeath()` |

---

## 4. GAME STATE MACHINE

Ten states, defined in `GameState.hpp` (`STATE_COUNT` is a sentinel, not a state).

| State | Purpose | Entered from | Left by | Drawn | Input accepted | Update logic |
|---|---|---|---|---|---|---|
| `STATE_MENU` | Title screen | program start; ESC/Enter from Instructions, Credits, Game Over; Pause → "Abandon"; Victory ESC | Start Campaign → `STATE_STORY` (after `resetRun()` + `newRunPlayer()`); Instructions; Credits; Exit → `exit(0)` | backdrop, two banners, title, 4 buttons, best score, hint bar, fade | ↑/↓/W/S move selection, Enter activates, ESC quits, mouse hover + click | none beyond `uiTime` |
| `STATE_INSTRUCTIONS` | Controls and enemy weaknesses | Menu | ESC or Enter, or any click → Menu | panel of key bindings + "Know your enemy" | ESC, Enter, click | none |
| `STATE_CREDITS` | Team and course | Menu; Victory + Enter | ESC or Enter, or click → Menu | credits panel | ESC, Enter, click | none |
| `STATE_STORY` | Mission card before each level | Menu Start; after Path Choice; after Level Complete on levels 1→2, 2→3 | Enter/Space or click → `beginLevelFromStory()` → `startLevel()` → `STATE_PLAYING`; ESC → Menu (stops music) | mission number, level name, four story lines | Enter, Space, ESC, click | none |
| `STATE_PLAYING` | The game | Story; Pause resume/restart; Path Choice confirm; Game Over "R" | P/ESC → Pause; player death with no lives → Game Over; reach `gateX` (levels 1–2) or boss death (level 3) → Level Complete | full battlefield + HUD (see §7) | movement, jump, sword, bow, Q/E/R abilities, P/ESC, M; left/right mouse attack | `updateGame()` — the only state that simulates |
| `STATE_PAUSED` | Pause menu | P or ESC during play | Resume; Restart Level (`startLevel(currentLevel)`); Abandon → Menu (stops music); P/ESC resumes | frozen battlefield, dim overlay, panel, 3 buttons | ↑/↓, Enter, P/ESC, mouse hover + click | none — `gameUpdate()` returns early, so the world is genuinely frozen |
| `STATE_PATH_CHOICE` | Choose Blade or Bow | Level Complete after level 1 while `heroPath == 0` | Enter or click → `confirmPath()`: sets `heroPath`, `setHeroStage(2)`, `currentLevel = 2`, → `STATE_STORY` | two cards with stat lists, selection marker | ←/→, A/D, Enter, click | none |
| `STATE_LEVEL_COMPLETE` | End-of-mission summary | reaching `gateX` (levels 1–2); boss death (level 3) | Enter or click → `advanceAfterLevel()` | "FIELD WON", level name, score, lives, next-step line | Enter, click | none |
| `STATE_GAMEOVER` | Run lost | `onPlayerDeath()` when `lives` reaches 0 | "R" restarts the current level (score 0, lives restored); Enter/ESC → Menu | "THE BANNER FALLS", final score, level reached | R, Enter, ESC | none |
| `STATE_VICTORY` | Campaign won | `advanceAfterLevel()` when `currentLevel >= TOTAL_LEVELS` | Enter → Credits; ESC → Menu | light rays, banners, final score, path taken, best score | Enter, ESC | none |

### Actual flow diagram

```
                         program start
                              |
                              v
                        +-----------+
              +-------->|   MENU    |<--------------------+
              |         +-----+-----+                     |
              |          |    |    |                      |
              |  Instructions Credits  Exit -> exit(0)    |
              |          |    |                            |
              |          +----+ (ESC / Enter / click)      |
              |                                            |
              |  Start Campaign: resetRun() + newRunPlayer()|
              |                    |                        |
              |                    v                        |
              |              +-----------+                   |
              |         +--->|   STORY   |                   |
              |         |    +-----+-----+                   |
              |         |          | Enter/Space/click       |
              |         |          | startLevel(currentLevel)|
              |         |          v                         |
              |         |    +-----------+   P/ESC   +--------------+
              |         |    |  PLAYING  |<--------->|    PAUSED    |
              |         |    +-----+-----+  resume   +------+-------+
              |         |          |                        |
              |         |          |                 Abandon -> MENU
              |         |          |                 Restart -> startLevel()
              |         |          |
              |         |   +------+---------------------+
              |         |   |                            |
              |         |  reach gateX (L1, L2)     boss dies (L3)
              |         |  or boss death (L3)            |
              |         |   |                            |
              |         |   v                            v
              |         | +--------------------------------+
              |         | |        LEVEL COMPLETE          |
              |         | +----------------+---------------+
              |         |                  | Enter/click -> advanceAfterLevel()
              |         |     +------------+-------------+
              |         |     |                          |
              |         |  level 1 and heroPath == 0     currentLevel >= 3
              |         |     |                          |
              |         |     v                          v
              |         | +-------------+          +-----------+
              |         | | PATH CHOICE |          |  VICTORY  |
              |         | +------+------+          +-----+-----+
              |         |        | Enter: heroPath,      | Enter
              |         |        | stage 2, level = 2    v
              |         +--------+                 +-----------+
              |         |                          |  CREDITS  |
              |         | otherwise: currentLevel++ +-----+-----+
              |         +---------------------------------+     |
              |                                                 |
              |    player dies with lives == 0                   |
              |                    |                             |
              |                    v                             |
              |              +-----------+                       |
              +--------------| GAME OVER |  R -> PLAYING          |
                Enter/ESC    +-----------+  (same level)          |
                                                                  |
                             ESC/Enter from CREDITS --------------+
```

**`STORE` state: `[NOT IMPLEMENTED]`** — there is no shop screen. `player.gold` accumulates and is displayed on the HUD, but nothing spends it.

---

## 5. INPUT SYSTEM

### How input physically arrives
The framework registers GLUT callbacks in `iStart()`. Its handlers do **one thing only**: set `keyPressed[key] = 1` on key-down and `0` on key-up (and the same for `specialKeyPressed[]` for arrows/function keys). **There is no `iKeyboard()` callback in this iGraphics version.** Any tutorial code that defines one will never be called.

Separately, `iInitialize()` starts a Win32 timer that calls `fixedUpdate()` every `keyboardSamplingRate` milliseconds.

### `KEY_POLL_MS` — yes, it is used
`Config.hpp` defines `KEY_POLL_MS = 8` and `main()` passes it as the **fourth argument** to `iInitialize()`. It sets how often the framework calls `fixedUpdate()`. It exists because the framework only *polls* held-key state — a key pressed and released entirely between two polls is invisible to the game. At the framework default of 16 ms a very fast tap could be dropped; 8 ms halves that window. It cannot be eliminated without modifying the framework.

### The chain
```
GLUT key-down -> keyPressed[k] = 1
                        |
      every 8 ms:  fixedUpdate()
                        |
                 handleCurrentKeys()   (switch on gameState)
                        |
      +-----------------+-----------------+
      |                                   |
  menu/screen handlers              handlePlayingKeys()
  (keyTapped only)                        |
                                    readPlayerInput()
                                          |
                          keyHeld -> player.vx / facing
                          keyTapped -> jump, swing, arrow, ability flags
                        |
                 inputEndFrame()  copies current state into prevKey[]
```

### Key map as implemented

| Key | Where handled | Effect |
|---|---|---|
| `A` / `D` / `←` / `→` | `readPlayerInput()` (held) | sets `player.vx` and `player.facing` |
| `W` / `SPACE` / `↑` | `readPlayerInput()` (tapped) | jump, only when `player.onGround` |
| `J` | `readPlayerInput()` (tapped) | sword swing if `swingCooldown <= 0` |
| `K` | `readPlayerInput()` (tapped) | fires an arrow, requires `player.stage >= 2` |
| `Q` | `readPlayerInput()` (tapped) | sets `castHeal`, requires stage ≥ 2 and cooldown ready |
| `E` | `readPlayerInput()` (tapped) | sets `castSummon`, requires stage ≥ 3 |
| `R` | `readPlayerInput()` (tapped) | sets `castUlt`, requires stage ≥ 4 |
| `P` / `ESC` | `handlePlayingKeys()` | → `STATE_PAUSED` |
| `M` | `handlePlayingKeys()` | `musicToggle()` |
| `↑` / `↓` / `W` / `S` | `handleMenuKeys()`, `handlePauseKeys()` | move selection |
| `←` / `→` / `A` / `D` | `handlePathChoiceKeys()` | select Blade / Bow |
| `ENTER` | every menu-like handler | confirm |
| `ESC` | menu handlers | back, or quit from the main menu |
| `R` | `handleGameOverKeys()` | restart the current level |
| `SPACE` | `handleStoryKeys()` | begin the level |

### Mouse
- **Click** — `iMouse()` receives coordinates the framework has already y-flipped, so they match drawing coordinates directly. During `STATE_PLAYING`: left = sword, right = arrow (stage ≥ 2). In all other states left click → `handleCurrentClick()`, which tests the button rectangles for Menu, Pause and Path Choice, and treats a click as "continue" on Instructions, Credits, Story and Level Complete.
- **Hover** — `updateCurrentHover()` runs inside `iDraw()` and sets `menuSelected` / `pauseSelected` / `pathSelected` from `iMouseX`/`iMouseY`, so mouse and keyboard share one selection index and can never disagree.
- **Motion callbacks** — `iMouseMove()` and `iPassiveMouseMove()` are empty `[PLACEHOLDER]`.

---

## 6. TIMER / UPDATE SYSTEM

| Clock | Rate | Registered where | Responsible for | Must NOT do | Variables it writes |
|---|---|---|---|---|---|
| `iDraw()` | **Uncapped** — `glutIdleFunc(animFF)` posts a redisplay continuously, so it runs as fast as the machine allows | wired by `iStart()` | clearing the screen, hover tracking, drawing the current screen | must not change game state, move anything, or depend on being called at a fixed rate | (only `menuSelected`/`pauseSelected`/`pathSelected` via hover — see §16) |
| `fixedUpdate()` | every **8 ms** (`KEY_POLL_MS`) | Win32 timer started inside `iInitialize()` | reading the keyboard, turning holds into taps, dispatching per-state key handlers | must not run physics or draw | `player.vx`, `player.facing`, `swingTimer`, cast flags, menu selections, `gameState` (via handlers), `prevKey[]`, `prevSpecial[]` |
| `gameUpdate()` | every **20 ms** (`LOGIC_TICK_MS`) | `iSetTimer` in `main()` | the whole simulation; UI time and the fade counter, which advance in every state | must not draw | `uiTime`, `fadeTimer`, and through `updateGame()`: player, units, projectiles, effects, camera, score, `gameState` |
| `animationTick()` | every **120 ms** (`ANIM_TICK_MS`) | `iSetTimer` in `main()` | intended sprite-frame clock | — | `animFrame` only. **Nothing reads `animFrame`, so this clock currently has no visible effect** `[PLACEHOLDER]` |

**Why they are separate.** Rendering is uncapped and machine-dependent, so putting movement there would make the game run faster on faster computers. Input must be sampled more often than the simulation or quick taps are missed. The simulation must advance at a constant rate for physics and cooldowns to be predictable. The framework allows at most **10** `iSetTimer` timers; two are used.

Note that `gameUpdate()` advances `uiTime` *before* its early return, which is why menus keep animating and the fade keeps running while paused, even though the battlefield is frozen.

---

## 7. RENDERING SYSTEM

`iDraw()` → `iClear()` → `updateCurrentHover()` → `drawCurrentScreen()` → the state's draw function.

### Gameplay frame (`drawGameplay()` in `Game.hpp`)

```
glPushMatrix + glTranslatef(screen shake offset)
   1. drawParallaxBackground()   battlefield painting, 0.35x camera, alternate copies mirrored,
                                 tinted per level, then a haze rectangle
   2. drawGround()               darkened gradient band + scrolling ruts and stones
   3. drawObstacles()            barricade planks, braces, damage bar
   4. drawGate()                 the level-end banner on its pole
   5. drawPickups()              bobbing coins and flasks
   6. drawUnits()                enemies, companions, boss (shadow, sprite, health pip,
                                 pike shaft, windup flash)
   7. drawPlayer()               shadow, mirrored sprite, sword arc, stage-3 aura ring
   8. drawProjectiles()          arrows and shockwaves
   9. drawEffects()              sparks, damage numbers, rings, dust
glPopMatrix
  10. drawHUD()                  never shakes, so the interface stays readable
  11. level intro title card     only while levelIntroTimer > 0
```
Then `drawPlayingScreen()` adds **`drawFadeOverlay()`** last of all.

### Other screens
- **Menu / Instructions / Credits / Story / Path Choice / Victory**: `drawBackdrop()` (gradient sky, stars, moon with halo, hills, castle silhouette, campfires, stakes) → decorative banners where used → panels and text → `drawHintBar()` → `drawFadeOverlay()`.
- **Pause**: calls `drawPlayingScreen()` first so the frozen battlefield shows through, then a 72% dark overlay, then the panel and buttons, then the hint bar. It does not draw a second fade.
- **Game Over**: backdrop, red wash, panel with a red frame, text.

### Drawing rules in force
`iSetColor()` is RGB only; anything translucent goes through `fillRectAlpha()` or an explicit `beginBlend()`/`glColor4f()`/`endBlend()` block. Sprites always go through `drawSpriteTint()`/`drawSpriteEx()`, which enable blending and `GL_MODULATE`, sample only the `u1`/`v1` sub-rectangle, and restore `glColor4f(1,1,1,1)` afterwards.

---

## 8. ASSET SYSTEM

### Storage
```
demo/demo/Assets/player/joan_side_00.png       287 x 512 in a 512 x 512 canvas
demo/demo/Assets/ui/joan_portrait.png          235 x 512 in a 256 x 512 canvas
demo/demo/Assets/enemy/brute_idle_00.png       256 x 187 in a 256 x 256 canvas
demo/demo/Assets/background/field_orleans.png 1600 x 800 in a 2048 x 1024 canvas
```
Four sprites are loaded. `Assets/boss/`, `Assets/items/`, `Assets/effects/` exist but are empty — the boss reuses the enemy sprite with a red tint and a 2.35× scale, and items/effects are drawn with primitives.

### Loading
`loadAllAssets()` is called once in `main()`, **after `iInitialize()`** (a texture needs a live GL context) and **before `iStart()`** (which never returns). For each row of `ASSET_TABLE` it:
1. computes `u1 = w / texW`, `v1 = h / texH` — the fraction of the padded canvas the artwork occupies;
2. verifies the file exists with `fopen`, because `stbi_load` inside `iLoadImage` fails silently;
3. calls `iLoadImage(path)` and stores the returned texture ID;
4. prints `ok` or `MISSING` and a final count to the console.

Paths are **relative to the working directory**, which is the project folder when launched with F5 from the solution. Launching the `.exe` directly gives a different working directory and every sprite becomes a magenta box.

### From folder to screen
```
tools/source_art/joan_turnaround.jpg
        |  python tools/prepare_assets.py   (crop, background removal, trim, POT padding)
        v
demo/demo/Assets/player/joan_side_00.png  +  regenerated AssetList.hpp entry
        |  loadAllAssets()  ->  iLoadImage  ->  OpenGL texture, ID stored in gSprites[]
        v
drawPlayer() -> drawSpriteEx(SPR_PLAYER_JOAN_SIDE_00, ...) -> drawSpriteTint()
        |  binds texture, enables blending, GL_MODULATE, draws one quad
        v  using texcoords (0..u1, 0..v1) so canvas padding is never sampled
     screen
```

### Pipeline relationship
The Python tool and the C++ project meet at exactly **one** file: `AssetList.hpp`. The tool writes it; `Assets.hpp` reads it. The game never runs Python, and Visual Studio never builds it. Adding artwork is: drop the file in `tools/source_art/`, add a job to `tools/assets.json`, run the script, rebuild. The generated sprite ID is `SPR_<CATEGORY>_<FILENAME>` in upper case.

The pipeline's background removal uses three combined tests — flood fill inward from the border, a local neighbour-delta test (handles gradient backdrops), and an edge-block test (the fill cannot cross high-contrast outlines). That combination is what allowed grey-blue armour to survive on a grey backdrop.

### Helper functions
| Function | Used? |
|---|---|
| `drawSpriteTint()` | yes — background, units |
| `drawSpriteEx()` | yes — player, HUD portrait |
| `spriteAspect()` | yes — unit and player draw sizes |
| `drawMissingBox()` | yes — fallback path |
| `drawSprite()`, `drawSpriteH()`, `drawSpriteRect()`, `animSprite()`, `drawAnimH()` | **defined but never called** `[PLACEHOLDER]` — they exist for future multi-frame animation |

---

## 9. CURRENT GAMEPLAY MECHANISM

**Player** — `struct Player` with a single instance `player`. Hitbox is `PLAYER_W 46 × PLAYER_H 104` anchored at the feet; the sprite is drawn 1.45× taller with its width from the artwork aspect.

**Movement** — horizontal velocity is set from held keys each input tick, applied in `updatePlayer()`, then clamped to `[PLAYER_W/2, levelLength - PLAYER_W/2]`. Speed is `PLAYER_SPEED 4.5` plus 0.7 on the Bow path plus 0.6 at stage ≥ 3.

**Gravity** — `vy -= GRAVITY (0.62)` each tick, clamped to `TERMINAL_V 22`. Landing is checked against barricade tops first, then `GROUND_Y (96)`. Jump impulse is `PLAYER_JUMP 13.6` and only fires when `onGround`.

**Camera** — `updateCamera()` eases `cameraX` toward `player.x - SCREEN_WIDTH*0.36` at 12% per tick and clamps to `[0, levelLength - SCREEN_WIDTH]`.

**Enemy** — four ranks in one struct. Brute (46 HP), Pikeman (78 HP, reach 96, +100% arrow damage taken, −25% sword), Crossbow (30 HP, reach 420, shoots, +100% sword damage taken), Boss (900 HP, reach 130, ground shockwaves, three phases). Ranks are distinguished visually by tint, scale, and a drawn pike shaft. AI: sleep until Joan is within `AGGRO_RANGE 640`, walk toward her, stop inside reach, wind up (18 ticks; boss 26), strike, cool down 60 ticks (boss 70).

**Companions** — `U_COMPANION`, `side = SIDE_PLAYER`, summoned by `E`, live `ALLY_LIFETIME 1000` ticks, seek the nearest enemy, and deal +50% damage within `ALLY_AURA 220` of Joan.

**Combat** — sword creates a rectangle in front of Joan for `SWING_TICKS 16`; `swingId` increments per swing and each unit records `lastHitSwing`, so one swing damages a unit once. Arrows are pooled projectiles carrying a side and a damage value. Damage always flows through `damageUnit()` (units) or `damagePlayer()` (Joan).

**Collision** — all AABB, all in `Combat.hpp`, in this order: sword vs units and barricades → projectiles vs units/player/companions/barricades → unit melee vs player or vs enemies → body contact (capped 3–10 damage plus push-apart) → pickups → abilities. Barricades also block horizontal movement and can be stood on.

**Health** — Joan has `PLAYER_MAX_HP 100` plus 20 per stage above 1, `PLAYER_INVULN 45` ticks of mercy after a hit, and a red screen vignette below 30%.

**Score** — `score` rises from kills (25/40/35/750), coins (10) and broken barricades (5). `highScore` is updated at level completion, victory, and both game-over exits. Score is **not** reset by "Restart Level" in the pause menu.

**Collectibles** — coins (+10 score, +1 gold) and flasks (+30 health). Both bob and are collected by overlap.

**Level system** — three levels. `buildLevel()` sets length (5400 / 6600 / 4600), mood tint, gate position (`levelLength - 260`), barricades, coins and three flasks; `populateLevel()` places 13 / 18 / 10 enemies plus the boss on level 3. Levels 1–2 complete when `player.x >= gateX`; level 3 completes when the boss dies.

**Boss system** — spawned at `levelLength - 620` on level 3. Phases change at 2/3 and 1/3 health, each adding speed and firing a ring effect. Its arrival (waking from `US_SLEEP`) promotes Joan to stage 4. The HUD shows a dedicated boss bar with the phase number.

**Path selection** — after level 1 the player picks Blade (×1.45 sword, ×1.15 reach) or Bow (×1.5 arrow, ×0.75 sword, +0.7 speed). Stored in `heroPath`, used by `playerSwordDamage()`, `playerArrowDamage()`, `playerSpeed()`, `swordRect()`, and displayed on the HUD and Victory screen.

**Victory** — `advanceAfterLevel()` moves to `STATE_VICTORY` when `currentLevel >= TOTAL_LEVELS`.

**Game over** — `onPlayerDeath()` decrements `lives`; if any remain Joan respawns 260px back with 120 ticks of invulnerability and full health; otherwise `gameover.mp3` plays and the state becomes `STATE_GAMEOVER`.

**Store system** — `[NOT IMPLEMENTED]`. `player.gold` is incremented and displayed but is never spent; there is no shop state or upgrade screen.

**Other absent items** — no save/load, no difficulty setting, no options screen, no multi-frame sprite animation, no vertical platforms (barricades are the only thing that can be stood on).

---

## 10. DEBUG / TEST KEYS

**There are no debug or cheat keys in the current build.**

The Phase-1 placeholder keys (`N` finish level, `G` lose the run, `K` add 250 score) were part of the temporary gameplay stub and were **removed** when the real gameplay screen replaced it. `K` is now the bow. The complete current `handlePlayingKeys()` is:

```cpp
void handlePlayingKeys()
{
    readPlayerInput();
    if (keyTappedCh('p', 'P') || keyTapped(KEY_ESCAPE)) changeState(STATE_PAUSED);
    if (keyTappedCh('m', 'M')) musicToggle();
}
```

The only non-gameplay keys are:

| Key | Effect | Variable / state changed |
|---|---|---|
| `P` / `ESC` | pause | `gameState` → `STATE_PAUSED`, `fadeTimer` |
| `M` | music on/off | `gMusicOn`, closes the MCI alias |
| `R` (Game Over screen only) | restart the current level | `score`, `lives`, `player`, `gameState` |

---

## 11. IMPORTANT VARIABLES

| Variable | Type | File | Purpose | Values | Written by | Read by |
|---|---|---|---|---|---|---|
| `gameState` | `int` | GameState.hpp | current screen | `STATE_MENU` … `STATE_VICTORY` | `changeState()` only | every dispatcher, `gameUpdate()`, `iMouse()` |
| `previousState` | `int` | GameState.hpp | previous screen | same set | `changeState()` | **nobody** `[PLACEHOLDER]` |
| `fadeTimer` | `int` | GameState.hpp | fade-in countdown | 0…`FADE_TICKS` (14) | `changeState()`, `gameUpdate()` | `drawFadeOverlay()` |
| `uiTime` | `long` | GameState.hpp | universal animation clock | grows forever | `gameUpdate()` | banners, buttons, coins, campfires, stars, HUD pulses |
| `animFrame` | `long` | GameState.hpp | sprite-frame clock | grows forever | `animationTick()` | **nobody** `[PLACEHOLDER]` |
| `currentLevel` | `int` | GameState.hpp | mission number | 1…3 | `resetRun()`, `advanceAfterLevel()`, `confirmPath()`, `startLevel()` | level build, HUD, story, boss logic |
| `score` | `int` | GameState.hpp | run score | ≥ 0 | `damageUnit()`, `resolvePickups()`, `resolveSword()`, game-over restart | HUD, all end screens |
| `highScore` | `int` | GameState.hpp | best score this session | ≥ 0 | level complete, victory, game over | menu, victory |
| `lives` | `int` | GameState.hpp | remaining lives | 0…`START_LIVES` (3) | `resetRun()`, `onPlayerDeath()`, game-over restart | HUD, level complete |
| `heroPath` | `int` | GameState.hpp | chosen path | 0 none, 1 Blade, 2 Bow | `resetRun()`, `confirmPath()` | damage/speed functions, HUD, victory |
| `player` | `Player` | Player.hpp | all player state | — | `readPlayerInput()`, `updatePlayer()`, `Combat.hpp`, `Game.hpp` | everything gameplay |
| `player.stage` | `int` | Player.hpp | hero stage | 1…4 | `setHeroStage()`, `newRunPlayer()` | ability gates, damage, HUD |
| `player.swingId` | `int` | Player.hpp | swing identity | grows | `readPlayerInput()`, `iMouse()` | `resolveSword()` vs `unit.lastHitSwing` |
| `units[]` | `Unit[56]` | Units.hpp | enemies + companions + boss | — | `spawnUnit()`, `updateUnits()`, `damageUnit()`, `Combat.hpp` | drawing, collisions, HUD boss bar |
| `projectiles[]` | `Projectile[72]` | Projectiles.hpp | arrows, shockwaves | — | `spawnArrow()`, `spawnShock()`, `updateProjectiles()`, `Combat.hpp` | collisions, drawing |
| `obstacles[]` | `Obstacle[28]` | World.hpp | barricades | — | `buildLevel()`, `resolveSword()` | movement, projectiles, drawing |
| `pickups[]` | `Pickup[48]` | World.hpp | coins and flasks | — | `buildLevel()`, `addPickup()`, `resolvePickups()` | collisions, drawing |
| `effects[]` | `Effect[96]` | Effects.hpp | visual feedback | — | `fx*()`, `updateEffects()` | `drawEffects()` |
| `screenShake` | `double` | Effects.hpp | camera shake | 0…~20, decays ×0.86 | `shakeScreen()`, `updateEffects()` | `drawGameplay()` |
| `cameraX` | `double` | World.hpp | scroll position | 0…`levelLength - SCREEN_WIDTH` | `updateCamera()`, `startLevel()`, `onPlayerDeath()` | every world draw |
| `levelLength` | `double` | World.hpp | level width | 4600…6600 | `buildLevel()` | camera, clamps, HUD progress |
| `gateX` | `double` | World.hpp | finish line | `levelLength - 260` | `buildLevel()` | `updateGame()` win check, `drawGate()` |
| `levelKills` | `int` | World.hpp | kills this level | ≥ 0 | `buildLevel()`, `damageUnit()` | HUD |
| `levelKillGoal` | `int` | World.hpp | enemies placed | ≥ 0 | `populateLevel()` | **nobody** `[PLACEHOLDER]` |
| `bossAlive` | `int` | World.hpp | boss still standing | 0/1 | `buildLevel()`, `populateLevel()`, `damageUnit()` | `updateGame()` win check |
| `bossWoke` | `int` | Game.hpp | boss has activated | 0/1 | `startLevel()`, `updateGame()` | stage-4 promotion, win check |
| `levelIntroTimer` | `int` | Game.hpp | title card countdown | 0…150 | `startLevel()`, `updateGame()` | `drawGameplay()` |
| `gSprites[]` | `Sprite[]` | Assets.hpp | texture IDs + UVs | — | `loadAllAssets()` | all sprite drawing |
| `menuSelected`, `pauseSelected`, `pathSelected` | `int` | Screens.hpp | highlighted item | index | key handlers **and** `updateCurrentHover()` (from `iDraw`) | the matching draw and activate functions |
| `prevKey[]`, `prevSpecial[]` | `int[512]` | Input.hpp | last frame's keys | 0/1 | `inputEndFrame()` | `keyTapped()`, `specialTapped()` |
| `gMusicOn`, `gSoundOn`, `gMusicAlias` | `int`, `char[32]` | Audio.hpp | audio switches | 0/1, alias name | `musicToggle()`, `musicPlay()`, `musicStop()` | `sfx()`, `musicPlay()` |
| `storyTimer` | `int` | Screens.hpp | — | 0 | nobody | **nobody** `[PLACEHOLDER]` |

---

## 12. IMPORTANT FUNCTIONS

| Function | File | Purpose | Called by | Calls | When |
|---|---|---|---|---|---|
| `main()` | iMain.cpp | entry point | C runtime | `srand`, `iSetTimer`×2, `iInitialize`, `loadAllAssets`, `iStart` | once |
| `iDraw()` | iMain.cpp | render a frame | framework idle loop | `iClear`, `updateCurrentHover`, `drawCurrentScreen` | uncapped |
| `fixedUpdate()` | iMain.cpp | sample input | framework key timer | `handleCurrentKeys`, `inputEndFrame` | every 8 ms |
| `gameUpdate()` | iMain.cpp | simulation clock | `iSetTimer` | `updateGame` (only while PLAYING) | every 20 ms |
| `animationTick()` | iMain.cpp | animation clock | `iSetTimer` | — | every 120 ms |
| `iMouse()` | iMain.cpp | mouse buttons | framework | `spawnArrow`, `sfx`, `handleCurrentClick` | on click |
| `loadAllAssets()` | Assets.hpp | upload textures | `main()` | `fileExists`, `iLoadImage`, `printf` | once, after `iInitialize` |
| `drawSpriteTint()` | Assets.hpp | draw one sprite | all sprite drawing | raw GL calls | many times per frame |
| `changeState()` | GameState.hpp | switch screen | screen handlers, `updateGame`, `onPlayerDeath` | — | on every transition |
| `handleCurrentKeys()` | Screens.hpp | dispatch keys | `fixedUpdate` | per-state handler | every 8 ms |
| `drawCurrentScreen()` | Screens.hpp | dispatch drawing | `iDraw` | per-state draw fn | every frame |
| `updateCurrentHover()` | Screens.hpp | mouse highlight | `iDraw` | `pointInRect` | every frame |
| `handleCurrentClick()` | Screens.hpp | dispatch clicks | `iMouse` | activate functions, `changeState` | on click |
| `beginLevelFromStory()` | Screens.hpp | start a mission | story key handler and click path | `startLevel`, `changeState` | leaving STORY |
| `advanceAfterLevel()` | Screens.hpp | decide what follows a win | level-complete handler/click | `changeState` | on Enter |
| `confirmPath()` | Screens.hpp | lock in Blade/Bow | path-choice handler/click | `setHeroStage`, `sfx`, `changeState` | on Enter |
| `startLevel()` | Game.hpp | build a mission | `beginLevelFromStory`, pause restart, game-over restart | `clearUnits`, `clearProjectiles`, `clearEffects`, `buildLevel`, `populateLevel`, `resetPlayer`, `setHeroStage`, `musicPlay` | at each level start |
| `updateGame()` | Game.hpp | one simulation tick | `gameUpdate` | `updatePlayer`, `updateUnits`, `updateProjectiles`, `updateEffects`, `resolveAllCollisions`, `updateCamera`, `onPlayerDeath`, `changeState` | every 20 ms while playing |
| `drawGameplay()` | Game.hpp | render the battlefield | `drawPlayingScreen` | all world/unit/effect draws, `drawHUD` | every frame while playing/paused |
| `onPlayerDeath()` | Game.hpp | spend a life | `updateGame` | `resetPlayer`, `setHeroStage`, `musicPlay`, `changeState` | when death animation ends |
| `readPlayerInput()` | Player.hpp | translate keys to intent | `handlePlayingKeys` | `spawnArrow`, `sfx`, `fxDust` | every 8 ms while playing |
| `updatePlayer()` | Player.hpp | physics and timers | `updateGame` | — | every 20 ms |
| `drawPlayer()` | Player.hpp | draw Joan | `drawGameplay` | `drawSpriteEx`, `fillRectAlpha`, GL calls | every frame |
| `damagePlayer()` / `healPlayer()` | Player.hpp | change health | Combat, pickups, abilities | `fx*`, `sfx`, `shakeScreen` | on hit/heal |
| `spawnUnit()` | Units.hpp | create a unit | `populateLevel`, `resolveAbilities` | — | level start, on summon |
| `updateUnits()` | Units.hpp | AI for everything | `updateGame` | `spawnArrow`, `spawnShock`, `fx*`, `sfx`, `shakeScreen` | every 20 ms |
| `damageUnit()` | Units.hpp | hurt a unit, award score | Combat | `fx*`, `sfx`, `addPickup` | on every hit |
| `resolveAllCollisions()` | Combat.hpp | all collisions | `updateGame` | the six resolve functions | every 20 ms |
| `updateCamera()` | Combat.hpp | scroll | `updateGame` | `clampd` | every 20 ms |
| `buildLevel()` | World.hpp | terrain and loot | `startLevel` | `addObstacle`, `addPickup`, `setLevelMood` | at level start |
| `populateLevel()` | Game.hpp | place the army | `startLevel` | `spawnUnit`, `countLiveEnemies` | at level start |
| `drawHUD()` | HUD.hpp | interface | `drawGameplay` | `drawAbilityIcon`, UI helpers | every frame |
| `musicPlay()` / `sfx()` | Audio.hpp | audio | level start, death, events | `mciSendStringA` / `PlaySoundA` | as needed |

---

## 13. GAME FLOW — launch to victory

```
1.  Program starts, main() runs
       srand(time(0))
       iSetTimer(20, gameUpdate)        <- registered BEFORE the window
       iSetTimer(120, animationTick)
2.  iInitialize(1200, 675, title, 8)
       creates the GLUT window, sets glOrtho with the origin bottom-left,
       starts the 8 ms keyboard timer that drives fixedUpdate()
3.  loadAllAssets()
       four PNGs become OpenGL textures; a load report prints to the console
4.  iStart()
       wires the GLUT callbacks, enables alpha testing, enters glutMainLoop —
       never returns
5.  STATE_MENU
       iDraw paints the night backdrop and buttons; fixedUpdate reads the keys
6.  Player picks START CAMPAIGN
       resetRun()      -> level 1, score 0, lives 3, heroPath 0
       newRunPlayer()  -> stage 1, gold 0, Joan reset to x = 140
       changeState(STATE_STORY)
7.  STATE_STORY, Enter
       beginLevelFromStory() -> startLevel(1)
           builds the 5400px field, places 13 enemies, starts background.mp3
       changeState(STATE_PLAYING)
8.  STATE_PLAYING  (the only simulating state)
       every 8 ms   : keys -> player intent
       every 20 ms  : player, units, projectiles, effects, collisions, camera
       every frame  : world -> HUD -> fade
       P at any time -> STATE_PAUSED -> resume, restart or abandon
9.  Joan reaches gateX (5140)
       highScore updated, music stopped, sfx("levelup")
       changeState(STATE_LEVEL_COMPLETE)
10. Enter -> advanceAfterLevel()
       level 1 and no path chosen yet -> STATE_PATH_CHOICE
11. Path Choice, Enter
       heroPath = Blade or Bow, setHeroStage(2), currentLevel = 2
       changeState(STATE_STORY)
12. Story -> startLevel(2)
       6600px, 18 enemies including pikemen and crossbows, dusk tint,
       setHeroStage(2) gives 120 max HP and unlocks K and Q
13. Reach the banner -> LEVEL COMPLETE -> Enter -> currentLevel = 3 -> STORY
14. startLevel(3)
       4600px, 10 enemies, storm tint, stage 3 (unlocks E), boss at x = 3980
15. Boss wakes when Joan comes within 640px
       bossWoke = 1, setHeroStage(4) -> unlocks R, "STAGE 4" floats up
       HUD shows the boss bar; phases escalate at 600 and 300 HP
16. Boss dies
       bossAlive = 0 -> changeState(STATE_LEVEL_COMPLETE)
17. Enter -> advanceAfterLevel(): currentLevel >= TOTAL_LEVELS
       changeState(STATE_VICTORY)
18. Victory screen -> Enter -> CREDITS -> Enter/ESC -> MENU

At any point, dying with lives left respawns Joan 260px back;
dying with no lives left plays gameover.mp3 and goes to STATE_GAMEOVER.
```

---

## 14. DEPENDENCY MAP

```
iMain.cpp
├── iGraphics.h            (framework — must be included first)
│   ├── glut.h  -> also links winmm.lib via #pragma
│   ├── glaux.h
│   └── stb_image.h
├── Config.hpp                                   (leaf, depends on nothing)
├── Utils.hpp
│   └── Config.hpp
├── GameState.hpp
│   └── Config.hpp
├── Input.hpp
│   └── Config.hpp                (+ reads iGraphics key arrays)
├── Assets.hpp
│   ├── Config.hpp
│   ├── Utils.hpp
│   └── AssetList.hpp             (GENERATED by tools/prepare_assets.py)
├── UI.hpp
│   ├── Config.hpp / Utils.hpp / GameState.hpp
├── Audio.hpp
│   ├── windows.h / mmsystem.h
│   └── Config.hpp
├── World.hpp
│   ├── Config / Utils / GameState / Assets / Audio
│   └── (calls UI.hpp helpers — relies on UI being included first)
├── Effects.hpp
│   └── Config / Utils / World
├── Projectiles.hpp
│   └── Config / Utils / World / Effects
├── Player.hpp
│   └── Config / Utils / GameState / Input / Assets / Audio / World / Effects / Projectiles
├── Units.hpp
│   └── Config / Utils / Assets / Audio / World / Effects / Projectiles / Player
├── Combat.hpp
│   └── Config / Utils / GameState / Audio / World / Effects / Projectiles / Player / Units
├── HUD.hpp
│   └── Config / Utils / GameState / UI / Assets / Player / Units / World
├── Game.hpp
│   └── Config / Utils / GameState / UI / Assets / Audio / World / Effects
│       / Projectiles / Player / Units / Combat / HUD
└── Screens.hpp
    └── Config / Utils / GameState / Input / UI / Game
```

**Include order matters.** This is a single-translation-unit project: the headers define globals and non-inline functions, so they can be included by `iMain.cpp` only. `World.hpp` uses `UI.hpp` helpers without including it and relies on `iMain.cpp` including `UI.hpp` first. Adding a second `.cpp` that includes any of these headers would produce duplicate-symbol link errors.

---

## 15. HOW TO ADD FUTURE FEATURES

| Feature | Where it belongs |
|---|---|
| New player move / attack | key detection in `readPlayerInput()`, physics in `updatePlayer()`, visuals in `drawPlayer()`, constants in `Config.hpp` |
| New enemy type | add to `enum UnitType`, then add a case to `unitMaxHp/unitSpeedOf/unitScaleOf/unitDamageOf/unitScoreOf/unitReachOf/unitTint/unitName`, behaviour in `updateUnits()`, place it in `populateLevel()` |
| New collision rule | a new `resolveXxx()` in `Combat.hpp` called from `resolveAllCollisions()` — nowhere else |
| New level | extend `TOTAL_LEVELS`, add a case to `levelName()`, a branch in `buildLevel()` and one in `populateLevel()` |
| New asset | drop the file in `tools/source_art/`, add a job to `tools/assets.json`, run `python tools/prepare_assets.py`, rebuild; use the generated `SPR_*` id |
| Multi-frame animation | name files `name_00.png`, `name_01.png`; the pipeline groups them into an `ANIM_*` entry; draw with the already-written `drawAnimH(ANIM_x, animFrame, ...)` — this is what `animFrame` was intended for |
| New sound | put the WAV in `Audios/sfx/` and call `sfx("name")`; music through `musicPlay()` |
| New game state | add to `enum GameStateId`, write `drawXxxScreen()` + `handleXxxKeys()` in `Screens.hpp`, add cases to `drawCurrentScreen()`, `handleCurrentKeys()`, and if it has buttons to `updateCurrentHover()` and `handleCurrentClick()` |
| **Store / upgrade screen** | a new state as above; spend `player.gold`; the natural hook is inside `advanceAfterLevel()` before moving on to the next `STATE_STORY` |
| New HUD element | `drawHUD()` in `HUD.hpp` |
| New particle/feedback | a new `FX_*` in `Effects.hpp` with a spawner and a case in `drawEffects()` — note `FX_SHOCK` already has a draw case and no spawner |
| Tuning difficulty | `Config.hpp` for player/global values, `unitMaxHp()` and friends for enemies, `buildLevel()`/`populateLevel()` for density |
| Debug keys | `handlePlayingKeys()` in `Screens.hpp` |

---

## 16. CURRENT LIMITATIONS

### Framework limitations (cannot be fixed without editing iGraphics)
1. **No key event callback.** Only polled key arrays, so an extremely brief tap between two 8 ms polls is invisible. `KEY_POLL_MS` mitigates but cannot eliminate this.
2. **`iDraw()` is uncapped**, so frame rate varies by machine. All timing therefore lives on the 20 ms timer, and nothing in the draw path may depend on frame rate.
3. **Maximum 10 `iSetTimer` timers.** Two are in use.
4. **`iShowImage()` is unusable for this game** (no blending, no flip, whole-texture sampling), which is why `Assets.hpp` draws its own quads.
5. **`iLoadImage()` fails silently** on a missing file, hence the explicit `fileExists()` check.
6. **OpenGL 1.1 requires power-of-two textures**, which is why the pipeline pads every export.
7. **Single translation unit.** Headers define globals; only `iMain.cpp` may include them.
8. **Working-directory dependence.** The game must be launched from Visual Studio (working directory = project folder) or all asset and audio paths fail.

### Current implementation limitations
9. **Enemies never target companions.** `findTarget()` returns "the player" for every enemy, and `resolveUnitAttacks()` only tests enemy melee against `playerRect()`. Companions can be hit by enemy *arrows* but never by enemy melee, and they cannot pull aggro.
10. **`updateCurrentHover()` mutates selection state from inside `iDraw()`.** It is harmless because it is idempotent for a given cursor position, but it does mean the render path writes game state, which contradicts the rule stated elsewhere in the code comments.
11. **`drawGameplay()` consumes random numbers** (screen shake) during rendering, so the RNG sequence depends on frame rate.
12. **Restarting a level from the pause menu keeps the accumulated score**; only the game-over restart zeroes it.
13. **Lives are never restored between levels** — only `resetRun()` (menu start) and the game-over restart reset them.
14. **`heroPath` survives a game-over restart** but stage is recomputed from `currentLevel`, so a restart can produce stage/path combinations that differ from the first playthrough of that level.
15. **The level-3 gate is decorative.** Level 3 completes on boss death only; walking past the gate does nothing. If the boss never woke, the level could not be completed.
16. **No frame-independent movement.** Everything assumes the 20 ms tick actually fires at 20 ms; under heavy load the game slows rather than skips.
17. **Barricades are the only standable surface.** There are no platforms, pits, or vertical level design.
18. **Pool overflows fail silently.** `spawnUnit()` returns `0` when `units[]` is full and most callers ignore the result.

### Placeholder / dead code
19. `animationTick()` increments `animFrame`, which **nothing reads** — the animation clock currently has no effect.
20. `drawSprite()`, `drawSpriteH()`, `drawSpriteRect()`, `animSprite()`, `drawAnimH()` are defined and never called.
21. `iMouseMove()` and `iPassiveMouseMove()` are empty.
22. `FX_SHOCK` has a draw case but no spawner. `OB_CRATE` is declared and never used.
23. `previousState`, `levelKillGoal`, `storyTimer` are written (or declared) and never read.
24. `Projectile::gravity` is always 0.
25. `Assets/boss/`, `Assets/items/`, `Assets/effects/` are empty folders.
26. Legacy files still in the tree: `Balloon.hpp`, `bitmap_loader.h`, `Images/`, `iMain_boxdemo_backup.txt`, `BubblePop Game.vcxproj`, `demo.vcxproj.bak`.
27. `getEuclideanDistance()` in `Utils.hpp` is unused by the game.

### Future work (explicitly not implemented)
28. Store / upgrade screen for spending gold.
29. Save/load, options screen, difficulty selection.
30. Multi-frame sprite animation (the pipeline and the C++ helpers already support it; only the artwork and the call sites are missing).
31. Companion aggro and enemy target selection.
32. `MSB8003` build warning: the project does not resolve `WindowsSDKDir` from the registry. The build succeeds because explicit SDK include/library paths are hard-coded in `demo.vcxproj`; those paths are machine-specific and would need editing on another computer.

---

## 17. DEVELOPER QUICK REFERENCE

| I want to change… | Go to |
|---|---|
| Menu layout, labels, actions | `Screens.hpp` → `menuLabels[]`, `menuItemRect()`, `activateMenuItem()`, `drawMenuScreen()` |
| Player speed, jump, health, damage | `Config.hpp` constants; formulas in `playerSpeed()`, `playerSwordDamage()`, `playerArrowDamage()` (`Player.hpp`) |
| Player physics or animation | `updatePlayer()` and `drawPlayer()` (`Player.hpp`) |
| Add or change a game state | `enum GameStateId` (`GameState.hpp`) + draw/handler pair + three dispatchers (`Screens.hpp`) |
| Key bindings | `readPlayerInput()` (`Player.hpp`) for gameplay; `handle*Keys()` (`Screens.hpp`) for screens |
| Mouse behaviour | `iMouse()` (`iMain.cpp`), `handleCurrentClick()` / `updateCurrentHover()` (`Screens.hpp`) |
| Draw order of the battlefield | `drawGameplay()` (`Game.hpp`) |
| Menu backdrop, buttons, panels, fonts | `UI.hpp` |
| Sprite drawing, tinting, flipping | `drawSpriteTint()` (`Assets.hpp`) |
| Animation timing | `animationTick()` (`iMain.cpp`) and `ANIM_TICK_MS` (`Config.hpp`) — remember `animFrame` is currently unread |
| Add or replace artwork | `tools/assets.json` + `python tools/prepare_assets.py`, then rebuild |
| Score values | `SCORE_*` in `Config.hpp`; awarded in `damageUnit()` (`Units.hpp`) and `resolvePickups()` (`Combat.hpp`) |
| Level length, barricades, coins | `buildLevel()` (`World.hpp`) |
| Enemy counts and placement | `populateLevel()` (`Game.hpp`) |
| Enemy stats and AI | the `unit*Of()` table functions and `updateUnits()` (`Units.hpp`) |
| Boss behaviour and phases | `updateUnits()` boss branch (`Units.hpp`); spawn in `populateLevel()`; bar in `drawHUD()` |
| Any collision | `Combat.hpp` — and nowhere else |
| Camera feel | `updateCamera()` (`Combat.hpp`) |
| HUD contents | `drawHUD()` (`HUD.hpp`) |
| Win / lose conditions | `updateGame()` (`Game.hpp`) and `advanceAfterLevel()` (`Screens.hpp`) |
| Music and sound | `Audio.hpp`; triggers in `startLevel()`, `onPlayerDeath()`, and at each event site |
| Debug keys | `handlePlayingKeys()` (`Screens.hpp`) — currently there are none |
| Window size | `SCREEN_WIDTH` / `SCREEN_HEIGHT` (`Config.hpp`) |
| Tick rates | `LOGIC_TICK_MS`, `ANIM_TICK_MS`, `KEY_POLL_MS` (`Config.hpp`) |

