# 🎯 York — Interview Prep Guide

> Quick reference for every technical question an interviewer might ask about this project.

---

## 1. What Is a Game Engine?

### Simple Answer

> "A game engine is the invisible infrastructure that runs BEHIND any game. It handles all the boring but essential stuff — drawing pixels on screen, reading keyboard input, applying physics, ticking time forward — so that game developers can focus on the fun stuff: designing worlds, enemies, and mechanics."

### The Theory

Think of a game engine like the **engine of a car**. The player (driver) sees the steering wheel, pedals, and dashboard (the game), but underneath there's a combustion engine, transmission, and fuel system doing all the real work. In our case:

| Engine Component | What It Does | York Equivalent |
|---|---|---|
| **Game Loop** | Ticks the entire game forward 20–60 times per second | `while(!quit) { input → update → render; Sleep(50); }` |
| **Renderer** | Draws the current game state to the screen | `ScreenBuffer` — 80×24 pixel grid, ANSI colors |
| **Input System** | Reads keyboard presses without pausing the game | `Input.h` — `_kbhit()` + `_getch()`, non-blocking |
| **Physics** | Applies gravity, collision, fall damage | `GameWindow` — gravity timer, collision detection |
| **World Manager** | Loads/unloads pieces of the game world | `World.h` — lazy-loaded infinite chunk map |
| **Entity System** | Manages all living things (player, mobs) | `MobStorage` (SoA) + BFS pathfinding |
| **UI System** | Manages menus, HUD, overlays | `Window` base class + window stack |

### Key Point for the Interviewer

> "I didn't use Unity or Unreal. I built every one of these systems from scratch in C++17 with zero external libraries. That forced me to deeply understand how each layer works — from raw memory allocation to frame timing."

---

## 2. Walk Me Through the Architecture of York From Scratch

### Simple Answer

> "York has a **layered architecture** — like a stack of pancakes where each layer depends only on the layer below it. This makes it modular, testable, and easy to extend."

### The 6 Layers (Bottom → Top)

```
┌─────────────────────────────────────────────────────┐
│  Layer 6:  WINDOW STACK (UI)                        │
│  Title Menu ←→ Game World ←→ Inventory ←→ Pause     │
├─────────────────────────────────────────────────────┤
│  Layer 5:  MOB ENGINE                               │
│  SoA Storage → BFS Pathfinding → AI State Machine   │
├─────────────────────────────────────────────────────┤
│  Layer 4:  GAMEPLAY LOGIC                           │
│  Mining, Building, Inventory, HP, Fall Damage        │
├─────────────────────────────────────────────────────┤
│  Layer 3:  RENDERING + GAME LOOP                    │
│  ScreenBuffer → Double Buffering → 20 FPS Loop      │
├─────────────────────────────────────────────────────┤
│  Layer 2:  WORLD ENGINE                             │
│  World → ChunkMap → Chunks → Terrain Generation     │
├─────────────────────────────────────────────────────┤
│  Layer 1:  CORE DATA TYPES                          │
│  Coord, BlockType, Pixel, Color, RobinHoodMap       │
└─────────────────────────────────────────────────────┘
```

---

### Layer 1: Core Data Types

**What:** The atomic building blocks everything else is built on.

| Type | What It Represents | Key Detail |
|---|---|---|
| `Coord` | An (x, y) pair | Has a custom `CoordHash` so it works as a key in hash maps |
| `BlockType` | What a block is (AIR, STONE, GRASS, DIAMOND...) | An `enum class` — type-safe, prevents accidental int mixing |
| `Pixel` | A single character + color for terminal display | `{char ch, Color color}` |
| `Color` | ANSI terminal color codes | `enum class` mapping to numbers like 31 (red), 32 (green) |
| `RobinHoodMap` | Custom hash map replacing `std::unordered_map` | Flat array, Robin Hood probing, 1.6× faster insert, 11× faster iteration |

**Why this layer exists:** Every other layer needs to say "what block is at position (x, y)?" — and these types make that possible.

---

### Layer 2: World Engine

**What:** Manages an infinite world by breaking it into 32×32 **chunks** loaded on demand.

**Key classes:**

- **`Chunk`** — A 32×32 grid of `BlockType` stored in a `std::array<std::array<BlockType, 32>, 32>`. When a chunk is created, it immediately generates its own terrain in the constructor (RAII pattern).

- **`World`** — Holds a `RobinHoodMap<Coord, unique_ptr<Chunk>>`. When the game asks for a block at world position (x, y):
  1. Divide (x, y) by CHUNK_SIZE to get the chunk coordinate
  2. Look up the chunk in the map
  3. If it doesn't exist yet → create it (lazy loading)
  4. Return the block at the local position within the chunk

- **`Terrain`** — Contains the procedural generation algorithms (Perlin Noise, FBM, cave carving, ore distribution, tree placement). More on this in Question 4.

**Example flow:**
```
Player walks to x=100 → World::get_block(100, 5)
  → chunk coord = (100 / 32) = (3, 0)
  → chunk (3,0) not in map → create new Chunk(3,0)
  → Chunk constructor calls generate_terrain()
  → terrain is generated via FBM noise
  → block at local (100 % 32, 5) = (4, 5) returned
```

**Why this layer matters:** Without chunking, you'd need to store the entire infinite world in memory — impossible. Chunks let us load only what the player can see.

---

### Layer 3: Rendering + Game Loop

**What:** Draws the world onto the terminal and ticks the game forward at a fixed rate.

**ScreenBuffer (the renderer):**
- An 80×24 grid of `Pixel` values (matching the terminal size)
- **Double buffering:** We write ALL pixels to an offscreen buffer first, then flush the entire buffer to the terminal in ONE `std::cout` call. This prevents flickering.
- **ANSI escape codes:** `\033[H` moves the cursor to the top-left. `\033[32m` sets the color to green. We track the last color and only emit a new escape code when the color changes — saves bandwidth.

