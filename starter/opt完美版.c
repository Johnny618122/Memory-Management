#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"
#include "sim.h"

extern int debug;

extern struct frame *coremap;

node_t *curr_trace;
node_t *head;
node_t *tail;
/* Page to evict is chosen using the optimal (aka MIN) algorithm. 
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {

	int victim = 0;
	int max = 0;
	int i;
	node_t *curr;

	for (i = 0; i < memsize; i++) {
		int distance = 0;
		curr = curr_trace->next;

		while (curr && curr->vaddr != coremap[i].vaddr) {
			distance += 1;
			curr = curr->next;
		}

		if (curr == NULL) {
			return i;
		} else if (distance > max) {
			victim = i;
			max = distance;
		}
	}

	return victim;
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {
	if (curr_trace->next) {
		curr_trace = curr_trace->next;
	} else {
		curr_trace = head; 
	}
	return;
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
	char buf[MAXLINE];
	char type;
	FILE* tfp;
	if((tfp = fopen(tracefile, "r")) == NULL) {
		perror("Error opening tracefile:");
		exit(1);
	}

	head = NULL; curr_trace = NULL; tail = NULL;
	addr_t vaddr = 0;
	while( fgets(buf, MAXLINE, tfp) != NULL) {  
		if (buf[0] != '=') {
			sscanf(buf, "%c %lx", &type, &vaddr);
			node_t *trace = (node_t*) malloc(sizeof(node_t));
			vaddr &= ~0xF;
			trace->vaddr = vaddr;
			trace->next = NULL;
			if (!head){
				head = trace;
				tail = head;
			} else {
				tail->next = trace;
				tail = tail->next;
			}		
		} else {
			continue;
		}
	}
	curr_trace = head;
}
