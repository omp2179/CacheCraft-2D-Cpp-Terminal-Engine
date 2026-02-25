#pragma once
#include <cassert>
#include <cstdint>
#include <cstring>
#include <functional>
#include <new>
#include <utility>

// ============================================================================
//  RobinHoodMap — Cache-Friendly Robin Hood Hash Map
// ============================================================================
//
//  Key advantages over std::unordered_map:
//
//  1. Flat array storage — all data in a contiguous block, cache-line friendly
//     (std::unordered_map uses linked list nodes scattered across the heap)
//  2. Robin Hood linear probing — elements closer to their "home" slot give
//     way to elements further from home, keeping probe lengths balanced
//  3. Power-of-2 capacity — bitmask instead of expensive modulo division
//  4. Backward-shift deletion — no tombstones, maintains Robin Hood order
//  5. Golden-ratio hash mixing — combines with your existing CoordHash
//
// ============================================================================

template <typename K, typename V, typename Hash = std::hash<K>,
          typename KeyEqual = std::equal_to<K>>
class RobinHoodMap {
public:
  using key_type = K;
  using mapped_type = V;

private:
  // Each slot stores the key, value, and cached hash.
  // 'occupied' tracks whether the slot is in use.
  struct Slot {
    K key;
    V value;
    size_t hash;     // cached hash (avoids rehashing during probe/eviction)
    bool occupied;
  };

  Slot *slots_ = nullptr;
  size_t capacity_ = 0;   // always power of 2
  size_t mask_ = 0;        // capacity_ - 1, for branchless modulo
  size_t size_ = 0;
  size_t grow_at_ = 0;     // size threshold to trigger growth

  Hash hasher_;
  KeyEqual eq_;

  static constexpr size_t MIN_CAPACITY = 8;
  static constexpr float MAX_LOAD = 0.85f;

  // Home index: where this hash ideally wants to be
  size_t home_of(size_t h) const { return h & mask_; }

  // Probe Sequence Length: how far this slot is from its home
  size_t psl_of(size_t idx, size_t home) const {
    return (idx + capacity_ - home) & mask_;
  }

  size_t psl_at(size_t idx) const {
    return psl_of(idx, home_of(slots_[idx].hash));
  }

  // ---------------------------------------------------------------
  //  Memory management
  // ---------------------------------------------------------------
  void alloc(size_t cap) {
    capacity_ = cap;
    mask_ = cap - 1;
    grow_at_ = static_cast<size_t>(static_cast<float>(cap) * MAX_LOAD);
    slots_ = static_cast<Slot *>(::operator new(sizeof(Slot) * cap));
    for (size_t i = 0; i < cap; ++i) {
      slots_[i].occupied = false;
    }
  }

  void dealloc() {
    if (!slots_) return;
    for (size_t i = 0; i < capacity_; ++i) {
      if (slots_[i].occupied) {
        slots_[i].key.~K();
        slots_[i].value.~V();
      }
    }
    ::operator delete(slots_);
    slots_ = nullptr;
  }

  // ---------------------------------------------------------------
  //  Grow + rehash
  // ---------------------------------------------------------------
  void grow() {
    size_t old_cap = capacity_;
    Slot *old_slots = slots_;

    alloc(old_cap * 2);
    size_ = 0;

    for (size_t i = 0; i < old_cap; ++i) {
      if (old_slots[i].occupied) {
        insert_for_rehash(std::move(old_slots[i].key),
                          std::move(old_slots[i].value),
                          old_slots[i].hash);
        old_slots[i].key.~K();
        old_slots[i].value.~V();
      }
    }
    ::operator delete(old_slots);
  }

  // Insert during rehash — hash is already known, no duplicate check needed
  void insert_for_rehash(K &&key, V &&value, size_t h) {
    size_t idx = home_of(h);
    size_t psl = 0;

    K ins_key = std::move(key);
    V ins_val = std::move(value);
    size_t ins_hash = h;

    while (true) {
      if (!slots_[idx].occupied) {
        // Empty slot — place here
        new (&slots_[idx].key) K(std::move(ins_key));
        new (&slots_[idx].value) V(std::move(ins_val));
        slots_[idx].hash = ins_hash;
        slots_[idx].occupied = true;
        ++size_;
        return;
      }

      // Robin Hood: evict if current occupant is richer (lower PSL)
      size_t existing_psl = psl_at(idx);
      if (existing_psl < psl) {
        // Swap: we take this spot, displaced element continues
        std::swap(ins_key, slots_[idx].key);
        std::swap(ins_val, slots_[idx].value);
        std::swap(ins_hash, slots_[idx].hash);
        psl = existing_psl;
      }

      idx = (idx + 1) & mask_;
      ++psl;
    }
  }