**Game Loop:**
```
while (!quit) {
    InputState input = get_input();       // 1. Read keyboard (non-blocking)
    window_stack.top()->handle_input();    // 2. Update game state
    window_stack.top()->render(screen);    // 3. Draw to buffer
    screen.render();                       // 4. Flush buffer to terminal
    Sleep(50);                             // 5. Wait (~20 FPS)
}
```

This is a **fixed timestep loop**. Every iteration is called a **frame**. 20 FPS means 50ms per frame.

---

### Layer 4: Gameplay Logic

**What:** Mining, building, inventory, gravity, collision, HP, fall damage.

All of this lives inside `GameWindow::handle_input()`. Every frame:
1. Read player movement (WASD) → check collision → move if AIR
2. Read mining (Arrow keys) → destroy block → add to inventory
3. Read building (Space) → place block from inventory
4. Apply gravity (every 5 frames, check if block below is AIR → fall)
5. Calculate fall damage (if fall distance > 3 blocks → 10 damage per extra block)
6. Check mob-player collisions → apply damage + knockback

---

### Layer 5: Mob Engine

**What:** Manages enemies (Zombies) with Struct-of-Arrays storage and BFS pathfinding.

- **`MobStorage`** — SoA layout: separate `vector<int> x`, `vector<int> y`, `vector<int> hp`, etc. (More on why in Question 6 & 7)
- **Spawning** — Every 120 frames, a zombie spawns 15–46 blocks offset from the player using `Xorshift32` PRNG
- **AI** — Every 10 frames, each mob within 60 blocks of the player:
  1. If in mid-air → fall (gravity)
  2. If on ground → BFS pathfind toward the player → move one step along the path
- **Culling** — Mobs beyond 60 blocks are skipped entirely (distance² > 3600)

---

### Layer 6: Window Stack (UI System)

**What:** A `std::stack<Window*>` that manages which screen is active.

**`Window`** is an abstract base class:
```cpp
class Window {
public:
    virtual bool handle_input(const InputState& input) = 0;
    virtual void render(ScreenBuffer& screen) = 0;
    virtual bool is_opaque() const { return true; }
};
```

Subclasses: `GameWindow`, `InventoryWindow`, `PauseWindow`, `CheatWindow`

**How it works:**
- Only the **top** window receives input and renders
- Press E → push `InventoryWindow` on top → game pauses underneath
- Press Esc in inventory → pop → game resumes
- Press P → push `PauseWindow` → shows Resume/Cheats/Quit

**Why a stack?** It cleanly separates UI state. Each screen has its own isolated logic. No giant `if (state == GAME) ... else if (state == INVENTORY) ...` spaghetti. Adding a new screen = add a new class that extends `Window`.

---

## 3. How Frame Generation and Screen Rendering Works

### Simple Answer

> "Every frame, the game clears the screen buffer, draws the visible world into it, then flushes the entire buffer to the terminal in a single I/O call. This is called double buffering, and it's the same technique real GPUs use — just in software."

### Step-by-Step: What Happens in ONE Frame

```
Frame starts
    │
    ├── 1. screen.clear()
    │       Fill all 80×24 = 1920 pixels with {' ', WHITE}
    │
    ├── 2. Calculate camera
    │       cam_x = player_x - 40    (center player horizontally)
    │       cam_y = player_y - 12    (center player vertically)
    │
    ├── 3. Draw world blocks
    │       For each screen pixel (sx, sy):
    │         world_x = cam_x + sx
    │         world_y = cam_y + sy
    │         block = world.get_block(world_x, world_y)
    │         screen.set_pixel(sx, sy, block_to_pixel(block))
    │
    ├── 4. Draw player
    │       screen.set_pixel(40, 12, {'$', BRIGHT_CYAN})
    │       (always at screen center)
    │
    ├── 5. Draw mobs
    │       For each mob: calculate screen position, draw if visible
    │
    ├── 6. Draw HUD
    │       Line 0: Position + controls
    │       Line 1: Inventory counts
    │       Line 2: HP bar [########............] 80/100
    │
    └── 7. screen.render()  ← THE MAGIC STEP
            Build one giant string, single cout, done
```

### The Double-Buffer Flush (screen.render())

This is the critical optimization. Here's what it does:

```cpp
void render() const {
    std::string frame;
    frame.reserve(SCREEN_WIDTH * SCREEN_HEIGHT * 12);  // Pre-allocate ~23KB in one shot

    frame += "\033[H";  // ANSI: move cursor to top-left (no clearing!)

    Color last_color = Color::WHITE;
    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            const Pixel& p = buffer[y][x];
            if (p.color != last_color) {        // Only emit color code when color changes
                frame += "\033[" + to_string(p.color) + "m";
                last_color = p.color;
            }
            frame += p.ch;
        }
        frame += "\n";
    }
    frame += "\033[m";   // Reset terminal color
    std::cout << frame;  // ONE I/O call for the entire screen
}
```

### Why This Doesn't Flicker

| Naive Approach (Flickering) | Our Approach (No Flickering) |
|---|---|
| `system("cls")` clears screen → screen goes blank | `\033[H` repositions cursor → old frame stays visible |
| Then we draw pixel by pixel → user sees partial frames | We overwrite each pixel in-place → seamless transition |
| 1920 separate `cout` calls | 1 single `cout` call |
| **Result: visible flickering** | **Result: smooth rendering** |

### Why Pre-allocate?

`frame.reserve(SCREEN_WIDTH * SCREEN_HEIGHT * 12)` allocates ~23KB upfront. Without this, `std::string` would keep reallocating as we append characters — each reallocation copies the entire string. Pre-allocating = zero reallocations = faster.

