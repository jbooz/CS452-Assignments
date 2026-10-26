#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

/*
 * Allocates a region of virtual memory using the mmap system call.
 * Requests 'size' bytes of anonymous, private, read/write memory directly from
 * the kernel via mmap().
 */
extern void *mmalloc(size_t size) {
  void *p = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  return p;
}

/*
 * Releases a region of memory back to the operating system via munmap.
 * Unmaps 'size' bytes starting at address 'p' that were previously obtained with mmalloc().
 */
extern void mmfree(void *p, size_t size) {
  if (p && p != (void *)-1) {
    munmap(p, size);
  }
}

/*
 * Performs integer division rounded up (ceiling division).
 * Computes ceil(n / d) using integer arithmetic. Returns 0 if n or d is 0.
 */
extern size_t divup(size_t n, size_t d) {
  if (d == 0) return 0;
  return (n == 0) ? 0 : (n + d - 1) / d;
}

/*
 * Converts a bit count into the minimum number of bytes needed.
 * Divides the bit count by bitsperbyte (8), rounding up to cover all bits.
 */
extern size_t bits2bytes(size_t bits) {
  return divup(bits, bitsperbyte);
}

/*
 * Converts a power-of-two exponent e into its byte size (2^e).
 * Computes 1 << e using bitwise left shift.
 */
extern size_t e2size(int e) {
  return ((size_t)1) << e;
}

/*
 * Calculates the smallest exponent e such that 2^e >= size.
 * Iteratively shifts until 1 << e is at least equal to 'size'.
 */
extern int size2e(size_t size) {
  int e = 0;
  while (e2size(e) < size) {
    e++;
  }
  return e;
}

/*
 * Sets a specific bit to 1 within a byte array.
 * Finds the target byte offset (bit / 8) and bit mask (1 << (bit % 8)), applying bitwise OR.
 */
extern void bitset(void *p, int bit) {
  unsigned char *byte = (unsigned char *)p + (bit / bitsperbyte);
  *byte |= (1 << (bit % bitsperbyte));
}

/*
 * Clears a specific bit (sets to 0) within a byte array.
 * Finds the target byte offset and clears the corresponding bit using bitwise AND NOT.
 */
extern void bitclr(void *p, int bit) {
  unsigned char *byte = (unsigned char *)p + (bit / bitsperbyte);
  *byte &= ~(1 << (bit % bitsperbyte));
}

/*
 * Inverts (flips) a specific bit within a byte array.
 * Toggles the target bit using bitwise XOR.
 */
extern void bitinv(void *p, int bit) {
  unsigned char *byte = (unsigned char *)p + (bit / bitsperbyte);
  *byte ^= (1 << (bit % bitsperbyte));
}

/*
 * Tests whether a specific bit is set (1) or cleared (0).
 * Returns non-zero if the target bit is set, or 0 if cleared.
 */
extern int bittst(void *p, int bit) {
  unsigned char *byte = (unsigned char *)p + (bit / bitsperbyte);
  return (*byte & (1 << (bit % bitsperbyte))) != 0;
}