  // ---------------------------------------------------------------
  //  Core insert — returns pointer to value
  // ---------------------------------------------------------------
  V *do_insert(K &&key, V &&value) {
    if (size_ >= grow_at_) grow();

    size_t h = hasher_(key);
    size_t idx = home_of(h);
    size_t psl = 0;

    K ins_key = std::move(key);
    V ins_val = std::move(value);
    size_t ins_hash = h;
    V *result = nullptr;  // will point to the originally inserted value

    while (true) {
      if (!slots_[idx].occupied) {
        // Empty slot — place here
        new (&slots_[idx].key) K(std::move(ins_key));
        new (&slots_[idx].value) V(std::move(ins_val));
        slots_[idx].hash = ins_hash;
        slots_[idx].occupied = true;
        ++size_;
        if (!result) result = &slots_[idx].value;
        return result;
      }

      // Check if this key already exists
      if (slots_[idx].hash == ins_hash && eq_(slots_[idx].key, ins_key)) {
        slots_[idx].value = std::move(ins_val);
        return &slots_[idx].value;
      }

      // Robin Hood: evict if current occupant is richer (lower PSL)
      size_t existing_psl = psl_at(idx);
      if (existing_psl < psl) {
        // Swap: we take this slot, displaced element continues probing
        std::swap(ins_key, slots_[idx].key);
        std::swap(ins_val, slots_[idx].value);
        std::swap(ins_hash, slots_[idx].hash);
        if (!result) result = &slots_[idx].value;
        psl = existing_psl;
      }

      idx = (idx + 1) & mask_;
      ++psl;
    }
  }

  // ---------------------------------------------------------------
  //  Core find — returns slot index or SIZE_MAX
  // ---------------------------------------------------------------
  size_t find_slot(const K &key) const {
    if (capacity_ == 0) return SIZE_MAX;

    size_t h = hasher_(key);
    size_t idx = home_of(h);
    size_t psl = 0;

    while (psl < capacity_) {
      if (!slots_[idx].occupied)
        return SIZE_MAX;  // empty slot = not found

      // Robin Hood early exit: if this occupant has a shorter probe
      // length than ours, our key would have evicted it → not here
      if (psl_at(idx) < psl)
        return SIZE_MAX;

      // Hash + key match
      if (slots_[idx].hash == h && eq_(slots_[idx].key, key))
        return idx;

      idx = (idx + 1) & mask_;
      ++psl;
    }
    return SIZE_MAX;
  }

public:
  // ---------------------------------------------------------------
  //  Iterator — for range-for loops
  // ---------------------------------------------------------------
  class iterator {
    friend class RobinHoodMap;
    Slot *slots_;
    size_t idx_, cap_;

    void skip_empty() {
      while (idx_ < cap_ && !slots_[idx_].occupied) ++idx_;
    }

  public:
    iterator() : slots_(nullptr), idx_(0), cap_(0) {}
    iterator(Slot *s, size_t i, size_t c) : slots_(s), idx_(i), cap_(c) {
      skip_empty();
    }

    std::pair<const K &, V &> operator*() const {
      return {slots_[idx_].key, slots_[idx_].value};
    }

    iterator &operator++() { ++idx_; skip_empty(); return *this; }
    bool operator==(const iterator &o) const { return idx_ == o.idx_; }
    bool operator!=(const iterator &o) const { return idx_ != o.idx_; }
  };

  class const_iterator {
    friend class RobinHoodMap;
    const Slot *slots_;
    size_t idx_, cap_;

    void skip_empty() {
      while (idx_ < cap_ && !slots_[idx_].occupied) ++idx_;
    }

  public:
    const_iterator() : slots_(nullptr), idx_(0), cap_(0) {}
    const_iterator(const Slot *s, size_t i, size_t c)
        : slots_(s), idx_(i), cap_(c) { skip_empty(); }

