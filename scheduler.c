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

#include "sysvars.h"
#include "utilTest.h"

/* Variables describing number of running/waiting processes. */
unsigned int process_counter = 0;
unsigned int soft_block_counter = 0;

/* Multiple Queues. One for each processor */
struct list_head readyQ[MAX_CPU];
pcb_t *running[MAX_CPU];

extern state_t new_old_areas[MAX_CPU][8];

void initReadyQueues() {

	int i;

	for (i = 0; i < NUM_CPU; i++)
		mkEmptyProcQ(&readyQ[i]);
}

/* Main scheduling function. */
void scheduler() {

	int i,j;

	for (i = NUM_CPU-1; i >= 0; i--) {

		if ((running[i] = removeProcQ(&readyQ[i])) != NULL) {

			if (--running[i]->priority < 0)
				running[i]->priority = 0;

			process_counter++;

			setTIMER(TIMESLICE);

			if (i > 0)
				INITCPU(i,&running[i]->p_s,new_old_areas[i]);
			else
				LDST(&running[i]->p_s);
		}

	}

}