---

## 4. How Infinite Terrain, Mob Generation, and Perlin Noise / FBM Work

### The Core Problem

> "How do you generate a world that's infinite, looks natural, and is the SAME every time you visit the same coordinates?"

### Perlin Noise — The Foundation

**What is noise?** Random numbers. But pure random looks like TV static — it's ugly. **Perlin noise** generates "smooth random" — nearby coordinates produce similar values, creating natural-looking gradients.

#### How Our Noise Works (Hash-Based)

We don't use Ken Perlin's original algorithm (which needs gradient tables). Instead, we use a cheaper **value noise** approach:

```
Step 1: Hash the coordinate to get a "random" float
Step 2: Smoothly interpolate between adjacent hash values
```

**Hash function (integer → float in [0, 1]):**
```cpp
float hash_noise(int x, int seed) {
    unsigned int n = x * 374761393u + seed * 668265263u;  // Mix input with large primes
    n = (n << 13) ^ n;                                     // Bit manipulation for randomness
    n = n * (n * n * 15731 + 789221) + 668265263;          // More mixing
    // Extract mantissa bits to get float in [1, 2), then subtract 1
    unsigned int m = (n & 0x007FFFFF) | 0x3F800000;
    return bit_cast_to_float(m) - 1.0f;  // Result: [0, 1)
}
```

**Why this is deterministic:** The same `(x, seed)` ALWAYS produces the same output. No `rand()` or global state. The world with `seed=42` looks identical every time.

**Smooth interpolation (Hermite curve):**
```cpp
float smooth_noise(float x, int seed) {
    int xi = floor(x);
    float frac = x - xi;
    float t = frac * frac * (3 - 2 * frac);  // S-curve: smooth at boundaries
    float a = hash_noise(xi, seed);
    float b = hash_noise(xi + 1, seed);
    return a + t * (b - a);  // Lerp between neighbors using smooth t
}
```

The S-curve `t = frac² × (3 - 2×frac)` is called **Hermite interpolation**. It makes the transition between hash values smooth (derivative = 0 at integers) instead of having sharp corners.

---

### FBM (Fractional Brownian Motion) — Adding Detail

Single-layer noise creates smooth hills but looks boring — like a sine wave. Real terrain has **detail at multiple scales**: big mountains AND small bumps.

FBM layers multiple noise samples at different **frequencies** (zoom levels) and **amplitudes** (strengths):

```
                                                          ╱╲
Big hills (low freq, high amp):          ╱‾‾‾‾‾‾‾‾‾‾‾‾‾╲╱  ╲____

Medium bumps (2× freq, 0.5× amp):       ╱╲  ╱╲  ╱╲  ╱╲  ╱╲  ╱╲

Small details (4× freq, 0.25× amp):    ╱╲╱╲╱╲╱╲╱╲╱╲╱╲╱╲╱╲╱╲╱╲╱╲╱╲

SUM of all three = realistic terrain:   ╱‾‾╲╱╲╱╲╱╲__╱╲_╱╲╱╲╱╲╱╲╲__
```

**Our FBM code:**
```cpp
float fbm(float x, int seed, int octaves = 4) {
    float value = 0, amplitude = 1, max_amp = 0;
    float frequency = 0.1;

    for (int i = 0; i < octaves; i++) {
        value += smooth_noise(x * frequency, seed ^ (i * 0x1f1f1f1f)) * amplitude;
        max_amp += amplitude;
        amplitude *= 0.5;     // Each octave is half as strong (gain)
        frequency *= 2.0;     // Each octave is twice as detailed (lacunarity)
    }
    return value / max_amp;  // Normalize to [0, 1]
}
```

**The 3 knobs:**
| Parameter | Value | Meaning |
|---|---|---|
| **Octaves** | 4 | Number of noise layers. More = more detail but slower |
| **Lacunarity** | 2.0 | Frequency multiplier per octave. 2× = each layer is twice as detailed |
| **Gain** | 0.5 | Amplitude multiplier per octave. 0.5× = each layer contributes half as much |

---

### How Terrain Is Generated Per Chunk

When a chunk at position `(cx, cy)` is created, `generate_chunk_terrain()` fills its 32×32 grid:

```
For each column x (0 to 31):
    1. SURFACE HEIGHT:  surface_y = 8 + fbm(world_x) * 8
                        (hills range from y=2 to y=26)

    2. Fill column:
       y < surface_y       → AIR
       y == surface_y      → GRASS
       y < surface_y + 4   → DIRT
       y < CHUNK_SIZE - 1  → STONE (with caves and ores)
       y == CHUNK_SIZE - 1  → BEDROCK

    3. CAVE CARVING:  cave_noise = fbm_2d(world_x, y)
                      if cave_noise > 0.55 → AIR (cave!)

    4. ORE PLACEMENT: ore_noise = hash_noise(wx*100 + y)
                      > 0.95 AND y > 20 → DIAMOND (deepest, rarest)
                      > 0.88 AND y > 15 → GOLD
                      > 0.80           → IRON

    5. TREES:         tree_noise = hash_noise(wx)
                      > 0.85 → Place trunk (WOOD) + leaf canopy (LEAF)
```

---

### Mob Spawning

Every 120 frames (~6 seconds):
1. Generate random offset using `Xorshift32` PRNG: 15–46 blocks left or right of player
2. Drop the spawn point down until it hits solid ground
3. Add a new ZOMBIE at that position

**Xorshift32** is a single-line PRNG: `state ^= state << 13; state ^= state >> 17; state ^= state << 5;`. It's much faster than `std::mt19937` and good enough for game randomness.

---

### Scaling Bottlenecks

