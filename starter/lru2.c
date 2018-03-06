#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

int top;
int bottom;
/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int lru_evict() {
	int frame = bottom;
	bottom = coremap[frame].prev;
	coremap[bottom].next = -1;
	
	coremap[top].prev = frame;
	coremap[frame].prev = -1;
	coremap[frame].next = top;
	top = frame;

	return frame;
}

 /* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
  */
 
void lru_ref(pgtbl_entry_t *p) {
	int frame = p->frame >> PAGE_SHIFT;
	
	int prev = coremap[frame].prev;
	int next = coremap[frame].next;

	if (prev == -1 && next == -1) {
		return;
	}
	if (prev == -1) { 
		top = next;
		coremap[top].prev = -1;
	} else if (next == -1) { 
		bottom = prev;
		coremap[bottom].next = -1;
	} else {
		coremap[prev].next = next;
		coremap[next].prev = prev; 
	}

	coremap[top].prev = frame;
	coremap[frame].prev = -1;
	coremap[frame].next = top;
	top = frame;
	return;
}


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
	coremap[0].prev = -1;
	coremap[0].next = 1;
	int i;
	for (i = 1; i < memsize; i++) {
		coremap[i].prev = i - 1;
		coremap[i].next = i + 1;
	}
	coremap[memsize-1].next = -1;

	top = 0;
	bottom = memsize - 1;
}
