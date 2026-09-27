#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deq.h"
#include "error.h"

typedef enum {Head,Tail,Ends} End;

typedef struct Node {
  struct Node *np[Ends];
  Data data;
} *Node;

typedef struct {
  Node ht[Ends];
  int len;
} *Rep;

/*
 * Validates queue handle q and returns representation Rep.
 */
static Rep rep(Deq q) {
  if (!q) ERROR("zero pointer");
  return (Rep)q;
}

/*
 * Creates a new Node and inserts it at specified end e.
 */
static void put(Rep r, End e, Data d) {
  Node n = (Node)malloc(sizeof(*n));
  if (!n) {
    ERROR("malloc() failed");
  }
  n->data = d;

  End o = 1 - e;
  n->np[e] = NULL;
  n->np[o] = r->ht[e];

  if (r->ht[e]) {
    r->ht[e]->np[e] = n;
  }
  else {
    r->ht[o] = n;
  }
  r->ht[e] = n;
  r->len++;
}

/*
 * Retrieves data from Node at index i from end e.
 */
static Data ith(Rep r, End e, int i) {
  if (i < 0 || i >= r->len) {
    ERROR("index out of range");
  }
  End o = 1 - e;
  Node n = r->ht[e];

  for (int k = 0; k < i; k++) {
    n = n->np[o];
  }
  return n->data;
}

/*
 * Removes and returns data from the Node at end e.
 */
static Data get(Rep r, End e) {
  if (r->len == 0) {
    ERROR("empty queue");
  }
  Node n = r->ht[e];
  Data d = n->data;
  End o = 1 - e;

  r->ht[e] = n->np[o];
  if (r->ht[e]) {
    r->ht[e]->np[e] = NULL;
  }
  else {
    r->ht[o] = NULL;
  }

  free(n);
  r->len--;
  return d;
}

/*
 * Removes first Node matching data d starting from end e.
 */
static Data rem(Rep r, End e, Data d) {
  End o = 1 - e;
  for (Node n = r->ht[e]; n; n = n->np[o]) {
    if (n->data == d) {
      Node prev = n->np[e];
      Node next = n->np[o];

      if (prev) {
        prev->np[o] = next;
      }
      else {     
        r->ht[e] = next;
      }
      if (next) {
        next->np[e] = prev;
      }
      else {
        r->ht[o] = prev;
      }

      free(n);
      r->len--;
      return d;
    }
  }
  return NULL;
}

/*
 * Creates a new empty double-ended queue.
 */
extern Deq deq_new() {
  Rep r=(Rep)malloc(sizeof(*r));
  if (!r) ERROR("malloc() failed");
  r->ht[Head]=0;
  r->ht[Tail]=0;
  r->len=0;
  return r;
}

/*
 * Returns element count in queue q.
 */
extern int deq_len(Deq q) { return rep(q)->len; }

/*
 * Pushes element d onto head of queue.
 */
extern void deq_head_put(Deq q, Data d) {        put(rep(q),Head,d); }

/*
 * Pops element from head of queue.
 */
extern Data deq_head_get(Deq q)         { return get(rep(q),Head);   }

/*
 * Inspects element at index i from head.
 */
extern Data deq_head_ith(Deq q, int i)  { return ith(rep(q),Head,i); }

/*
 * Removes first element matching d starting from head.
 */
extern Data deq_head_rem(Deq q, Data d) { return rem(rep(q),Head,d); }

/*
 * Pushes element d onto tail of queue.
 */
extern void deq_tail_put(Deq q, Data d) {        put(rep(q),Tail,d); }

/*
 * Pops element from tail of queue.
 */
extern Data deq_tail_get(Deq q)         { return get(rep(q),Tail);   }

/*
 * Inspects element at index i from tail.
 */
extern Data deq_tail_ith(Deq q, int i)  { return ith(rep(q),Tail,i); }

/*
 * Removes first element matching d starting from tail.
 */
extern Data deq_tail_rem(Deq q, Data d) { return rem(rep(q),Tail,d); }

/*
 * Applies function f to every data element in queue.
 */
extern void deq_map(Deq q, DeqMapF f) {
  for (Node n=rep(q)->ht[Head]; n; n=n->np[Tail])
    f(n->data);
}

/*
 * Destroys queue q and cleans up nodes.
 */
extern void deq_del(Deq q, DeqMapF f) {
  if (f) deq_map(q,f);
  Node curr=rep(q)->ht[Head];
  while (curr) {
    Node next=curr->np[Tail];
    free(curr);
    curr=next;
  }
  free(q);
}

/*
 * Generates space-separated string representation of queue.
 */
extern Str deq_str(Deq q, DeqStrF f) {
  char *s=strdup("");
  for (Node n=rep(q)->ht[Head]; n; n=n->np[Tail]) {
    char *d=f ? f(n->data) : n->data;
    char *t; asprintf(&t,"%s%s%s",s,(*s ? " " : ""),d);
    free(s); s=t;
    if (f) free(d);
  }
  return s;
}