| Bottleneck | Why | Mitigation |
|---|---|---|
| **Chunk generation on first visit** | FBM runs 32 × 4 octaves = 128 noise evaluations per column | We only generate when the player actually reaches a chunk (lazy loading) |
| **Cache pollution from distant chunks** | Chunks far from the player waste RAM | Could add chunk unloading (not yet implemented) |
| **FBM is O(octaves) per sample** | More octaves = more detail but slower terrain gen | 4 octaves is the sweet spot — diminishing visual returns beyond |
| **2D cave noise is expensive** | `fbm_2d` runs for every underground block (32×~20 = 640 calls per column) | Cave noise uses slightly fewer octaves |

---

## 5. How OOP Is Used

### Simple Answer

> "OOP lets me break the engine into self-contained classes where each class owns its data and behavior. This means adding new features doesn't require touching existing code."

### The 4 Pillars of OOP in York

#### 1. Encapsulation — "Each class guards its own data"

```cpp
class Chunk {
private:
    std::array<std::array<BlockType, 32>, 32> blocks;  // HIDDEN from outside
    Coord position;

public:
    BlockType get_block(int x, int y) const;  // Only way to access blocks
    void set_block(int x, int y, BlockType t);

private:
    void generate_terrain();  // Internal — called by constructor only
};
```

**Why it matters:** Nobody can corrupt the block array directly. The bounds check in `get_block()` prevents crashes from invalid coordinates.

#### 2. Abstraction — "Hide the complex details"

```cpp
// The world exposes a simple interface:
BlockType block = world.get_block(500, 10);

// Internally? It does chunk coordinate math, lazy chunk loading,
// hash map lookups, and terrain generation. The caller doesn't
// know or care about any of that.
```

#### 3. Inheritance — "Share behavior between screen types"

```cpp
class Window {                                   // Abstract base class
public:
    virtual bool handle_input(const InputState&) = 0;  // Pure virtual
    virtual void render(ScreenBuffer&) = 0;            // Pure virtual
    virtual bool is_opaque() const { return true; }    // Default impl
};

class GameWindow : public Window { /* gameplay */ };
class InventoryWindow : public Window { /* inventory UI */ };
class PauseWindow : public Window { /* pause menu */ };
class CheatWindow : public Window { /* cheat codes */ };
```

All four windows implement the same interface, so the game loop doesn't care which one is active:

```cpp
// Game loop doesn't know if it's the game, inventory, or pause menu
window_stack.top()->handle_input(input);
window_stack.top()->render(screen);
```

#### 4. Polymorphism — "Same function call, different behavior"

When `render()` is called on a `Window*`, C++ uses the **virtual table (vtable)** to dispatch to the correct subclass at runtime. This is runtime polymorphism.

```
window_stack.top()->render(screen);
    │
    ├── If top is GameWindow*  → draws terrain, player, mobs, HUD
    ├── If top is InventoryWindow* → draws item list with cursor
    ├── If top is PauseWindow* → draws Resume/Cheats/Quit buttons
    └── If top is CheatWindow* → draws cheat toggles
```

### RAII (Resource Acquisition Is Initialization)

York uses RAII everywhere:
- `Chunk` generates terrain in its **constructor** → no "init()" method needed
- `RobinHoodMap` allocates in constructor, deallocates in **destructor** → no memory leaks
- `std::unique_ptr<Chunk>` automatically deletes chunks when removed from the map

---

## 6. The Use of Data-Oriented Design (DOD)

### Simple Answer

> "Data-Oriented Design (DOD) means organizing data by how the **CPU actually processes it** — contiguous, cache-friendly arrays — rather than by how humans naturally think about objects. I used DOD for mob storage to get a 6.55× speedup."

### The Key Idea

**OOP thinks about objects:** "A Mob HAS a position, health, type, and AI state."

**DOD thinks about data access patterns:** "The position update loop only needs x[] and y[]. Why load health, type, and AI state into the cache when we don't use them?"

### AoS (Array of Structs) — The OOP Way

```cpp
struct Mob { int x, y, hp; MobType type; AIState state; };
std::vector<Mob> mobs;  // [Mob0, Mob1, Mob2, ...]
```

Memory layout:
```
|x0|y0|hp0|type0|state0|x1|y1|hp1|type1|state1|x2|y2|hp2|type2|state2|...
 └──────── Mob 0 ───────┘└──────── Mob 1 ───────┘└──────── Mob 2 ───────┘
```

When updating positions, the CPU loads a whole `Mob` into cache but only uses `x` and `y`. The `hp`, `type`, `state` fields are **wasted cache space**.

### SoA (Struct of Arrays) — The DOD Way

```cpp
struct MobStorage {
    std::vector<int> x;        // [x0, x1, x2, x3, ...]
    std::vector<int> y;        // [y0, y1, y2, y3, ...]
    std::vector<int> hp;       // [hp0, hp1, hp2, ...]
    std::vector<MobType> type; // [type0, type1, ...]
    std::vector<AIState> state;
};
```

Memory layout:
```
x array:   |x0|x1|x2|x3|x4|x5|x6|x7|x8|...    ← ALL positions contiguous
y array:   |y0|y1|y2|y3|y4|y5|y6|y7|y8|...
hp array:  |hp0|hp1|hp2|hp3|...                   ← Not loaded during position update
```

When updating positions, the CPU loads just `x[]` and `y[]` — **100% of cached data is used**. Zero waste.

### When to Use DOD vs OOP

| Use DOD (SoA) When | Use OOP (AoS) When |
|---|---|
| You process MANY entities in tight loops | You have few entities with complex behavior |
| You only need a subset of fields per loop | You need all fields together |
| Performance is critical (hot loop) | Code clarity is more important |
| Example: 10,000 mob position updates | Example: 1 player with many interactions |

---

