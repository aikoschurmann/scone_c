#include "core/arena.h"
#include "core/error.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ARENA_DEFAULT_SIZE    (64  * 1024)
#define ARENA_MAX_BLOCK_SIZE  (256 * 1024 * 1024)

static inline size_t align_up_sz(size_t v, size_t a) {
    return (v + a - 1) & ~(a - 1);
}

// ---------------------------------------------------------------
// Internal: allocate and wire up a new block
// ---------------------------------------------------------------

static ArenaBlock *block_create(size_t min_capacity) {
    size_t cap = align_up_sz(min_capacity, 4096); // round to OS page size
    ArenaBlock *b = malloc(sizeof(ArenaBlock) + cap);
    if (!b) return NULL;
    b->next     = NULL;
    b->capacity = cap;
    b->used     = 0;
    return b;
}

static void arena_install_block(Arena *a, ArenaBlock *b) {
    b->next  = a->head;
    a->head  = b;
    a->cursor = b->data;
    a->limit  = b->data + b->capacity;
}

// ---------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------

Arena *arena_create(size_t initial_capacity) {
    if (initial_capacity == 0) initial_capacity = ARENA_DEFAULT_SIZE;

    Arena *a = malloc(sizeof(Arena));
    if (!a) return NULL;

    ArenaBlock *b = block_create(initial_capacity);
    if (!b) { free(a); return NULL; }

    b->next           = NULL;
    a->head           = b;
    a->cursor         = b->data;
    a->limit          = b->data + b->capacity;
    a->block_size     = b->capacity;
    a->block_size_max = ARENA_MAX_BLOCK_SIZE;
    a->total_allocd   = 0;
    return a;
}

void arena_destroy(Arena *a) {
    if (!a) return;
    ArenaBlock *b = a->head;
    while (b) {
        ArenaBlock *next = b->next;
        free(b);
        b = next;
    }
    free(a);
}

void arena_reset(Arena *a) {
    if (!a) return;
    // Walk to the oldest (last) block, free all newer blocks.
    ArenaBlock *b = a->head;
    while (b->next) {
        ArenaBlock *next = b->next;
        free(b);
        b = next;
    }
    b->used         = 0;
    b->next         = NULL;
    a->head         = b;
    a->cursor       = b->data;
    a->limit        = b->data + b->capacity;
    a->block_size   = b->capacity;
    a->total_allocd = 0;
}

// ---------------------------------------------------------------
// Slow path: grow and allocate
// ---------------------------------------------------------------

void *arena_alloc_slow(Arena *a, size_t size, size_t align) {
    // Sync the used counter of the current head block before growing.
    a->head->used = (size_t)(a->cursor - a->head->data);

    size_t new_size = a->block_size * 2;
    if (new_size > a->block_size_max) new_size = a->block_size_max;
    if (new_size < size + align)      new_size = size + align;

    ArenaBlock *nb = block_create(new_size);
    if (!nb) {
        LOG_ERR("arena: OOM — failed to allocate block of %zu bytes", new_size);
        return NULL;
    }
    a->block_size = nb->capacity;
    arena_install_block(a, nb);

    // Now the fast path will succeed on retry. Inline it here to avoid
    // an extra call frame.
    uintptr_t cur     = (uintptr_t)a->cursor;
    uintptr_t aligned = (cur + align - 1) & ~(uintptr_t)(align - 1);
    char     *next    = (char *)aligned + size;
    a->cursor         = next;
    a->total_allocd  += size;
    return (void *)aligned;
}

// ---------------------------------------------------------------
// Checkpoint / Restore
// ---------------------------------------------------------------

void arena_restore(Arena *a, ArenaCheckpoint cp) {
    // Free any blocks allocated after the checkpoint.
    while (a->head != cp.block) {
        ArenaBlock *old = a->head;
        a->head = old->next;
        free(old);
    }
    // Restore cursor within the checkpoint block.
    a->head->used = cp.used;
    a->cursor     = a->head->data + cp.used;
    a->limit      = a->head->data + a->head->capacity;
}

// ---------------------------------------------------------------
// Diagnostics
// ---------------------------------------------------------------

size_t arena_bytes_used(const Arena *a) {
    if (!a) return 0;
    size_t total = (size_t)(a->cursor - a->head->data); // current block
    for (const ArenaBlock *b = a->head->next; b; b = b->next)
        total += b->used;
    return total;
}

size_t arena_bytes_capacity(const Arena *a) {
    if (!a) return 0;
    size_t total = 0;
    for (const ArenaBlock *b = a->head; b; b = b->next)
        total += b->capacity;
    return total;
}

size_t arena_block_count(const Arena *a) {
    if (!a) return 0;
    size_t n = 0;
    for (const ArenaBlock *b = a->head; b; b = b->next) n++;
    return n;
}

void arena_print_stats(const Arena *a) {
    if (!a) { printf("[arena] (null)\n"); return; }
    size_t used     = arena_bytes_used(a);
    size_t capacity = arena_bytes_capacity(a);
    double fill     = capacity > 0 ? 100.0 * (double)used / (double)capacity : 0.0;
    printf("[arena] blocks=%zu  used=%.1f KB  capacity=%.1f KB  fill=%.1f%%\n",
           arena_block_count(a), used / 1024.0, capacity / 1024.0, fill);
}