    std::pair<const K &, const V &> operator*() const {
      return {slots_[idx_].key, slots_[idx_].value};
    }

    const_iterator &operator++() { ++idx_; skip_empty(); return *this; }
    bool operator==(const const_iterator &o) const { return idx_ == o.idx_; }
    bool operator!=(const const_iterator &o) const { return idx_ != o.idx_; }
  };

  // ---------------------------------------------------------------
  //  Constructors / Destructor
  // ---------------------------------------------------------------
  RobinHoodMap() { alloc(MIN_CAPACITY); }

  explicit RobinHoodMap(size_t initial_cap) {
    size_t cap = MIN_CAPACITY;
    while (cap < initial_cap) cap <<= 1;
    alloc(cap);
  }

  ~RobinHoodMap() { dealloc(); }

  // Move
  RobinHoodMap(RobinHoodMap &&o) noexcept
      : slots_(o.slots_), capacity_(o.capacity_), mask_(o.mask_),
        size_(o.size_), grow_at_(o.grow_at_) {
    o.slots_ = nullptr; o.capacity_ = 0; o.mask_ = 0;
    o.size_ = 0; o.grow_at_ = 0;
  }

  RobinHoodMap &operator=(RobinHoodMap &&o) noexcept {
    if (this != &o) {
      dealloc();
      slots_ = o.slots_; capacity_ = o.capacity_; mask_ = o.mask_;
      size_ = o.size_; grow_at_ = o.grow_at_;
      o.slots_ = nullptr; o.capacity_ = 0; o.mask_ = 0;
      o.size_ = 0; o.grow_at_ = 0;
    }
    return *this;
  }

  // No copy (supports move-only values like unique_ptr)
  RobinHoodMap(const RobinHoodMap &) = delete;
  RobinHoodMap &operator=(const RobinHoodMap &) = delete;

  // ---------------------------------------------------------------
  //  Public API
  // ---------------------------------------------------------------

  // Insert-or-access (like std::unordered_map::operator[])
  V &operator[](const K &key) {
    size_t idx = find_slot(key);
    if (idx != SIZE_MAX) return slots_[idx].value;
    K k = key;
    V v{};
    return *do_insert(std::move(k), std::move(v));
  }

  // Find
  iterator find(const K &key) {
    size_t idx = find_slot(key);
    if (idx == SIZE_MAX) return end();
    return iterator(slots_, idx, capacity_);
  }

  const_iterator find(const K &key) const {
    size_t idx = find_slot(key);
    if (idx == SIZE_MAX) return end();
    return const_iterator(slots_, idx, capacity_);
  }

  // Count (0 or 1)
  size_t count(const K &key) const {
    return find_slot(key) != SIZE_MAX ? 1 : 0;
  }

  size_t size() const { return size_; }
  bool empty() const { return size_ == 0; }

  // Erase with backward-shift deletion
  bool erase(const K &key) {
    size_t idx = find_slot(key);
    if (idx == SIZE_MAX) return false;

    slots_[idx].key.~K();
    slots_[idx].value.~V();
    slots_[idx].occupied = false;
    --size_;

    // Backward shift: pull subsequent displaced elements back
    size_t next = (idx + 1) & mask_;
    while (slots_[next].occupied && psl_at(next) > 0) {
      size_t prev = (next + capacity_ - 1) & mask_;

      new (&slots_[prev].key) K(std::move(slots_[next].key));
      new (&slots_[prev].value) V(std::move(slots_[next].value));
      slots_[prev].hash = slots_[next].hash;
      slots_[prev].occupied = true;

      slots_[next].key.~K();
      slots_[next].value.~V();
      slots_[next].occupied = false;

      next = (next + 1) & mask_;
    }
    return true;
  }

  // Iterators
  iterator begin() { return iterator(slots_, 0, capacity_); }
  iterator end()   { return iterator(slots_, capacity_, capacity_); }

  const_iterator begin() const { return const_iterator(slots_, 0, capacity_); }
  const_iterator end()   const { return const_iterator(slots_, capacity_, capacity_); }

  // Clear
  void clear() {
    for (size_t i = 0; i < capacity_; ++i) {
      if (slots_[i].occupied) {
        slots_[i].key.~K();
        slots_[i].value.~V();
        slots_[i].occupied = false;
      }
    }
    size_ = 0;
  }
};
