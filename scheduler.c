/*
 * scheduler.c
 *
 *  Created on: 11/giu/2012
 *      Author: marco
 */

#include "asl.e"
#include "pcb.e"

#include "libumps.h"
#include "types11.h"

#include "utilTest.h"

#define MAX_PRIORITY 5
#define MAX_CPU 16

extern int NUM_CPU;

/* Timeslice in microseconds. */
#define TIMESLICE 5000

/* Variables describing number of running/waiting processes. */
unsigned int process_counter = 0;
unsigned int soft_block_counter = 0;

/* Multiple Priority Queues. */
struct list_head readyQ[MAX_PRIORITY+1];
/* Running processes. */
pcb_t* running[MAX_CPU];

void initReadyQueues() {

	int i;

	for (i = 0; i < MAX_PRIORITY+1; i++)
		mkEmptyProcQ(&readyQ[i]);
}

void test() {

	int i;

	for (i = 0; i < 10; i++)
		addokbuf("Test! ");

}

void scheduler() {

	int i,j;

	for (i = MAX_PRIORITY; i >= 0; i--) {

		pcb_t *curr;

		if ((curr = headProcQ(&readyQ[i])) != NULL) {

			for (j = 0; j < NUM_CPU; j++)
				if (running[j] == NULL) {
					running[j] = curr;

					process_counter++;

					SET_IT(TIMESLICE);

					if (j > 0) {

						state_t *currState; /* Quale state passare come secondo parametro..? */

						INITCPU(j,&curr->p_s,currState);
						break;
					}
					else
						LDST(&curr->p_s);
				}

		}

	}

}
