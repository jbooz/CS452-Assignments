#include <stdlib.h>
#include <string.h>

#include "bm.h"
#include "utils.h"

/*
 * Returns the number of bits managed by bitmap b.
 */
static size_t bmbits(BM b) { size_t *bits=b; return *--bits; }

/*
 * Returns the total byte size required for bitmap b.
 */
static size_t bmbytes(BM b) { return bits2bytes(bmbits(b)); }

/*
 * Validates that bitmap index i is within range [0, bmbits(b) - 1].
 */
static void ok(BM b, size_t i) {
  if (i<bmbits(b))
    return;
  fprintf(stderr,"bitmap index out of range\n");
  exit(1);
}         

/*
 * Instantiates a general-purpose bitmap holding 'bits' bits.
 */
extern BM bmcreate(size_t bits) {
  size_t bytes=bits2bytes(bits);
  size_t *p=mmalloc(sizeof(size_t)+bytes);
  if ((long)p==-1)
    return 0;
  *p=bits;
  BM b=++p;
  memset(b,0,bytes);
  return b;
}

/*
 * Destroys bitmap b and releases memory via mmfree.
 */
extern void bmdelete(BM b) {
  size_t *p=b;
  p--;
  mmfree(p,sizeof(size_t)+bits2bytes(*p));
}

/*
 * Sets bit i in bitmap b to 1.
 */
extern void bmset(BM b, size_t i) {
  ok(b,i); bitset(b+i/bitsperbyte,i%bitsperbyte);
}

/*
 * Clears bit i in bitmap b to 0.
 */
extern void bmclr(BM b, size_t i) {
  ok(b,i); bitclr(b+i/bitsperbyte,i%bitsperbyte);
}

/*
 * Tests whether bit i in bitmap b is set (1) or cleared (0).
 */
extern int bmtst(BM b, size_t i) {
  ok(b,i); return bittst(b+i/bitsperbyte,i%bitsperbyte);
}

/*
 * Prints hex output of bitmap bytes to stdout.
 */
extern void bmprt(BM b) {
  for (int byte=bmbytes(b)-1; byte>=0; byte--)
    printf("%02x%s",((char *)b)[byte],(byte ? " " : "\n"));
}
