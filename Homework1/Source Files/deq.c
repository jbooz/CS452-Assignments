#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deq.h"
#include "error.h"

// indices and size of array of node pointers
typedef enum {Head,Tail,Ends} End;

typedef struct Node {
  struct Node *np[Ends];        // next/prev neighbors
  Data data;
} *Node;

typedef struct {
  Node ht[Ends];                // head/tail nodes
  int len;
} *Rep;

static Rep rep(Deq q) {
  if (!q) ERROR("zero pointer");
  return (Rep)q;
}

/* This method creates a new Node, then appends that Node to one
end of the queue. This increases the queue's size by 1. */
static void put(Rep r, End e, Data d) {
  // Create a Node
  Node n = (Node)malloc(sizeof(*n));
  if (!n) {
    ERROR("malloc() failed");
  }
  n->data = d;

  // Opposite End
  End o = 1 - e;
  n->np[e] = NULL;
  n->np[o] = r->ht[e];

  // Adds to one end, updates len
  if (r->ht[e]) {
    r->ht[e]->np[e] = n;
  }
  else {
    r->ht[o] = n;
  }
  r->ht[e] = n;
  r->len++;
}

/* This method returns data from the Node that is at the specified
index. This does not change the size of the queue. */
static Data ith(Rep r, End e, int i) {
  // Determine if index is in range, then begin traversal
  if (i < 0 || i >= r->len) {
    ERROR("index out of range");
  }
  End o = 1 - e;
  Node n = r->ht[e];

  // Find the node, then return its data
  for (int k = 0; k < i; k++) {
    n = n->np[o];
  }
  return n->data;
}

/* This method returns the data from a node at one end of
the queue. This decreases the queue's size by 1. */
static Data get(Rep r, End e) {
  // Check if empty, then get Node from one end
  if (r->len == 0) {
    ERROR("empty queue");
  }
  Node n = r->ht[e];
  Data d = n->data;
  End o = 1 - e;

  // Change previous/next nodes to null to "detach"
  // the node from the rest of the queue
  r->ht[e] = n->np[o];
  if (r->ht[e]) {
    r->ht[e]->np[e] = NULL;
  }
  else {
    r->ht[o] = NULL;
  }

  // Remove it, decrement len, and return the data
  free(n);
  r->len--;
  return d;
}

/* This method traverses the queue starting from one end
until a Node with matching data is found, and that Node is then removed
from the queue. This decreases the queue's size by 1. */
static Data rem(Rep r, End e, Data d) {
  End o = 1 - e;
  // Traverse queue until node is found, then remove it
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

extern Deq deq_new() {
  Rep r=(Rep)malloc(sizeof(*r));
  if (!r) ERROR("malloc() failed");
  r->ht[Head]=0;
  r->ht[Tail]=0;
  r->len=0;
  return r;
}

extern int deq_len(Deq q) { return rep(q)->len; }

extern void deq_head_put(Deq q, Data d) {        put(rep(q),Head,d); }
extern Data deq_head_get(Deq q)         { return get(rep(q),Head);   }
extern Data deq_head_ith(Deq q, int i)  { return ith(rep(q),Head,i); }
extern Data deq_head_rem(Deq q, Data d) { return rem(rep(q),Head,d); }

extern void deq_tail_put(Deq q, Data d) {        put(rep(q),Tail,d); }
extern Data deq_tail_get(Deq q)         { return get(rep(q),Tail);   }
extern Data deq_tail_ith(Deq q, int i)  { return ith(rep(q),Tail,i); }
extern Data deq_tail_rem(Deq q, Data d) { return rem(rep(q),Tail,d); }

extern void deq_map(Deq q, DeqMapF f) {
  for (Node n=rep(q)->ht[Head]; n; n=n->np[Tail])
    f(n->data);
}

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
