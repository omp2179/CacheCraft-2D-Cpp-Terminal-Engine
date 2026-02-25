# Robin Hood Hash Map — Hybrid Hashing Upgrade

Build a **custom Robin Hood Hash Map** that combines the current golden-ratio bitwise hashing with the advanced techniques from the video: Robin Hood linear probing, SWAR metadata scanning, separated scatter/hoist hash functions, branchless operations, and journaled evictions.

## Current State

- **[CoordHash](file:///d:/minecraft_2d_project/include/Coord.h#28-35)** in [Coord.h](file:///d:/minecraft_2d_project/include/Coord.h) — uses `std::hash<int>` XOR with golden ratio (`2654435761u`)
- All hash tables are **`std::unordered_map`** (chained hashing, pointer-heavy, cache-unfriendly)
- Used in: [World.h](file:///d:/minecraft_2d_project/include/World.h) (chunk lookup), [Pathfinding.h](file:///d:/minecraft_2d_project/include/Pathfinding.h) (BFS parent map), [main.cpp](file:///d:/minecraft_2d_project/src/main.cpp) (tests)

## Why This Matters

`std::unordered_map` uses **separate chaining** — each bucket is a linked list node allocated on the heap. This causes:
- **Poor cache locality** — pointer chasing across random memory
- **Extra memory overhead** — each node stores a pointer + hash + key + value
- **Expensive iteration** — nodes scattered across the heap

Robin Hood linear probing with flat storage fixes all of this.

---

## Proposed Changes

### Data Types & Hash Infrastructure

#### [NEW] [RobinHoodMap.h](file:///d:/minecraft_2d_project/include/RobinHoodMap.h)

A templated `RobinHoodMap<K, V, Hash, KeyEqual>` class implementing:

**1. Flat Storage (Cache-Friendly)**
- Contiguous array of `Slot { K key; V value; }` — no heap-allocated nodes
- Separate `uint8_t metadata[]` array — packed PSL + hoisted hash bits
- Power-of-2 capacity — use bitmask instead of modulo (avoid expensive division)

**2. Metadata Encoding (Video 29:30)**
```
┌──────────────────────────┐
│ Metadata byte per slot   │
│ [HH HH H PPP]           │
│  ↑        ↑              │
│  5 hoisted│3-bit PSL     │
│  hash bits│(0-6 = probe  │
│           │ 7 = empty)   │
└──────────────────────────┘
```
- **PSL 0-6**: probe sequence length (how far from home)
- **PSL 7 (0b111)**: empty sentinel — slot is unoccupied
- **Top 5 bits**: hoisted hash bits for fast rejection without key comparison

**3. Separated Hash: Scatter + Hoist (Video 35:54)**
- `scatter(key)` → home index (uses existing golden-ratio mixing for excellent distribution)
- `hoist(key)` → 5 metadata bits (cheap bit extraction from a different portion of the hash, increases entropy)
- This avoids using a single expensive hash and instead picks the cheapest mixer for each purpose

**4. Robin Hood Insertion with Journaled Evictions (Video 6:05 + 44:24)**
- On insert: compute PSL, walk forward. If current slot's PSL < our PSL → evict (Robin Hood rule: "rich give to poor")
- **Journal** eviction moves in a small stack-allocated buffer, then apply all moves at the end — more prefetch-friendly than immediate swaps (Video 44:24)

**5. SWAR-Accelerated Lookup (Video 17:23 + 56:52)**
- Pack 8 metadata bytes into a `uint64_t` register
- Broadcast the search byte (target hoisted hash + expected PSL) and XOR → matching bytes become zero
- Use bitwise tricks to find zero-bytes in one shot — checks 8 slots in parallel
- Falls back to simple linear scan if < 8 slots remain

**6. Branchless Operations (Video 36:06, 40:48)**
- [find()](file:///d:/minecraft_2d_project/include/Pathfinding.h#9-98) uses SWAR scan → reduces conditional branches for metadata comparison
- Home index computed with bitmask (`hash & (capacity - 1)`) instead of modulo (Video 24:52: divisions are very expensive)
- Early termination: if the slot's PSL < our expected PSL, key is definitely not present (Robin Hood invariant)

**7. Growth & Rehash**
- When load factor exceeds ~87% (a sweet spot for Robin Hood), allocate a new array at 2× capacity and reinsert all entries
- No max PSL limit needed if we grow before it gets too high

**API** (drop-in replacement for `std::unordered_map` usage patterns in the project):
```cpp
V& operator[](const K& key);     // insert-or-access
iterator find(const K& key);     // lookup
size_t count(const K& key);      // 0 or 1
size_t size() const;
iterator begin() / end();        // for range-for loops
void erase(const K& key);
```

---

#### [MODIFY] [Coord.h](file:///d:/minecraft_2d_project/include/Coord.h)

Keep existing [CoordHash](file:///d:/minecraft_2d_project/include/Coord.h#28-35) unchanged — it will be reused as the hash functor for `RobinHoodMap`. The golden-ratio mixing is already a good scatter function. We'll add an inline `coord_hash_raw(Coord)` helper that returns the full `size_t` hash for both scatter and hoist extraction.

#### [MODIFY] [World.h](file:///d:/minecraft_2d_project/include/World.h)

Replace:
```cpp
std::unordered_map<Coord, std::unique_ptr<Chunk>, CoordHash> chunks;
```
With:
```cpp
RobinHoodMap<Coord, std::unique_ptr<Chunk>, CoordHash> chunks;
```

#### [MODIFY] [Pathfinding.h](file:///d:/minecraft_2d_project/include/Pathfinding.h)

Replace:
```cpp
std::unordered_map<Coord, Coord, CoordHash> parent;
```
With:
```cpp
RobinHoodMap<Coord, Coord, CoordHash> parent;
```

---

### Benchmarking

#### [NEW] [HashBenchmark.h](file:///d:/minecraft_2d_project/include/HashBenchmark.h)

Benchmark comparing `std::unordered_map<Coord, int, CoordHash>` vs `RobinHoodMap<Coord, int, CoordHash>`:

| Test | Operations | What it measures |
|------|-----------|-----------------|
| **Sequential Insert** | 100,000 Coord insertions | Insertion throughput |
| **Random Lookup (Hit)** | 100,000 lookups of existing keys | Cache-friendly lookup speed |
| **Random Lookup (Miss)** | 100,000 lookups of non-existing keys | Early termination via PSL |
| **Iteration** | Iterate all 100,000 entries | Flat array vs pointer chasing |

Target: **2-5× faster lookups**, **3-10× faster iteration** vs `std::unordered_map`.

#### [MODIFY] [main.cpp](file:///d:/minecraft_2d_project/src/main.cpp)

- Add `#include "RobinHoodMap.h"` and `#include "HashBenchmark.h"`
- Add `test_robinhood()` — correctness tests for the new map
- Add `run_hash_benchmark()` call alongside existing AoS/SoA benchmark
- Update test_coord() and test_integration() to also exercise `RobinHoodMap` alongside existing `unordered_map` tests

---

### Documentation

#### [MODIFY] [PROJECT_ROADMAP.md](file:///d:/minecraft_2d_project/PROJECT_ROADMAP.md)

Add a new Advanced CS section:

> ### 5. Robin Hood Hash Map — Cache-Friendly Custom HashMap
> Resume: *"Custom HashMaps"*
> - **Where:** Chunk lookup, BFS pathfinding, world storage
> - **Techniques:** Robin Hood probing, SWAR metadata scanning, separated scatter/hoist, journaled evictions, branchless ops
> - **Benchmark:** Target 2-5× faster lookups vs `std::unordered_map`

---

## User Review Required

> [!IMPORTANT]
> This replaces `std::unordered_map` with a custom hash map in [World.h](file:///d:/minecraft_2d_project/include/World.h) and [Pathfinding.h](file:///d:/minecraft_2d_project/include/Pathfinding.h). The existing `std::unordered_map` tests in [main.cpp](file:///d:/minecraft_2d_project/src/main.cpp) will be kept as-is for comparison. The `RobinHoodMap` must support `std::unique_ptr<Chunk>` as values (move-only types).

> [!NOTE]
> The `RobinHoodMap` will be a **header-only** implementation (consistent with the project's header-only structure). SWAR tricks require `uint64_t` support (guaranteed on your platform with g++ C++23).

---

## Verification Plan

### Automated Tests (via compile + run)

**Command:**
```
cd d:\minecraft_2d_project && compile.bat
```

This compiles and runs the game, which executes all `test_*` functions. We will verify:

1. **[test_coord()](file:///d:/minecraft_2d_project/src/main.cpp#37-75)** — existing HashMap tests still pass (exercises [CoordHash](file:///d:/minecraft_2d_project/include/Coord.h#28-35) + `unordered_map`)
2. **`test_robinhood()`** (new) — correctness tests for `RobinHoodMap`:
   - Insert 1000 key-value pairs, verify all lookable
   - Delete half, verify deletions and remaining
   - Edge cases: duplicate keys, non-existent lookups, empty map
   - Move-only values (`unique_ptr`) work correctly
   - Growth/rehash triggers correctly
3. **[test_integration()](file:///d:/minecraft_2d_project/src/main.cpp#152-192)** — world map operations still work end-to-end
4. **[test_world()](file:///d:/minecraft_2d_project/src/main.cpp#285-326)** — chunk auto-creation, negative coords, mining (now backed by `RobinHoodMap`)
5. **[test_terrain()](file:///d:/minecraft_2d_project/src/main.cpp#239-284)** — FBM terrain generation still deterministic and correct
6. **Hash benchmark output** — verify `RobinHoodMap` shows measurable speedup

### Manual Verification
- Run the game and play for 30+ seconds — walk, mine, build, open inventory, open pause menu — to confirm nothing is broken from the hash map swap.