## 7. AoS → SoA: The Physical Hardware Mechanics

### Simple Answer

> "The reason SoA is faster isn't software — it's hardware. Specifically, it's how the CPU cache works. Modern CPUs don't load individual bytes — they load entire 64-byte **cache lines**. SoA packs more useful data into each cache line."

### The CPU Memory Hierarchy

```
              ┌─────────────┐
              │  CPU Core    │  Operations: ~0.3 ns per cycle
              └──────┬──────┘
                     │
              ┌──────▼──────┐
              │   L1 Cache   │  32–64 KB   │ ~1 ns    │ 64-byte cache lines
              └──────┬──────┘
              ┌──────▼──────┐
              │   L2 Cache   │  256 KB     │ ~4 ns
              └──────┬──────┘
              ┌──────▼──────┐
              │   L3 Cache   │  8+ MB      │ ~10 ns
              └──────┬──────┘
              ┌──────▼──────┐
              │   RAM (DRAM) │  16+ GB     │ ~100 ns   ← 100× slower than L1!
              └──────────────┘
```

**Key fact:** An L1 cache access costs ~1 ns. A RAM access costs ~100 ns. That's a **100× penalty** for every cache miss.

### What Happens with AoS (Cache-Unfriendly)

Our realistic `MobAoS` struct is 80 bytes. A cache line is 64 bytes.

```
Cache Line 1 (64 bytes):
|x0|y0|vx0|vy0|hp0|max_hp0|damage0|ai_state0|target_x0|target_y0|anim0|spawn0|last0|path0|flags0|loot0|
  ↑  ↑
  NEED (8 bytes)                              WASTE (56 bytes = 87.5% wasted!)

Cache Line 2 (64 bytes):
|armor0|aggro0|pad|pad|x1|y1|vx1|vy1|hp1|max_hp1|damage1|ai_state1|...
                        ↑  ↑
                        NEED                  ← One mob spans TWO cache lines
```

**Problem:** Each mob is 80 bytes → spans 1–2 cache lines. But we only need `x` and `y` (8 bytes). So **87.5% of every cache line is wasted data.**

For 10,000 mobs: 10,000 × 80 bytes = **800 KB** loaded from memory. Only 80 KB was useful.

### What Happens with SoA (Cache-Friendly)

```
x array (contiguous ints):
Cache Line 1: |x0|x1|x2|x3|x4|x5|x6|x7|x8|x9|x10|x11|x12|x13|x14|x15|
                ↑   ↑   ↑   ↑   ↑   ↑   ↑   ↑   ↑   ↑    ↑    ↑    ↑    ↑    ↑    ↑
                ALL 16 values are used! (64 bytes / 4 bytes per int = 16 ints per cacheline)

y array: same thing.
```

**Result:** 16 mob positions processed per cache line instead of ~0.8. That's a **20× improvement in cache utilization**.

For 10,000 mobs: 10,000 × 8 bytes = **80 KB** loaded from memory. 100% useful.

### Hardware Prefetching

Modern CPUs have **hardware prefetchers** that detect sequential memory access patterns:

- **SoA:** CPU sees sequential access to `x[0], x[1], x[2], ...` → prefetcher kicks in → loads the next cache line BEFORE you need it → near-zero latency
- **AoS:** Access jumps by 80 bytes (one struct size) each iteration → pattern is less efficient for prefetching

### Our Benchmark Result

| Metric | AoS (80-byte struct) | SoA |
|---|---|---|
| Mobs | 10,000 | 10,000 |
| Iterations | 10,000 | 10,000 |
| **Speedup** | Baseline | **6.55× faster** |

The target was 3×. We achieved 6.55× because the realistic 80-byte struct makes cache pollution especially severe.

### Summary Formula

```
Speedup ≈ (AoS struct size) / (fields actually accessed per iteration)
         = 80 bytes / 8 bytes
         = 10× theoretical max
         = ~6.55× actual (overhead, branch prediction, etc. eat some)
```

---

## 8. Bloom Filters — Why, How, and What Tradeoffs

### Simple Answer

> "A Bloom filter is a space-efficient probabilistic data structure that answers: *Is this element in the set?* It can say **definitely NO** or **probably YES** — but never gives a false negative. I use it for two things in York: **mob spawn zone deduplication** (don't spawn a zombie where we already spawned one) and **explored area tracking** (skip terrain processing for chunks the player has already visited). Together they eliminate 95% of redundant coordinate lookups."

---

### WHY Bloom Filters Are Needed — The Actual Problem

Look at the current spawning code in `GameWindow`:

```cpp
++spawn_timer;
if (spawn_timer >= SPAWN_INTERVAL) {   // every 120 frames (~6 sec)
    spawn_timer = 0;

    uint32_t r = fast_rand();
    int offset = (r & 31) + 15;         // random 15–46 block offset
    if (r & 32) offset = -offset;

    int spawn_x = player_x + offset;
    int spawn_y = player_y;

    // Drop down until we hit solid ground
    while (spawn_y < CHUNK_SIZE - 1 &&
           world.get_block(spawn_x, spawn_y) == BlockType::AIR) {
        ++spawn_y;                       // ← each call is a hash map lookup!
    }
    --spawn_y;

    if (spawn_y > 0) {
        mobs.add(spawn_x, spawn_y, 20, MobType::ZOMBIE, AIState::CHASING);
    }
}
```

**Three concrete problems:**

