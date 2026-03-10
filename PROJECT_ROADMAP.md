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

## 📊 Overall Progress: ~80%

```
Engine Core    ████████████████████████████████████████  100%
Gameplay       ████████████████████████████████████████  100%
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
- [x] Mining in 4 directions (Arrow keys)
- [x] Building (Space to place selected block)
- [x] Facing direction tracking
- [x] Inventory counting + HUD display
- [x] Mine-up auto-climb + gravity timer reset
- [x] InputState multi-key support

**Resume claim:** ✅ *"mining/building"*

### Phase 5: Inventory & Window Stack
- [x] Block selection (1-6 number keys) + HUD indicator
- [x] E key → inventory screen overlay (cursor, Enter to select)
- [x] Window Stack System (`Window.h` → `GameWindow.h` + `InventoryWindow.h`)
- [x] Refactored 200-line game loop → 20-line stack loop

**Resume claim:** ✅ *"inventory"* + ✅ *"window stack"*

### Phase 6: Cave Generation
- [x] 2D FBM noise caves (`fbm_2d`, `smooth_noise_2d`, `hash_noise_2d`)
- [x] Depth-based ore rarity (Diamond y>20, Gold y>15)
- [x] Cave threshold carving (0.55)

**Resume claim:** ✅ *"Perlin/FBM terrain"* (surface + caves)

### Phase 7: Surface Decoration
- [x] Trees (noise-based placement, variable trunk height, 3×3 leaf canopy)
- [x] Hybrid ore visibility (ores visible only when adjacent to AIR)
- [x] New block types: WOOD, LEAF


### Phase 8: Mobs (Enemies + AI) ✅
> Resume: *"mobs"* + *"SoA mobs with BFS pathfinding (3× faster than AoS)"*

- [x] Mob types/enums (MobType, AIState) + AoS struct + SoA MobStorage
- [x] BFS pathfinding (gravity-aware, diagonal climbing, ground checks)
- [x] Mob spawning (periodic, Xorshift32 PRNG) + AI culling
- [x] AoS vs SoA benchmark: **6.55× speedup** with realistic 80-byte struct

---

### Phase 9: Pause Menu & Cheats ✅
> Resume: supports polish and usability

- [x] P key → PauseWindow (Resume / Cheats / Quit)
- [x] CheatWindow (Spectator Mode, Speed Boost, God Mode, Give Diamonds)
- [x] CheatState shared struct — clean data separation
- [x] Spectator mode: fly through blocks, no gravity/collision, WASD+WS movement
- [x] Speed boost: 2× horizontal movement per frame

---

### Phase 10: Health & Damage ✅
- [x] HP system (100 HP, color-coded HP bar in HUD)
- [x] Fall damage — `(fall_distance - 3) * 10` formula, god mode bypass
- [x] Mob contact damage — proximity-based (≤2 blocks) + knockback
- [x] Death screen → respawn at spawn point
- [x] Decoupled physics from AI (gravity-first mob architecture)

---

### Phase 11: Save/Load System ✅
> Resume: *"save/load"*

- [x] Binary serialization with `MC2D` magic header
- [x] Serialize world (all chunk positions + 32×32 block data)
- [x] Serialize player (position, HP, facing, inventory)
- [x] Serialize mobs (positions, HP, type, AI state)
- [x] Save/Load from pause menu (single slot: `saves/save.mc2d`)

---

### Phase 12: 60 FPS Upgrade
> Resume: *"60 FPS loop"*

- [ ] Upgrade `Sleep(50)` → `Sleep(16)` (~60 FPS)
- [ ] Delta-time based physics (decouple physics from frame rate)
- [ ] Gravity/jump use delta-time instead of frame counting
- [ ] Benchmark: measure actual FPS, display in debug HUD
- [ ] Optimize render loop for 60 FPS (minimize string allocations)

---

### ~~Phase 13: Story Mode~~ (SKIPPED)

---

### Phase 14: Title Screen & Polish
- [ ] ASCII art title screen
- [ ] Main menu (New Game / Load / Quit)
- [ ] Smooth transitions between screens

---

## 🧠 ADVANCED CS/DSA INTEGRATION

Each advanced feature directly supports a resume claim — with **benchmarks to prove it**.

### 1. Bloom Filter — Fast Explored Area & Spawn Deduplication
> Resume (Updated): *"Bloom filters (95% fewer redundant spatial checks)"*

- [ ] **Where:** Mob spawn zone deduplication and explored area tracking.
- [ ] **How:** Multiple hash functions → set bits in a bit array → O(1) membership test.
- [ ] **Implementation:**
  - `BloomFilter` class with configurable size and hash count.
  - When the game checks an area for spawning mobs, it queries the Bloom filter.
  - If bloom says NO → area is fresh, process spawning, then add to bloom.
  - If bloom says YES → skip (already processed recently).
- [ ] **Benchmark:**
  - Measure 10,000 spawn area checks over time.
  - **Target: 95% fewer redundant coordinate checks**, saving significant CPU cycles.
  - Print results: "Bloom filter eliminated X% of redundant spawn location checks"

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

### 4. Dynamic Programming (0/1 Knapsack & Coin Change)
> Resume (Updated): *"Dynamic Programming for optimal inventory and crafting"*

- [ ] **Where:** Inventory management (capacity optimization) and Crafting systems.
- [ ] **How:** Standard DP algorithms adapted for real gameplay mechanics.
- [ ] **Implementation 1: 0/1 Knapsack (Inventory Optimization)**
  - Players have a limited inventory capacity (weight/slots).
  - Given current items, compute the combination that maximizes total value without exceeding capacity.
  - Add a "Sort/Optimize" button in the inventory that automatically retains the most valuable haul.
- [ ] **Implementation 2: Coin Change (Crafting Minimization)**
  - Given complex crafting recipes (e.g. Iron requires Wood and Stone).
  - Use DP to compute the exact minimum raw resources needed to craft a target item.
- [ ] **Benchmark:**
  - Execute the Knapsack solver on a 100-item inventory vs a greedy approach.
  - Print results: "DP Inventory Optimization yielded X% more value than greedy algorithm."

### 5. Robin Hood Hash Map — Cache-Friendly Custom HashMap ✅
> Resume: *"Custom HashMaps"*

- [x] **Where:** Chunk lookup (`World.h`), BFS pathfinding (`Pathfinding.h`), world storage
- [x] **How:** Custom `RobinHoodMap<K, V>` replacing `std::unordered_map` — flat array storage, Robin Hood linear probing, backward-shift deletion, power-of-2 bitmask indexing
- [x] **Key advantages over `std::unordered_map`:**
  - Flat contiguous array (no linked-list node allocation)
  - Robin Hood eviction keeps probe lengths balanced
  - Backward-shift deletion (no tombstones)
  - Power-of-2 capacity with bitmask (no expensive modulo)
- [x] **Benchmark results (100K Coord entries):**
  - Insert: **1.62× faster**
  - Iteration: **11.4× faster**
  - `RobinHoodMap` used everywhere in the engine
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
│  World → ChunkMap → Chunks → Blocks                 │
├─────────────────────────────────────────────────────┤
│                  MOB ENGINE (SoA)                    │
│  SoA Storage → BFS Pathfinding → Bloom Deduplication│
│       (3× faster than AoS, benchmarked)             │
├─────────────────────────────────────────────────────┤
│                 OPTIMIZATION LAYER                   │
│  DP Knapsack/Crafting → Bloom Deduplication         │
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
| *"Perlin/FBM terrain"* | Phase 2 + 6 | ✅ Done |
| *"mining/building"* | Phase 4 | ✅ Done |
| *"mobs"* | Phase 8 | ✅ Done |
| *"inventory"* | Phase 5 | ✅ Done |
| *"save/load"* | Phase 11 | ✅ Done |
| *"Windows console"* | Phase 0-3 | ✅ Done |
| *"OOP engine (Chunk/World/ScreenBuffer)"* | Phase 1-3 | ✅ Done |
| *"window stack"* | Phase 5 | ✅ Done |
| *"60 FPS loop"* | Phase 12 | ❌ Planned |
| *"SoA mobs"* | Phase 8 | ✅ Done |
| *"BFS pathfinding"* | Phase 8 | ✅ Done |
| *"3× faster than AoS"* | Phase 8 benchmark | ✅ Done (6.55×) |
| *"Bloom filters"* | Advanced CS #1 | ❌ Planned |
| *"DP Knapsack/Crafting"* | Advanced CS #4 | ❌ Planned |
| *"validated with benchmarks"* | All advanced CS phases | ❌ Planned |

---

## 📝 Notes

- **Standard library only** — no external dependencies (just like York)
- **Windows terminal** — ANSI escape codes for rendering
- **C++17** — modern C++ features throughout
- **Every advanced feature has a benchmark** — numbers to cite in interviews
- **Educational focus** — every feature teaches a real concept
