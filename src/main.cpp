#include "Benchmark.h"
#include "BlockType.h"
#include "CheatState.h"
#include "CheatWindow.h"
#include "Chunk.h"
#include "Coord.h"
#include "FastRand.h"
#include "GameWindow.h"
#include "HashBenchmark.h"
#include "Input.h"
#include "InventoryWindow.h"
#include "PauseWindow.h"
#include "Pixel.h"
#include "RobinHoodMap.h"
#include "ScreenBuffer.h"
#include "World.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <stack>
#include <string>
#include <unordered_map>

// THIS enables colored output on Windows terminal
#ifdef _WIN32
#include <windows.h>
inline void enable_virtual_terminal() {
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD mode = 0;
  GetConsoleMode(hOut, &mode);
  SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}
#endif

using namespace std;

void test_coord() {
  cout << "=== COORD TESTS ===\n";

  // 1. Construction
  Coord a = {3, 4};
  Coord b = {1, 2};
  Coord origin; // should be (0, 0) from defaults
  cout << "a = " << a << ", b = " << b << ", origin = " << origin << "\n";

  // 2. Arithmetic
  Coord sum = a + b;
  Coord diff = a - b;
  cout << a << " + " << b << " = " << sum << "\n";
  cout << a << " - " << b << " = " << diff << "\n";
  assert(sum.x == 4 && sum.y == 6);
  assert(diff.x == 2 && diff.y == 2);

  // 3. Equality
  Coord a_copy = {3, 4};
  assert(a == a_copy);
  assert(!(a == b));
  cout << a << " == " << a_copy << " : true\n";

  // 4. HashMap with CoordHash (std::unordered_map — legacy)
  unordered_map<Coord, string, CoordHash> map;
  map[{0, 0}] = "Origin";
  map[{10, 5}] = "Chunk_1_0";
  map[{-5, 3}] = "Negative";
  auto val1 = map[{0, 0}];
  assert(val1 == "Origin");
  auto val2 = map[{10, 5}];
  assert(val2 == "Chunk_1_0");
  auto val3 = map.count({99, 99});
  assert(val3 == 0);
  cout << "HashMap (unordered_map): 3 inserted, lookup works\n";

  // 5. RobinHoodMap with CoordHash (new!)
  RobinHoodMap<Coord, string, CoordHash> rmap;
  rmap[{0, 0}] = "Origin";
  rmap[{10, 5}] = "Chunk_1_0";
  rmap[{-5, 3}] = "Negative";
  assert((rmap[{0, 0}] == "Origin"));
  assert((rmap[{10, 5}] == "Chunk_1_0"));
  assert((rmap.count({99, 99}) == 0));
  cout << "HashMap (RobinHoodMap): 3 inserted, lookup works\n";

  cout << "All Coord tests PASSED!\n\n";
}

void test_blocktype() {
  cout << "=== BLOCKTYPE TESTS ===\n";

  // 1. Size check — must be 1 byte
  assert(sizeof(BlockType) == 1);
  cout << "Size: " << sizeof(BlockType) << " byte (uint8_t confirmed)\n";

  // 2. COUNT value
  assert(static_cast<int>(BlockType::COUNT) == 10);
  cout << "Total types: " << static_cast<int>(BlockType::COUNT) << "\n";

  // 3. Char mapping
  assert(block_to_char(BlockType::STONE) == '#');
  assert(block_to_char(BlockType::DIAMOND) == 'D');
  assert(block_to_char(BlockType::AIR) == ' ');
  cout << "Char mapping: correct\n";

  // 4. String mapping
  assert(block_to_string(BlockType::GOLD) == "Gold");
  assert(block_to_string(BlockType::GRASS) == "Grass");
  cout << "String mapping: correct\n";

  // 5. Print all types
  for (int i = 0; i < static_cast<int>(BlockType::COUNT); i++) {
    BlockType b = static_cast<BlockType>(i);
    cout << "  [" << block_to_char(b) << "] " << block_to_string(b) << "\n";
  }

  cout << "All BlockType tests PASSED!\n\n";
}