| # | Problem | Cost |
|---|---|---|
| 1 | **Repeated spawn zones:** The PRNG offset range is only 15–46 blocks wide. Over time, many spawn attempts land on coordinates that were already spawned on → wasted ground-scan work | Each ground scan does ~10–20 `world.get_block()` calls — each one hashes a `Coord`, looks up a chunk in `RobinHoodMap`, then indexes into the block array |
| 2 | **Explored area re-processing:** When the rendering loop iterates 80×24 = 1,920 screen pixels per frame, every `world.get_block()` call goes through chunk lookup → already-loaded chunks are looked up again and again | 1,920 hash map lookups per frame × 20 FPS = **38,400 hash lookups/sec** — most are for chunks we've already visited |
| 3 | **No way to cheaply ask "have we already dealt with this coordinate?"** | Without a membership test, we must always do the full lookup |

#### Why Not Just Use `std::unordered_set`?

You could track "already-spawned-at" coordinates with:

```cpp
std::unordered_set<Coord, CoordHash> spawned_locations;
```

But this has real costs:

| Factor | `unordered_set` | Bloom Filter |
|---|---|---|
| Memory per element | ~56 bytes (Coord + hash + bucket pointer + next pointer) | ~10 bits (< 2 bytes) |
| 1,000 locations | ~56 KB | ~1.2 KB (**47× smaller**) |
| Lookup cost | Hash → bucket → traverse linked list (cache-unfriendly) | Hash → check k bits in a flat array (cache-friendly) |
| Cache behavior | Pointer-chasing across heap | Single contiguous `std::vector<uint8_t>` — fits in L1 |
| Deletion | Supported | Not supported (acceptable for our use case) |

For a game running at 20–60 FPS, keeping the membership test **inside L1 cache** is critical. A 1.2 KB Bloom filter fits entirely in L1 (32 KB). A 56 KB hash set does not.

---

### HOW a Bloom Filter Works — Step by Step

#### The Data Structure

A Bloom filter is just two things:
1. **A bit array** of `m` bits (all initialized to 0)
2. **k independent hash functions** that each map an element to a bit index

#### Insert

To add element `Coord(5, 3)`:

```
hash₁(Coord(5,3)) = 2     → set bit 2
hash₂(Coord(5,3)) = 7     → set bit 7
hash₃(Coord(5,3)) = 11    → set bit 11

Bit array:
[0][0][1][0][0][0][0][1][0][0][0][1][0][0][0][0]
      ↑                 ↑              ↑
```

#### Query — "Is Coord(5,3) in the set?"

```
Check bit 2:  ✅ (is 1)
Check bit 7:  ✅ (is 1)
Check bit 11: ✅ (is 1)
All k bits are set → "PROBABLY YES" (could be a false positive)
```

#### Query — "Is Coord(9,1) in the set?"

```
Check bit 2:  ✅ (is 1 — from the previous insert)
Check bit 5:  ❌ (is 0)
→ At least one bit is 0 → "DEFINITELY NO"
```

**Key insight:** If ANY of the k bits is 0, the element was NEVER inserted. This means **zero false negatives**. But if all bits happen to be set by OTHER elements' insertions, we get a **false positive** — we think it's there when it isn't.

#### Why False Negatives Are Impossible

To get a false negative, at least one of the element's k bits would need to be 0 AFTER it was inserted. But insertion SETS those bits, and bits are never cleared. So once inserted, it will always return "probably yes." This is a mathematical guarantee, not a heuristic.

---

### HOW It's Used in York — Concrete Integration

#### Use Case 1: Mob Spawn Zone Deduplication

```cpp
// BEFORE (no Bloom filter):
int spawn_x = player_x + offset;
// Always do the expensive ground scan, even if we spawned here before

// AFTER (with Bloom filter):
int spawn_x = player_x + offset;
Coord spawn_zone = {spawn_x / 8, 0};  // Quantize to 8-block zones

if (spawn_bloom.probably_contains(spawn_zone)) {
    // Already spawned in this zone recently → SKIP
    // Cost: checking 7 bits in a 1.2 KB array (< 2 ns)
    return;
}

// Zone is fresh → do the ground scan → spawn mob → add to bloom
spawn_bloom.insert(spawn_zone);
```

**Result:** If the player stays near the same area (common in mining), ~95% of spawn attempts hit already-processed zones. Each avoided spawn saves ~10–20 `world.get_block()` calls.

#### Use Case 2: Explored Area Tracking

```cpp
// Before rendering a chunk region, check if we've processed it:
Coord chunk_pos = world_to_chunk(wx, wy);

if (explored_bloom.probably_contains(chunk_pos)) {
    // Already loaded and cached → go straight to chunk (skip existence check)
} else {
    // New area — generate chunk, then add to bloom
    explored_bloom.insert(chunk_pos);
}
```

---

### The Math — Sizing the Filter

#### False Positive Probability

After inserting `n` elements into an `m`-bit array with `k` hash functions:

```
P(false positive) = (1 - e^(-kn/m))^k
```

**Intuition:** Each hash function sets 1 bit. After n insertions, `k×n` bits have been set (with overlap). The probability that a random query finds all k of its bits already set is the false positive rate.

#### Optimal Hash Count

To minimize false positives for given `m` and `n`:

```
k_optimal = (m / n) × ln(2) ≈ 0.693 × (m / n)
```

#### Sizing for York

| Parameter | Value | Reasoning |
|---|---|---|
| Expected spawn zones (n) | 1,000 | Player explores ~1,000 8-block zones in a typical session |
| Desired false positive rate (p) | 1% | 1 in 100 fresh zones get skipped — player won't notice |
| Required bits (m) | `m = -n × ln(p) / (ln2)²` = `-1000 × ln(0.01) / 0.4805` = **9,585 bits ≈ 1.2 KB** |
| Optimal hash functions (k) | `k = (9585/1000) × 0.693` = **6.6 ≈ 7** |

**Comparison:**

