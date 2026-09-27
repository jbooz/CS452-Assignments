#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "freelist.h"
#include "bbm.h"
#include "bm.h"
#include "utils.h"

typedef struct FreeListRep {
  void **head;
  BBM *map;
  BM *alloc_map;
} *FreeListRep;

/*
 * Removes a specific block pointer from a singly-linked free list.
 * Traverses list at *head and unlinks node target if found.
 */
static void remove_from_freelist(void **head, void *target) {
  void *curr = *head;
  void *prev = NULL;
  while (curr) {
    if (curr == target) {
      if (prev) {
        *(void **)prev = *(void **)curr;
      } else {
        *head = *(void **)curr;
      }
      break;
    }
    prev = curr;
    curr = *(void **)curr;
  }
}

/*
 * Instantiates and initializes a multi-level free list management structure.
 * Allocates metadata arrays for list heads, buddy bitmaps (BBM), and allocation maps (BM)
 * across order levels l through u.
 */
extern FreeList freelistcreate(size_t size, int l, int u) {
  FreeListRep f = mmalloc(sizeof(*f));
  if ((long)f == -1 || !f) return NULL;

  f->head = mmalloc(sizeof(void *) * (u + 1));
  f->map = mmalloc(sizeof(BBM) * (u + 1));
  f->alloc_map = mmalloc(sizeof(BM) * (u + 1));

  if ((long)f->head == -1 || (long)f->map == -1 || (long)f->alloc_map == -1) {
    return NULL;
  }

  for (int e = 0; e <= u; e++) {
    f->head[e] = NULL;
    if (e >= l) {
      f->map[e] = bbmcreate(size, e);
      size_t blocks = divup(size, e2size(e));
      f->alloc_map[e] = bmcreate(blocks);
    } else {
      f->map[e] = NULL;
      f->alloc_map[e] = NULL;
    }
  }

  return (FreeList)f;
}

/*
 * Destroys a free list manager and releases metadata memory.
 * Frees all buddy bitmaps, allocation tracking maps, and structural arrays.
 */
extern void freelistdelete(FreeList f, int l, int u) {
  if (!f) return;
  FreeListRep r = (FreeListRep)f;

  for (int e = l; e <= u; e++) {
    if (r->map[e]) bbmdelete(r->map[e]);
    if (r->alloc_map[e]) bmdelete(r->alloc_map[e]);
  }

  mmfree(r->head, sizeof(void *) * (u + 1));
  mmfree(r->map, sizeof(BBM) * (u + 1));
  mmfree(r->alloc_map, sizeof(BM) * (u + 1));
  mmfree(r, sizeof(*r));
}

/*
 * Allocates a memory block of order e from the free lists.
 * Finds smallest available block order k >= e, iteratively splits higher-order blocks down
 * to order e (placing remaining buddies onto free lists), and updates buddy/allocation bitmaps.
 */
extern void *freelistalloc(FreeList f, void *base, int e, int l) {
  if (!f) return NULL;
  FreeListRep r = (FreeListRep)f;

  int k = e;
  while (r->map[k] != NULL) {
    if (r->head[k] != NULL) break;
    k++;
  }

  if (r->map[k] == NULL || r->head[k] == NULL) return NULL;
  void *p = r->head[k];
  r->head[k] = *(void **)p;
  bbmclr(r->map[k], base, p, k);

  while (k > e) {
    k--;
    void *buddy = baddrinv(base, p, k);
    *(void **)buddy = r->head[k];
    r->head[k] = buddy;
    bbmset(r->map[k], base, buddy, k);
  }

  size_t idx = ((char *)p - (char *)base) / e2size(e);
  if (r->alloc_map[e]) {
    bmset(r->alloc_map[e], idx);
  }

  return p;
}

/*
 * Deallocates a block and merges it with available buddy blocks.
 * Clears the allocation map entry, tests if the block's buddy is free via the buddy bitmap,
 * unlinks free buddies, and recursively merges blocks up to maximum order u.
 */
extern void freelistfree(FreeList f, void *base, void *mem, int e, int l) {
  if (!f || !mem) return;
  FreeListRep r = (FreeListRep)f;

  size_t idx = ((char *)mem - (char *)base) / e2size(e);
  if (r->alloc_map[e]) {
    bmclr(r->alloc_map[e], idx);
  }

  while (r->map[e] != NULL) {
    if (bbmtst(r->map[e], base, mem, e)) {
      bbmclr(r->map[e], base, mem, e);
      void *buddy = baddrinv(base, mem, e);
      remove_from_freelist(&(r->head[e]), buddy);
      mem = baddrclr(base, mem, e);
      e++;
      if (r->map[e] == NULL) {
        e--;
        bbmset(r->map[e], base, mem, e);
        *(void **)mem = r->head[e];
        r->head[e] = mem;
        return;
      }
    } else {
      bbmset(r->map[e], base, mem, e);
      *(void **)mem = r->head[e];
      r->head[e] = mem;
      return;
    }
  }
}

/*
 * Returns the byte size of an allocated memory block.
 * Scans allocation tracking maps for orders l through u to find the order level where
 * 'mem' is registered as allocated.
 */
extern int freelistsize(FreeList f, void *base, void *mem, int l, int u) {
  if (!f || !mem) return 0;
  FreeListRep r = (FreeListRep)f;

  for (int e = l; e <= u; e++) {
    size_t blk_size = e2size(e);
    size_t offset = (char *)mem - (char *)base;
    if (offset % blk_size == 0) {
      size_t idx = offset / blk_size;
      if (r->alloc_map[e] && bmtst(r->alloc_map[e], idx)) {
        return (int)blk_size;
      }
    }
  }
  return 0;
}

/*
 * Prints formatted view of free list headers and bitmaps to stdout.
 * Displays order levels, free block pointer chains, and bitmap states.
 */
extern void freelistprint(FreeList f, int l, int u) {
  if (!f) return;
  FreeListRep r = (FreeListRep)f;
  printf("FreeList Status:\n");
  for (int e = u; e >= l; e--) {
    printf("  Order %2d (size %zu bytes): ", e, e2size(e));
    void *curr = r->head[e];
    int count = 0;
    while (curr) {
      printf("[%p] -> ", curr);
      curr = *(void **)curr;
      count++;
    }
    printf("NULL (%d free blocks)\n", count);
    if (r->map[e]) {
      printf("    Buddy Bitmap: ");
      bbmprt(r->map[e]);
    }
  }
}
