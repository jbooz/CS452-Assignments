#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "deq.h"

/* The main function of this program is intended to run through the test suite
that ensures functionality of every aspect of the queue. In this case, main tests
put, len, ith, rem, and get to determine if they work as intended or not. The
assert() function tests that the expected value is what results when running the
program. */
int main() {
  Deq q = deq_new();
  
  // Test put & len
  deq_head_put(q, "B");
  deq_head_put(q, "A");
  deq_tail_put(q, "C");
  assert(deq_len(q) == 3);

  // Test ith
  assert(deq_head_ith(q, 0) == (Data)"A");
  assert(deq_tail_ith(q, 0) == (Data)"C");

  // Test rem
  Data r = deq_head_rem(q, "B");
  assert(r == (Data)"B");
  assert(deq_len(q) == 2);

  // Test get
  assert(deq_head_get(q) == (Data)"A");
  assert(deq_tail_get(q) == (Data)"C");
  assert(deq_len(q) == 0);

  // Clean up
  deq_del(q, 0);
  printf("All assertion tests passed!\n");
  return 0;
}