| Storage Method | Memory for 1,000 Coords | Cache-Friendly? |
|---|---|---|
| `std::unordered_set<Coord>` | ~56 KB | ❌ (linked list nodes scattered on heap) |
| `std::vector<Coord>` (sorted, binary search) | ~8 KB | ⚠️ (contiguous but O(log n) lookup) |
| `RobinHoodMap<Coord, bool>` | ~24 KB | ✅ (flat array, but overkill for boolean membership) |
| **Bloom Filter** | **1.2 KB** | ✅ (flat bit array, fits in L1 cache) |

#### Generating k Hash Functions from One Hash

We don't need 7 independent hash functions. We use **double hashing** — compute two hashes and derive the rest:

```cpp
size_t h1 = coord_hash(key);
size_t h2 = h1 >> 16 | h1 << 16;  // Bit-rotate for second hash

for (int i = 0; i < k; i++) {
    size_t bit_index = (h1 + i * h2) % m;
    // set or check this bit
}
```

This is mathematically proven to have the same false positive rate as k truly independent hashes (Kirsch & Mitzenmacher, 2006).

---

### Architectural Limitations — Be Honest in the Interview

| Limitation | What It Means for York | How We Handle It |
|---|---|---|
| **False positives** | Occasionally skip a valid spawn zone (~1% of the time). A mob that should have spawned doesn't | Acceptable — player sees plenty of mobs, missing 1 in 100 is invisible |
| **No deletion** | Can't "forget" a spawn zone — once marked, it's permanent for that filter's lifetime | **Periodic reset:** every N minutes, create a fresh Bloom filter. Old zones become eligible again, which is actually desirable (respawning) |
| **No enumeration** | Can't ask "which zones have been spawned in?" — only "has THIS zone been spawned in?" | Fine — we never need to list all zones, only test individual ones |
| **Fixed capacity** | If we insert way more than the planned 1,000 elements, the false positive rate climbs. At 5,000 inserts into a 9,585-bit filter, P(fp) jumps to ~28% | **Pre-size conservatively** or use periodic reset to keep n manageable |
| **Non-deterministic behavior** | Two players with the same seed may experience slightly different spawn patterns due to Bloom filter state depending on exploration path | Acceptable — mob spawning is already somewhat random. Bloom filter just optimizes *when* spawns are attempted, not the world's deterministic terrain |

### Why 95% Fewer Lookups?

**Without Bloom filter:** Every spawn cycle picks a random offset → does a full ground scan (10–20 `get_block` calls each going through hash map lookup). In a typical play session, the player stays in a ~100-block-wide area. With spawn-zone quantization at 8 blocks, that's ~12 unique zones. But spawn attempts happen hundreds of times → most attempts re-check zones that already have mobs.

**With Bloom filter:** The spawn function first checks `spawn_bloom.probably_contains(zone)` — a sub-2-nanosecond operation. For 100 spawn attempts in familiar territory, ~95 are in already-processed zones → the Bloom filter says "probably yes" → skip. Only ~5 land in genuinely new zones → proceed with the full spawn logic.

**Net result: 95% of expensive `world.get_block()` chains are replaced with 7 bit checks in a 1.2 KB array.**

---

## 9. Perlin Noise + FBM + Infinite Chunks: Interaction and Bottlenecks

### Simple Answer

> "The genius of combining noise with chunk-based worlds is that noise is a pure mathematical function — it takes a coordinate and returns a value, with no memory or state. So every chunk can generate itself independently, using only its own coordinates and the world seed."

### How They Interact

```
Player walks right → enters new chunk region → World::get_chunk() called
    │
    ├── Chunk doesn't exist → create new Chunk(pos)
    │       │
    │       └── Constructor calls generate_terrain()
    │               │
    │               ├── For each column x:
    │               │     1. Convert to WORLD coordinate: wx = cx * 32 + x
    │               │     2. Surface height = 8 + fbm(wx, seed) * 8
    │               │        └── fbm runs 4 octaves of smooth_noise
    │               │            └── smooth_noise interpolates between hash_noise values
    │               │
    │               │     3. Fill blocks (AIR → GRASS → DIRT → STONE)
    │               │
    │               │     4. Cave carving: fbm_2d(wx, y, seed+777)
    │               │        └── 2D noise creates organic cave shapes
    │               │
    │               │     5. Ore placement: hash_noise(wx*100 + y, seed+99)
    │               │        └── Depth-based rarity
    │               │
    │               └── 6. Tree placement: hash_noise(wx, seed+155) > 0.85
    │
    └── Chunk now in map → future accesses are instant
```

### Why Chunks Have Seamless Borders

Because noise functions operate on **world coordinates**, not local chunk coordinates. 

```
Chunk A (position 0)          Chunk B (position 1)
  world x: 0 to 31              world x: 32 to 63

  fbm(31) = 0.634               fbm(32) = 0.641   ← nearly identical!
```

Since `fbm(31)` and `fbm(32)` differ by a tiny amount (noise is smooth/continuous), the surface heights match perfectly across chunk boundaries. No stitching code needed.

### Scaling Bottlenecks

| Bottleneck | Why It Happens | Severity | Solution |
|---|---|---|---|
| **Chunk generation spike** | Generating a new 32×32 chunk involves ~4,700 noise evaluations (32 columns × ~25 rows × ~5.8 noise calls per block). This creates a brief lag spike when entering new areas | Medium | Could use async generation (generate in background thread, display placeholder until ready) |
| **Memory growth** | Every visited chunk stays in memory forever. Exploring 1000 chunks = 1000 × (32×32 × 1 byte) = 1 MB of blocks + hash map overhead | Low for now | Add chunk unloading: evict chunks beyond draw distance. Use LRU cache |
| **Hash map resize** | As chunk count grows, `RobinHoodMap` needs to resize (double capacity + rehash). This is O(n) | Low | Rare event (only at powers of 2). Robin Hood flat storage makes rehash cache-friendly |
| **2D noise for caves** | `fbm_2d` is more expensive than 1D `fbm` — each octave does 4 hash lookups (bilinear) instead of 2 (linear). Underground blocks are ~60% of a chunk | Medium | Could precompute cave maps at lower resolution and upsample |
| **Noise precision** | Float precision degrades far from origin. At `x > 10,000,000`, `x * frequency` loses precision → terrain becomes blocky | Very Low | Not a practical issue — player would need to walk millions of blocks |

