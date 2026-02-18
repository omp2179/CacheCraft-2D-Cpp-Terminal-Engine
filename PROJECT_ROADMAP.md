# 🎮 MINECRAFT 2D — Complete Project Roadmap

> **Inspiration:** [York by kuratus89](https://github.com/kuratus89/york) — A 2D terminal Minecraft with infinite worlds, mining, building, inventory, cheats, story mode, and save/load. Built from scratch in C++ with only standard libraries.

> **Our Goal:** Build a game like York, but with **advanced CS/DSA concepts** integrated at every layer — making this not just a game, but a showcase of real engineering.

---

## 🎯 Target Project Description (Resume)

> • Built infinite chunked world game with Perlin/FBM terrain, mining/building, mobs, inventory, save/load on Windows console.
>
> • Designed OOP engine (Chunk/World/ScreenBuffer, window stack, 60 FPS loop) + SoA mobs with BFS pathfinding (3× faster than AoS).
>
> • Added Bloom filters (95% fewer chunk lookups) + DP mining optimization, validated with benchmarks.

**Every phase below maps directly to a claim in this description.**

---

## 📊 Overall Progress: ~25%

```
Engine Core    ████████████████████░░░░░░░░░░░░░░░░░░░░  ~95%
Gameplay       ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  ~0%
Advanced CS    ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  ~0%
Story & Polish ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  ~0%
```

---

## ✅ COMPLETED PHASES

### Phase 0: Project Setup
- [x] Folder structure (`src/`, `include/`, `build/`)
- [x] `compile.bat` build script (g++ with C++17)
- [x] Verify "Hello World" compiles and runs

### Phase 1: Core Data Types
- [x] `Coord` struct with custom hash (`CoordHash`) for `unordered_map`
- [x] `BlockType` enum with display helpers
- [x] `Pixel` struct — char + Color for terminal rendering
- [x] `Color` — ANSI terminal colors
- [x] All tests passing

**Resume claim:** ✅ *"OOP engine (Chunk/World/ScreenBuffer)"*

### Phase 2: World Engine
- [x] `Chunk` (32×32) — contiguous `std::array` storage, RAII terrain generation
- [x] `World` — infinite chunk map with lazy loading via `unordered_map<Coord, unique_ptr<Chunk>>`
- [x] `Terrain` — FBM noise (4-octave), smooth interpolation, ore distribution
- [x] Deterministic generation (same seed = same world)
- [x] All tests passing

**Resume claim:** ✅ *"infinite chunked world with Perlin/FBM terrain"*

### Phase 3: Rendering & Game Loop
- [x] `ScreenBuffer` — 80×24 double-buffered terminal rendering with ANSI escape codes
- [x] `Input` — non-blocking keyboard via `_kbhit()`/`_getch()` with buffer drain
- [x] Game loop (input → update → render @ 20 FPS)
- [x] Camera system — player-centered scrolling
- [x] Gravity — fall every 3 frames, jump 3 blocks from ground
- [x] Collision detection — can't walk through solid blocks
- [x] All tests passing

**Resume claim:** ✅ *"Windows console"* — partially covers *"60 FPS loop"* (upgrade later)

---

## 🔜 UPCOMING PHASES

### Phase 4: Mining & Building
> Resume: *"mining/building"*

- [ ] **Mining:** Shift+Arrow to break blocks in a direction
- [ ] Broken blocks become AIR in the world
- [ ] Broken blocks go into player inventory
- [ ] **Building:** Arrow keys to place selected block adjacent to player
- [ ] **Facing direction:** Track which way player faces (left/right)

---

### Phase 5: Inventory & Window Stack
> Resume: *"inventory"* + *"window stack"*

- [ ] Inventory data structure (block type → count map)
- [ ] **E key** opens inventory screen
- [ ] W/S to scroll items, Enter to select active block
- [ ] HUD shows selected block type and count
- [ ] **Window Stack System:** Each screen (game, inventory, pause) is a window pushed/popped from a `std::stack` — topmost renders

---

### Phase 6: Cave Generation
> Resume: *"Perlin/FBM terrain"* (extends terrain)

- [ ] **2D Perlin noise caves** — second noise function with threshold to carve cave networks
- [ ] Deeper caves = rarer ores (Diamond only below depth 20+)
- [ ] Underground pockets and open caverns

---

### Phase 7: Surface Decoration
- [ ] **Trees** — trunk blocks (3-5 tall) + leaf canopy
- [ ] Flowers and tall grass (cosmetic blocks)
- [ ] Variable terrain biomes (flat plains vs hilly mountains)

---

### Phase 8: Mobs (Enemies + AI)
> Resume: *"mobs"* + *"SoA mobs with BFS pathfinding (3× faster than AoS)"*

This is a **critical phase** — it delivers THREE resume claims at once.

- [ ] **Mob base class** — position, HP, AI state, sprite
- [ ] **Zombie mob** — hostile, chases player
- [ ] **AoS mob storage (baseline):**
  ```cpp
  // Array of Structs — each mob is a struct with all fields
  struct Mob { int x, y, hp; MobType type; int ai_state; };
  std::vector<Mob> mobs;  // iterate = cache misses (fields interleaved)
  ```
- [ ] **SoA mob storage (optimized):**
  ```cpp
  // Struct of Arrays — each field is a separate array
  struct MobStorage {
      std::vector<int> x, y, hp;         // position-only iteration = cache hits!
      std::vector<MobType> type;
      std::vector<int> ai_state;
  };
  ```
- [ ] **BFS pathfinding** — mobs find path to player through terrain
  - BFS on 2D grid, using `std::queue`
  - Mobs navigate around obstacles (stone, dirt)
  - Path cached and recomputed every N frames
- [ ] **AoS vs SoA benchmark:**
  - Spawn 1000+ mobs, measure update loop time
  - **Target: SoA 3× faster than AoS** (due to cache-line efficiency)
  - Print benchmark results to console

---

### Phase 9: Pause Menu & Cheats
> Resume: supports polish and usability

- [ ] **Enter key** → pause menu (push to window stack)
- [ ] Resume / Save / Load / Cheats / Quit options
- [ ] **Cheat Menu:**
  - [ ] Speed boost (increase movement speed)
  - [ ] Spectator mode (fly through blocks, ignore collision + gravity)
  - [ ] Give diamonds (add to inventory)
  - [ ] God mode (no fall damage, no mob damage)
  - [ ] Teleport to coordinates

---

### Phase 10: Health & Damage
- [ ] HP system (100 HP, shown in HUD)
- [ ] **Fall damage** — proportional to fall distance (>4 blocks)
- [ ] **Mob damage** — mobs deal damage on contact
- [ ] Hearts/HP bar rendering
- [ ] Death screen → respawn at spawn point

---

### Phase 11: Save/Load System
> Resume: *"save/load"*

- [ ] **Serialize world** to binary file (chunk positions + block data)
- [ ] **Serialize player** (position, HP, inventory)
- [ ] **Serialize mobs** (positions, HP, AI state)
- [ ] Save/Load from pause menu
- [ ] Multiple save slots

---

### Phase 12: 60 FPS Upgrade
> Resume: *"60 FPS loop"*

- [ ] Upgrade `Sleep(50)` → `Sleep(16)` (~60 FPS)
- [ ] Delta-time based physics (decouple physics from frame rate)
- [ ] Gravity/jump use delta-time instead of frame counting
- [ ] Benchmark: measure actual FPS, display in debug HUD
- [ ] Optimize render loop for 60 FPS (minimize string allocations)

---

### Phase 13: Story Mode
- [ ] NPC dialogues (text boxes on screen)
- [ ] Quest system (collect X diamonds, reach Y location)
- [ ] Story progression triggers
- [ ] *Story details TBD*

---

### Phase 14: Title Screen & Polish
- [ ] ASCII art title screen
- [ ] Main menu (New Game / Load / Quit)
- [ ] Smooth transitions between screens

---

## 🧠 ADVANCED CS/DSA INTEGRATION

Each advanced feature directly supports a resume claim — with **benchmarks to prove it**.

### 1. Bloom Filter — 95% Fewer Chunk Lookups
> Resume: *"Bloom filters (95% fewer chunk lookups)"*

- [ ] **Where:** Fast "has this chunk been generated?" pre-check BEFORE hitting `unordered_map`
- [ ] **How:** Multiple hash functions → set bits in a bit array → O(1) membership test
- [ ] **Implementation:**
  - `BloomFilter` class with configurable size and hash count
  - Inserted when chunk is created, checked before map lookup
  - If bloom says NO → skip map (guaranteed correct)
  - If bloom says YES → check map (might be false positive)
- [ ] **Benchmark:**
  - Generate 10,000 chunk lookups (mix of existing and non-existing)
  - Measure: map lookups WITH bloom vs WITHOUT bloom
  - **Target: 95% fewer map lookups** (most queries are for non-existing chunks during exploration)
  - Print results: "Bloom filter eliminated X% of map lookups"

### 2. SoA vs AoS — 3× Faster Mob Updates
> Resume: *"SoA mobs with BFS pathfinding (3× faster than AoS)"*

- [ ] **Where:** Mob storage and update loop
- [ ] **Why AoS is slow:**
  ```
  Mob[0]: {x, y, hp, type, ai_state}  → 20 bytes
  Mob[1]: {x, y, hp, type, ai_state}  → 20 bytes
  Cache line loads ALL fields, but position update only needs x,y
  → 60% of loaded data is WASTED
  ```
- [ ] **Why SoA is fast:**
  ```
  x_array: [x0, x1, x2, x3, ...]  → contiguous ints
  y_array: [y0, y1, y2, y3, ...]  → contiguous ints
  Position update iterates x_array, y_array only
  → 100% of cached data is USED → 3× faster
  ```
- [ ] **Benchmark:**
  - 1000 mobs, 10000 update cycles
  - AoS: iterate `vector<Mob>`, update positions
  - SoA: iterate `vector<int> x`, `vector<int> y`, update positions
  - **Target: SoA ≥ 3× faster**

### 3. BFS Pathfinding — Mob Navigation
> Resume: *"BFS pathfinding"*

- [ ] **Where:** Mobs finding path to player through 2D terrain
- [ ] **How:** BFS from mob position, expanding to adjacent AIR blocks, until reaching player
- [ ] **DSA concepts:** Queue-based graph traversal, visited set, path reconstruction
- [ ] **Implementation:**
  - `std::queue<Coord>` for BFS frontier
  - `std::unordered_set<Coord>` for visited
  - Reconstruct path via parent map
  - Mob follows path one step per N frames
- [ ] **Optimization:** Cache paths, recompute every 30 frames (not every frame)
- [ ] **Benchmark:** nodes explored vs path length, average pathfinding time

### 4. DP Mining Optimization
> Resume: *"DP mining optimization"*

- [ ] **Where:** Optimal mining path — "what's the most valuable path to mine through a region?"
- [ ] **How:** Given a grid section, use DP to find the path that maximizes ore value while minimizing blocks broken
- [ ] **DSA concepts:** 2D dynamic programming, optimal substructure, memoization
- [ ] **Implementation:**
  - `dp[y][x]` = maximum ore value reachable from position (x, y) moving down/left/right
  - Each ore has a value: Diamond=10, Gold=5, Iron=2, Stone=0
  - Recurrence: `dp[y][x] = value(x,y) + max(dp[y+1][x-1], dp[y+1][x], dp[y+1][x+1])`
  - Highlight optimal mining path on screen (debug/cheat mode)
- [ ] **Benchmark:** compare DP optimal path value vs greedy path value

---

## 🏗️ Architecture Overview

```
┌─────────────────────────────────────────────────────┐
│                    WINDOW STACK                      │
│  Title Menu ←→ Game World ←→ Inventory ←→ Pause     │
│       std::stack<Window*> — top renders              │
├─────────────────────────────────────────────────────┤
│                 GAME ENGINE (60 FPS)                 │
│  Input → Physics → Mob AI → World Update → Render   │
├─────────────────────────────────────────────────────┤
│                    WORLD ENGINE                      │
│  World → BloomFilter → ChunkMap → Chunks → Blocks   │
│              (95% fewer lookups)                     │
├─────────────────────────────────────────────────────┤
│                  MOB ENGINE (SoA)                    │
│  SoA Storage → BFS Pathfinding → AI State Machine   │
│       (3× faster than AoS, benchmarked)             │
├─────────────────────────────────────────────────────┤
│                 OPTIMIZATION LAYER                   │
│  DP Mining Path → Bloom Chunk Check → SoA Iteration │
│            (all benchmarked and validated)           │
├─────────────────────────────────────────────────────┤
│                    DATA LAYER                        │
│  Coord → BlockType → Pixel → Inventory → Save/Load  │
└─────────────────────────────────────────────────────┘
```

---

## 🎯 Resume Claim → Phase Mapping

| Resume Claim | Phase(s) | Status |
|---|---|---|
| *"infinite chunked world"* | Phase 2 | ✅ Done |
| *"Perlin/FBM terrain"* | Phase 2 + 6 | ✅ Partially (caves pending) |
| *"mining/building"* | Phase 4 | ❌ Next |
| *"mobs"* | Phase 8 | ❌ Planned |
| *"inventory"* | Phase 5 | ❌ Planned |
| *"save/load"* | Phase 11 | ❌ Planned |
| *"Windows console"* | Phase 0-3 | ✅ Done |
| *"OOP engine (Chunk/World/ScreenBuffer)"* | Phase 1-3 | ✅ Done |
| *"window stack"* | Phase 5 | ❌ Planned |
| *"60 FPS loop"* | Phase 12 | ❌ Planned |
| *"SoA mobs"* | Phase 8 | ❌ Planned |
| *"BFS pathfinding"* | Phase 8 | ❌ Planned |
| *"3× faster than AoS"* | Phase 8 benchmark | ❌ Planned |
| *"Bloom filters (95% fewer lookups)"* | Advanced CS #1 | ❌ Planned |
| *"DP mining optimization"* | Advanced CS #4 | ❌ Planned |
| *"validated with benchmarks"* | All advanced CS phases | ❌ Planned |

---

## 📝 Notes

- **Standard library only** — no external dependencies (just like York)
- **Windows terminal** — ANSI escape codes for rendering
- **C++17** — modern C++ features throughout
- **Every advanced feature has a benchmark** — numbers to cite in interviews
- **Educational focus** — every feature teaches a real concept