void test_pixel() {
  cout << "=== PIXEL TESTS ===\n";

  // 1. Size check — must be 2 bytes
  assert(sizeof(Pixel) == 2);
  cout << "Size: " << sizeof(Pixel) << " bytes (char + Color)\n";

  // 2. Default pixel is white space
  Pixel empty;
  assert(empty.ch == ' ');
  assert(empty.color == Color::WHITE);
  cout << "Default pixel: white space (transparent) confirmed\n";

  // 3. block_to_pixel mapping
  Pixel grass = block_to_pixel(BlockType::GRASS);
  assert(grass.ch == '"');
  assert(grass.color == Color::BRIGHT_GREEN);

  Pixel diamond = block_to_pixel(BlockType::DIAMOND);
  assert(diamond.ch == 'D');
  assert(diamond.color == Color::BRIGHT_CYAN);
  cout << "block_to_pixel mapping: correct\n";

  // 4. VISUAL TEST — colored output!
  cout << "\nColored block display:\n";
  for (int i = 0; i < static_cast<int>(BlockType::COUNT); i++) {
    BlockType b = static_cast<BlockType>(i);
    Pixel p = block_to_pixel(b);
    cout << "  " << p << "  " << block_to_string(b) << "\n";
  }

  // 5. Mini world preview — a row of blocks!
  cout << "\nMini terrain row: ";
  BlockType row[] = {BlockType::AIR,    BlockType::AIR,   BlockType::GRASS,
                     BlockType::DIRT,   BlockType::STONE, BlockType::STONE,
                     BlockType::IRON,   BlockType::GOLD,  BlockType::DIAMOND,
                     BlockType::BEDROCK};
  for (auto b : row) {
    cout << block_to_pixel(b);
  }
  cout << "\n";

  cout << "All Pixel tests PASSED!\n\n";
}

// INTEGRATION: All three types working together
void test_integration() {
  cout << "=== INTEGRATION TEST ===\n";

  // Simulate: "What block is at world position (25, 7)?"
  Coord world_pos = {25, 7};

  // Which chunk is this in? (chunk size = 10)
  Coord chunk_pos = {world_pos.x / 10, world_pos.y / 10};

  // What's the local position within the chunk?
  Coord local_pos = {world_pos.x % 10, world_pos.y % 10};

  cout << "World pos: " << world_pos << "\n";
  cout << "Chunk pos: " << chunk_pos << " (chunk 2, 0)\n";
  cout << "Local pos: " << local_pos << " (cell 5, 7)\n";

  // Pretend this block is diamond
  BlockType block = BlockType::DIAMOND;
  Pixel visual = block_to_pixel(block);

  cout << "Block at " << world_pos << " is " << block_to_string(block) << " -> "
       << visual << "\n";

  // Store in a world map (using RobinHoodMap!)
  RobinHoodMap<Coord, BlockType, CoordHash> world;
  world[world_pos] = BlockType::DIAMOND;
  world[{0, 0}] = BlockType::GRASS;
  world[{0, 5}] = BlockType::STONE;

  cout << "World map has " << world.size() << " blocks stored\n";

  // Retrieve and display
  for (auto [pos, type] : world) {
    cout << "  " << pos << " : " << block_to_pixel(type) << " "
         << block_to_string(type) << "\n";
  }

  cout << "Integration test PASSED!\n";
}

