#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "balloc.h"
#include "freelist.h"
#include "utils.h"

typedef struct BallocRep {
  void *base;
  unsigned int size;
  int l;
  int u;
  FreeList freelist;
} *BallocRep;

/*
 * Constructs and initializes a Buddy System memory allocator instance.
 * Obtains 'size' bytes of contiguous system virtual memory using mmalloc() (mmap),
 * creates the FreeList tracking metadata, and populates initial free blocks.
 */
extern Balloc bcreate(unsigned int size, int l, int u) {
  if (size == 0 || l < 0 || u < l) return NULL;

  void *base = mmalloc(size);
  if ((long)base == -1 || !base) return NULL;

  FreeList fl = freelistcreate(size, l, u);
  if (!fl) {
    mmfree(base, size);
    return NULL;
  }

  BallocRep ba = mmalloc(sizeof(*ba));
  if ((long)ba == -1 || !ba) {
    freelistdelete(fl, l, u);
    mmfree(base, size);
    return NULL;
  }

  ba->base = base;
  ba->size = size;
  ba->l = l;
  ba->u = u;
  ba->freelist = fl;

  size_t rem = size;
  char *ptr = (char *)base;
  while (rem >= e2size(l)) {
    int e = u;
    while (e >= l) {
      size_t blk_size = e2size(e);
      if (blk_size <= rem && (((uintptr_t)(ptr - (char *)base)) % blk_size == 0)) {
        break;
      }
      e--;
    }
    if (e < l) break;

    freelistfree(ba->freelist, ba->base, ptr, e, l);

    ptr += e2size(e);
    rem -= e2size(e);
  }

  return (Balloc)ba;
}

/*
 * Destroys an allocator instance and releases all associated virtual memory.
 * Deallocates free lists, metadata, the main memory region, and the BallocRep handle.
 */
extern void bdelete(Balloc ba) {
  if (!ba) return;
  BallocRep r = (BallocRep)ba;
  freelistdelete(r->freelist, r->l, r->u);
  mmfree(r->base, r->size);
  mmfree(r, sizeof(*r));
}

/*
 * Requests a contiguous block of memory of at least 'size' bytes.
 * Determines order level e for 'size' (bounded by l and u) and retrieves a block via freelistalloc().
 */
extern void *balloc(Balloc ba, unsigned int size) {
  if (!ba || size == 0) return NULL;
  BallocRep r = (BallocRep)ba;

  int e = size2e(size);
  if (e < r->l) e = r->l;
  if (e > r->u) return NULL;

  return freelistalloc(r->freelist, r->base, e, r->l);
}

/*
 * Deallocates a previously allocated block of memory at address mem.
 * Finds block order via freelistsize() and returns memory to free lists via freelistfree().
 */
extern void bfree(Balloc ba, void *mem) {
  if (!ba || !mem) return;
  BallocRep r = (BallocRep)ba;

  int sz = freelistsize(r->freelist, r->base, mem, r->l, r->u);
  if (sz <= 0) return;

  int e = size2e(sz);
  freelistfree(r->freelist, r->base, mem, e, r->l);
}

/*
 * Returns the allocation size in bytes of the block at mem.
 * Queries freelistsize() for the exact order block size allocated.
 */
extern unsigned int bsize(Balloc ba, void *mem) {
  if (!ba || !mem) return 0;
  BallocRep r = (BallocRep)ba;
  return freelistsize(r->freelist, r->base, mem, r->l, r->u);
}

/*
 * Writes a textual summary of the allocator instance to stdout.
 * Prints configuration parameters and detailed free list states.
 */
extern void bprint(Balloc ba) {
  if (!ba) return;
  BallocRep r = (BallocRep)ba;
  printf("Balloc Allocator @ %p\n", (void *)r);
  printf("  Base Address: %p\n", r->base);
  printf("  Total Size:   %u bytes\n", r->size);
  printf("  Min Order l:  %d (%zu bytes)\n", r->l, e2size(r->l));
  printf("  Max Order u:  %d (%zu bytes)\n", r->u, e2size(r->u));
  freelistprint(r->freelist, r->l, r->u);
}
