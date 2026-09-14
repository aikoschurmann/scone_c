#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// ============================================================
// Arena Allocator
//
// A bump-pointer allocator designed for high-throughput allocation
// of many small objects (e.g. 16-byte ExprNodes) with no individual
// frees. Grows via a linked list of doubling-size blocks.
//
// Usage:
//   Arena *a = arena_create(64 * 1024 * 1024);
//   ExprNode *n = ARENA_ALLOC(a, ExprNode);
//   arena_destroy(a);
// ============================================================

typedef struct ArenaBlock {
    struct ArenaBlock *next;
    size_t capacity;
    size_t used;
    char   data[];
} ArenaBlock;

typedef struct {
    // Hot fields: laid out first so a single ldp loads both in one instruction.
    char *cursor;       // next free byte in the current block
    char *limit;        // one past the last usable byte in the current block

    // Cold fields: only touched during block growth or diagnostics.
    ArenaBlock *head;
    size_t      block_size;
    size_t      block_size_max;
    size_t      total_allocd;
} Arena;

typedef struct {
    ArenaBlock *block;
    size_t      used;
} ArenaCheckpoint;

// ---------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------

Arena *arena_create(size_t initial_capacity);
void   arena_destroy(Arena *a);
void   arena_reset(Arena *a);

// ---------------------------------------------------------------
// Slow path — out-of-line block growth. Do not call directly.
// ---------------------------------------------------------------
void *arena_alloc_slow(Arena *a, size_t size, size_t align);

// ---------------------------------------------------------------
// HOT PATH — inlined bump-pointer.
// Exact ARM64 output (clang -O3, ExprNode: size=16 align=4):
//
//   ldp  x8, x9, [x19]              ; x8=cursor, x9=limit (one 128-bit load)
//   add  x8, x8, #3                 ; cur + (align-1)
//   and  x8, x8, #0xfffffffffffffffc ; aligned = mask off low 2 bits
//   add  x8, x8, #16                ; next = aligned + size
//   cmp  x8, x9                     ; next vs limit
//   b.hi <slow>                     ; slow path if next > limit (rare)
//   str  x8, [x19]                  ; cursor = next
// ---------------------------------------------------------------
static inline void *arena_alloc(Arena *a, size_t size, size_t align) {
    uintptr_t cur     = (uintptr_t)a->cursor;
    uintptr_t aligned = (cur + align - 1) & ~(uintptr_t)(align - 1);
    char     *next    = (char *)aligned + size;

    if (__builtin_expect(next <= a->limit, 1)) {
        a->cursor = next;
        return (void *)aligned;
    }

    return arena_alloc_slow(a, size, align);
}

static inline void *arena_calloc(Arena *a, size_t size, size_t align) {
    void *p = arena_alloc(a, size, align);
    if (p) memset(p, 0, size);
    return p;
}

// Typed allocation macros — no sizeof/alignof boilerplate at call sites.
#define ARENA_ALLOC(a, T)        ((T *)arena_alloc ((a), sizeof(T),      _Alignof(T)))
#define ARENA_CALLOC(a, T)       ((T *)arena_calloc((a), sizeof(T),      _Alignof(T)))
#define ARENA_ALLOC_N(a, T, n)   ((T *)arena_alloc ((a), sizeof(T)*(n),  _Alignof(T)))
#define ARENA_CALLOC_N(a, T, n)  ((T *)arena_calloc((a), sizeof(T)*(n),  _Alignof(T)))

// ---------------------------------------------------------------
// Checkpoint / Scratch region
// ---------------------------------------------------------------

static inline ArenaCheckpoint arena_checkpoint(const Arena *a) {
    return (ArenaCheckpoint){
        .block = a->head,
        .used  = (size_t)(a->cursor - a->head->data)
    };
}

// Roll back all allocations made after cp was taken. O(N dropped blocks).
void arena_restore(Arena *a, ArenaCheckpoint cp);

// ---------------------------------------------------------------
// Diagnostics
// ---------------------------------------------------------------

size_t arena_bytes_used(const Arena *a);
size_t arena_bytes_capacity(const Arena *a);
size_t arena_block_count(const Arena *a);
void   arena_print_stats(const Arena *a);