void test_chunk() {
  cout << "\n=== CHUNK TESTS ===\n";

  // 1. Create a chunk — terrain is now procedural!
  Chunk chunk({0, 0});
  cout << "Chunk (0,0):\n";
  print_chunk(chunk);

  // 2. Different chunk position → different terrain
  Chunk chunk2({1, 0});
  cout << "\nChunk (1,0):\n";
  print_chunk(chunk2);

  // 3. Bedrock is ALWAYS at bottom row (regardless of noise)
  for (int x = 0; x < CHUNK_SIZE; x++) {
    assert(chunk.get_block(x, CHUNK_SIZE - 1) == BlockType::BEDROCK);
  }
  cout << "Bedrock layer: always at row " << CHUNK_SIZE - 1 << " - correct\n";

  // 4. Top rows should be AIR
  for (int x = 0; x < CHUNK_SIZE; x++) {
    assert(chunk.get_block(x, 0) == BlockType::AIR);
  }
  cout << "Sky layer: row 0 always AIR - correct\n";

  // 5. Deterministic — same position + seed = same terrain
  Chunk chunk_copy({0, 0});
  bool identical = true;
  for (int y = 0; y < CHUNK_SIZE; y++) {
    for (int x = 0; x < CHUNK_SIZE; x++) {
      if (chunk.get_block(x, y) != chunk_copy.get_block(x, y)) {
        identical = false;
      }
    }
  }
  assert(identical);
  cout << "Deterministic generation: same seed = same world - correct\n";

  // 6. Mining and building still work
  chunk.set_block(5, 7, BlockType::AIR);
  assert(chunk.get_block(5, 7) == BlockType::AIR);
  cout << "Mining: correct\n";

  cout << "All Chunk tests PASSED!\n";
}

void test_terrain() {
  cout << "\n=== TERRAIN TESTS ===\n";

  // 1. Print 5 chunks side by side (50 columns wide!)
  World world;
  cout << "World view (5 chunks, x: 0-49):\n";
  print_world(world, 0, 49, 0, CHUNK_SIZE - 1);

  // 2. Count ores in the visible area
  int iron = 0, gold = 0, diamond = 0;
  for (int x = 0; x < 50; x++) {
    for (int y = 0; y < CHUNK_SIZE; y++) {
      BlockType b = world.get_block(x, y);
      if (b == BlockType::IRON)
        iron++;
      else if (b == BlockType::GOLD)
        gold++;
      else if (b == BlockType::DIAMOND)
        diamond++;
    }
  }
  cout << "Ores found: Iron=" << iron << " Gold=" << gold
       << " Diamond=" << diamond << "\n";
  cout << "Iron > Gold > Diamond? "
       << (iron >= gold && gold >= diamond ? "yes" : "no") << "\n";

  // 3. Verify noise is deterministic
  float a = fbm(25.0f, 42);
  float b = fbm(25.0f, 42);
  assert(a == b);
  cout << "Noise determinism: correct\n";

  // 4. Verify noise is smooth (neighbors differ by < 0.3)
  bool smooth = true;
  for (int x = 0; x < 100; x++) {
    float v1 = fbm(static_cast<float>(x), 42);
    float v2 = fbm(static_cast<float>(x + 1), 42);
    if (std::abs(v1 - v2) > 0.3f)
      smooth = false;
  }
  assert(smooth);
  cout << "Noise smoothness: correct\n";

  cout << "All Terrain tests PASSED!\n";
}

void test_world() {
  cout << "\n=== WORLD TESTS ===\n";

  World world;

  // 1. World starts empty
  assert(world.chunk_count() == 0);
  cout << "Empty world: 0 chunks\n";

  // 2. Access a block — chunk auto-created (lazy loading!)
  BlockType b = world.get_block(25, 7);
  cout << "Block at (25, 7): " << block_to_string(b) << "\n";
  assert(world.chunk_count() == 1);
  cout << "After first access: " << world.chunk_count() << " chunk loaded\n";

  // 3. Access same chunk — no new chunk created
  world.get_block(20, 3); // still in chunk (2, 0)
  assert(world.chunk_count() == 1);
  cout << "Same chunk access: still " << world.chunk_count() << " chunk\n";

  // 4. Access different chunk — new chunk auto-created
  world.get_block(35, 7); // chunk (3, 0)
  assert(world.chunk_count() == 2);
  cout << "Different chunk: " << world.chunk_count() << " chunks now\n";

  // 5. Negative coordinates work
  world.get_block(-5, -15);
  assert(world.chunk_count() == 3);
  cout << "Negative coords: " << world.chunk_count() << " chunks\n";

  // 6. Mining in world coordinates
  world.set_block(25, 7, BlockType::AIR);
  assert(world.get_block(25, 7) == BlockType::AIR);
  cout << "Mining at (25,7): correct\n";

  // 7. Print a slice of the world (3 chunks wide)
  cout << "\nWorld view (x: 0-29, y: 0-9):\n";
  print_world(world, 0, 29, 0, 9);

  cout << "All World tests PASSED!\n";
}