---

## 10. Dynamic Programming for Mining Optimization

> **Note:** DP is planned, not yet implemented. Here's the theory and planned application.

### Simple Answer

> "Dynamic Programming breaks a complex optimization problem into overlapping subproblems, solves each subproblem once, stores the result, and reuses it. I plan to use two DP algorithms: the **0/1 Knapsack** for inventory optimization and **Coin Change** for crafting recipe minimization."

### DP Core Concept

**Without DP (brute force):** Try every possible combination → exponential time (2ⁿ)

**With DP (tabulation):** Build a table bottom-up, where each cell reuses previously computed cells → polynomial time

### Application 1: 0/1 Knapsack → Inventory Optimization

**Problem:** The player's inventory has a weight capacity of W. They have N items, each with a weight and a value. What's the most valuable set of items they can carry?

**Example:**
```
Capacity: 15 kg
Items:
  Diamond  → weight: 5, value: 100
  Gold     → weight: 4, value: 70
  Iron     → weight: 3, value: 40
  Stone    → weight: 2, value: 10
  Dirt     → weight: 1, value: 2
```

**The DP table:**

We build a 2D table `dp[i][w]` where:
- `i` = considering items 0..i
- `w` = remaining capacity
- `dp[i][w]` = maximum value achievable

```
For each item i, for each capacity w:
    if item[i].weight <= w:
        dp[i][w] = max(
            dp[i-1][w],                              // Don't take item i
            dp[i-1][w - item[i].weight] + item[i].value  // Take item i
        )
    else:
        dp[i][w] = dp[i-1][w]  // Can't take it (too heavy)
```

**Result:** Optimal combo is Diamond + Gold + Iron = value 210, weight 12 ≤ 15 ✓

**Time complexity:** O(N × W) — much better than brute force O(2ᴺ)  
**Space complexity:** O(N × W) — can be optimized to O(W) since we only look at the previous row

### Application 2: Coin Change → Crafting Recipe Minimization

**Problem:** To craft a Diamond Pickaxe, you need exactly V = 15 "resource points" worth of materials. Each resource type provides a fixed number of points. What's the **minimum number of raw resources** needed?

**Example:**
```
Target: 15 resource points
Resource values:
  Diamond = 7 points
  Gold    = 4 points
  Iron    = 3 points
  Stone   = 1 point
```

**Greedy fails here!**  
Greedy: Take biggest first → Diamond(7) + Diamond(7) = 14. Need 1 more → Stone(1). Total: 3 items.
But: Gold(4) + Gold(4) + Diamond(7) = 15. Total: 3 items too. 
Actually: Iron(3) + Iron(3) + Iron(3) + Iron(3) + Iron(3) = 15, Total: 5 items → worse.
Better: Diamond(7) + Gold(4) + Gold(4) = 15. Total: 3 items.

**The DP approach:**

```
dp[v] = minimum number of resources to make exactly v points
dp[0] = 0  (base case: 0 points needs 0 resources)

For each amount v from 1 to 15:
    dp[v] = MIN over all resource values c:
        dp[v - c] + 1    (if v - c >= 0)
```

**Building the table:**
```
dp[0] = 0
dp[1] = dp[0] + 1 = 1   (use Stone)
dp[3] = dp[0] + 1 = 1   (use Iron)
dp[4] = dp[0] + 1 = 1   (use Gold)
dp[7] = dp[0] + 1 = 1   (use Diamond)
dp[8] = dp[4] + 1 = 2   (Gold + Gold)
dp[11] = dp[7] + 1 = 2  (Diamond + Gold)
dp[15] = dp[11] + 1 = 3  (Diamond + Gold + Gold)
```

**Time complexity:** O(V × C) where V = target value, C = number of resource types

### Why DP, Not Greedy?

| Approach | Method | Optimal? | Time |
|---|---|---|---|
| **Brute Force** | Try all combos | ✅ Yes | O(Cⱽ) — exponential |
| **Greedy** | Take biggest first | ❌ Not always | O(V) |
| **DP** | Build table bottom-up | ✅ Yes | O(V × C) — polynomial |

**Key insight:** DP guarantees the optimal answer while being orders of magnitude faster than brute force. Greedy is fast but can give wrong answers.

---

## 🎯 Quick-Fire Interview Tips

| If They Ask... | Key Phrases to Use |
|---|---|
| "Why C++?" | "Zero-overhead abstractions, manual memory control, STL, no GC pauses — essential for real-time game engines" |
| "Why no external libraries?" | "I wanted to deeply understand every system. Building from scratch forced me to learn how renderers, hash maps, and physics engines actually work" |
| "What was hardest?" | "Getting BFS pathfinding right with gravity constraints. Mobs can't just float — they need to climb walls, fall through gaps, and stay grounded" |
| "What would you do differently?" | "Add chunk unloading for memory management, use async chunk generation to avoid lag spikes, and implement delta-time physics for framerate independence" |
| "How does this scale?" | "The chunk system is O(1) per block access. Robin Hood map gives O(1) average lookup. SoA lets mob updates scale linearly with 6.55× better constant factor" |
| "Prove the benchmarks" | "I wrote isolated microbenchmarks with `chrono::high_resolution_clock`, used `volatile` accumulators to prevent compiler elimination, and tested with 10K mobs × 10K iterations" |