void test_screenbuffer() {
  cout << "\n=== SCREENBUFFER TESTS ===\n";

  ScreenBuffer screen;

  // 1. Size check
  cout << "Screen size: " << SCREEN_WIDTH << "x" << SCREEN_HEIGHT << "\n";
  assert(SCREEN_WIDTH == 80);
  assert(SCREEN_HEIGHT == 24);
  cout << "Size constants: correct\n";

  // 2. Clear fills with empty white spaces
  screen.clear();
  Pixel p = screen.get_pixel(0, 0);
  assert(p.ch == ' ' && p.color == Color::WHITE);
  p = screen.get_pixel(79, 23);
  assert(p.ch == ' ' && p.color == Color::WHITE);
  cout << "Clear: all pixels empty white space - correct\n";

  // 3. Set pixel and read it back
  screen.set_pixel(10, 5, {'@', Color::CYAN});
  p = screen.get_pixel(10, 5);
  assert(p.ch == '@');
  assert(p.color == Color::CYAN);
  cout << "Set/get pixel: correct\n";

  // 4. Out-of-bounds set is silently ignored
  screen.set_pixel(-1, -1, {'X', Color::RED});
  screen.set_pixel(999, 999, {'X', Color::RED});
  // No crash = success
  cout << "Out-of-bounds set_pixel: safely ignored\n";

  // 5. Out-of-bounds get returns empty pixel
  p = screen.get_pixel(-5, -5);
  assert(p.ch == ' ' && p.color == Color::WHITE);
  p = screen.get_pixel(999, 0);
  assert(p.ch == ' ' && p.color == Color::WHITE);
  cout << "Out-of-bounds get_pixel: returns empty - correct\n";

  // 6. Draw text and verify each character
  screen.clear();
  screen.draw_text(3, 0, "HI", Color::GREEN);
  Pixel h = screen.get_pixel(3, 0);
  Pixel i = screen.get_pixel(4, 0);
  Pixel after = screen.get_pixel(5, 0);
  assert(h.ch == 'H' && h.color == Color::GREEN);
  assert(i.ch == 'I' && i.color == Color::GREEN);
  assert(after.ch == ' '); // untouched pixel stays empty
  cout << "Draw text: characters placed correctly\n";

  // 7. Draw a scene and verify key pixels
  screen.clear();
  // Grass row
  for (int x = 0; x < SCREEN_WIDTH; x++) {
    screen.set_pixel(x, 5, {'"', Color::GREEN});
  }
  // Player
  screen.set_pixel(40, 4, {'@', Color::CYAN});
  // HUD
  screen.draw_text(0, 23, "HP:100", Color::YELLOW);

  // Verify grass
  p = screen.get_pixel(0, 5);
  assert(p.ch == '"' && p.color == Color::GREEN);
  p = screen.get_pixel(79, 5);
  assert(p.ch == '"' && p.color == Color::GREEN);

  // Verify player
  p = screen.get_pixel(40, 4);
  assert(p.ch == '@' && p.color == Color::CYAN);

  // Verify HUD
  p = screen.get_pixel(0, 23);
  assert(p.ch == 'H' && p.color == Color::YELLOW);
  p = screen.get_pixel(1, 23);
  assert(p.ch == 'P' && p.color == Color::YELLOW);

  // Verify sky (untouched = empty)
  p = screen.get_pixel(0, 0);
  assert(p.ch == ' ');
  cout << "Scene verification: all layers correct\n";

  cout << "All ScreenBuffer tests PASSED!\n";

  // 8. VISUAL DEMO — render ACTUAL procedural terrain!
  // 5 frames, each showing a different part of the world
  // This connects: World → Chunk → Terrain(FBM) → Pixel → ScreenBuffer →
  // render()
  cout << "\nVisual demo: 5 frames of real FBM terrain (5 sec each)...\n";
  cout << "(Screen will clear in 2 seconds)\n";

#ifdef _WIN32
  Sleep(2000);
  system("cls");

  World demo_world;

  // 5 frames: camera moves east by 80 blocks each frame
  int camera_positions[] = {0, 80, 160, -80, 300};
  string labels[] = {"Frame 1/5: World origin (x: 0-79)",
                     "Frame 2/5: Walking east (x: 80-159)",
                     "Frame 3/5: Far east (x: 160-239)",
                     "Frame 4/5: Negative world! (x: -80 to -1)",
                     "Frame 5/5: Deep east (x: 300-379)"};

  for (int frame = 0; frame < 5; frame++) {
    int cam_x = camera_positions[frame];

    screen.clear();

    // Title
    screen.draw_text(25, 0, "MINECRAFT 2D - TERRAIN VIEWER", Color::GREEN);

    // Render actual world terrain using FBM noise!
    // Screen rows 2-21 show world rows 0-9 (scaled 2x vertically for
    // visibility)
    for (int sx = 0; sx < SCREEN_WIDTH; sx++) {
      int world_x = cam_x + sx;

      for (int wy = 0; wy < 10; wy++) {
        BlockType block = demo_world.get_block(world_x, wy);
        Pixel pixel = block_to_pixel(block);

        // Draw each world row as 2 screen rows (so 10 world rows = 20 screen
        // rows)
        int screen_y = 2 + wy * 2;
        screen.set_pixel(sx, screen_y, pixel);
        screen.set_pixel(sx, screen_y + 1, pixel);
      }
    }

    // Player marker in the middle
    screen.set_pixel(40, 2, {'V', Color::CYAN});

    // HUD
    screen.draw_text(0, 22, labels[frame], Color::CYAN);

    string pos_info = "Camera X: " + std::to_string(cam_x) + " to " +
                      std::to_string(cam_x + 79) + "  Chunks loaded: " +
                      std::to_string(demo_world.chunk_count());
    screen.draw_text(0, 23, pos_info, Color::YELLOW);

    screen.render();
    Sleep(5000);
  }

  system("cls");
  cout << "Visual demo complete! 5 frames of FBM terrain rendered!\n";
  cout << "Total chunks loaded: " << demo_world.chunk_count() << "\n";
#endif
}

void test_robinhood() {
  cout << "\n=== ROBINHOOD MAP TESTS ===\n";

  // 1. Basic insert and lookup
  RobinHoodMap<Coord, int, CoordHash> rmap;
  rmap[{0, 0}] = 42;
  rmap[{1, 2}] = 99;
  rmap[{-5, 3}] = 77;
  assert((rmap[{0, 0}] == 42));
  assert((rmap[{1, 2}] == 99));
  assert((rmap[{-5, 3}] == 77));
  assert(rmap.size() == 3);
  cout << "Basic insert/lookup: correct\n";

  // 2. Count (exists vs missing)
  assert((rmap.count({0, 0}) == 1));
  assert((rmap.count({999, 999}) == 0));
  cout << "Count: correct\n";

  // 3. Find
  auto it = rmap.find({1, 2});
  assert(it != rmap.end());
  auto [k, v] = *it;
  assert(k.x == 1 && k.y == 2);
  assert(v == 99);
  assert((rmap.find({888, 888}) == rmap.end()));
  cout << "Find: correct\n";

  // 4. Overwrite existing key
  rmap[{0, 0}] = 100;
  assert((rmap[{0, 0}] == 100));
  assert(rmap.size() == 3); // size unchanged
  cout << "Overwrite: correct\n";

  // 5. Erase
  assert((rmap.erase({1, 2}) == true));
  assert((rmap.count({1, 2}) == 0));
  assert(rmap.size() == 2);
  assert((rmap.erase({999, 999}) == false)); // erase non-existent
  cout << "Erase: correct\n";

  // 6. Growth/rehash — insert many entries to trigger multiple grows
  RobinHoodMap<Coord, int, CoordHash> big_map;
  for (int i = 0; i < 1000; ++i) {
    Coord bk = {i, i * 3};
    big_map[bk] = i;
  }
  assert(big_map.size() == 1000);
  // Verify all still findable after rehashes
  bool all_found = true;
  for (int i = 0; i < 1000; ++i) {
    Coord bk = {i, i * 3};
    if (big_map.count(bk) != 1) {
      all_found = false;
      break;
    }
    if (big_map[bk] != i) {
      all_found = false;
      break;
    }
  }
  assert(all_found);
  cout << "Growth/rehash (1000 entries): correct\n";

  // 7. Iteration — count entries via range-for
  int iter_count = 0;
  for (auto [key, val] : big_map) {
    (void)key;
    (void)val;
    ++iter_count;
  }
  assert(iter_count == 1000);
  cout << "Iteration: correct (" << iter_count << " entries)\n";

  // 8. Negative coordinates (important for Minecraft chunks)
  RobinHoodMap<Coord, string, CoordHash> neg_map;
  neg_map[{-10, -20}] = "neg_chunk";
  neg_map[{-1, 0}] = "border";
  assert((neg_map[{-10, -20}] == "neg_chunk"));
  assert((neg_map[{-1, 0}] == "border"));
  cout << "Negative coords: correct\n";

  // 9. Clear
  big_map.clear();
  assert(big_map.size() == 0);
  assert(big_map.empty());
  assert((big_map.find({0, 0}) == big_map.end()));
  cout << "Clear: correct\n";

  // 10. Empty map operations
  RobinHoodMap<Coord, int, CoordHash> empty_map;
  assert(empty_map.size() == 0);
  assert(empty_map.empty());
  assert((empty_map.find({0, 0}) == empty_map.end()));
  assert((empty_map.count({0, 0}) == 0));
  assert((empty_map.erase({0, 0}) == false));
  cout << "Empty map ops: correct\n";

  cout << "All RobinHood Map tests PASSED!\n";
}

int main() {
#ifdef _WIN32
  enable_virtual_terminal();
#endif

  test_coord();
  test_blocktype();
  test_pixel();
  test_integration();
  test_chunk();
  test_world();
  test_terrain();
  test_robinhood();
  // test_screenbuffer();
  run_aos_vs_soa_benchmark();
  run_hash_benchmark();

  cout << "\n=== ALL TESTS PASSED! ===\n";
  cout << "Starting game in 3 seconds...\n";
#ifdef _WIN32
  Sleep(3000);
  system("cls");
#endif

  World world;
  ScreenBuffer screen;
  CheatState cheats;

  int player_x = 40;
  int player_y = 0;
  int facing = 1;
  int inventory[9] = {0};
  int selected_block = 1;

  while (player_y < CHUNK_SIZE - 1 &&
         world.get_block(player_x, player_y) == BlockType::AIR) {
    ++player_y;
  }
  --player_y;

  seed_fast_rand(static_cast<unsigned>(time(nullptr)));

  GameWindow game_window(world, player_x, player_y, facing, inventory,
                         selected_block, cheats);

  InventoryWindow inv_window(inventory, selected_block);

  PauseWindow pause_window;

  CheatWindow cheat_window(cheats, inventory);

  std::stack<Window *> windows;
  windows.push(&game_window);

  while (!windows.empty()) {
    InputState input = get_input();

    bool should_close = windows.top()->handle_input(input);
    if (should_close) {
      windows.pop();
      if (windows.empty())
        break;
    }

    if (windows.top() == &game_window && game_window.wants_inventory) {
      game_window.wants_inventory = false;
      windows.push(&inv_window);
    }

    if (windows.top() == &game_window && game_window.wants_pause) {
      game_window.wants_pause = false;
      windows.push(&pause_window);
    }

    if (windows.top() == &pause_window && pause_window.wants_cheat) {
      pause_window.wants_cheat = false;
      windows.pop();
      windows.push(&cheat_window);
    }

    if (windows.top() == &pause_window && pause_window.wants_quit) {
      break;
    }

    windows.top()->render(screen);
    screen.render();

#ifdef _WIN32
    Sleep(50);
#endif
  }

#ifdef _WIN32
  system("cls");
#endif
  cout << "Thanks for playing! Total chunks explored: " << world.chunk_count()
       << "\n";

  return 0;
}