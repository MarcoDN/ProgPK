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

/* Critical Sections Variables. */
static int process_counter_Lock = 0;

/* Variables describing number of running/waiting processes. */
unsigned int process_counter = 0;
unsigned int soft_block_counter = 0;

/* Multiple Queues. One for each processor */
struct list_head readyQ[MAX_CPU];
/* Active process on each CPU. */
pcb_t *running[MAX_CPU];
/* Counter of processes assigned to each CPU, for load balancing. */
int numProc[MAX_CPU];

extern state_t new_old_areas[MAX_CPU][8];

/* Initiate all the processors' ready queues. */
void initReadyQueues() {

	int i;

	for (i = 0; i < NUM_CPU; i++)
		mkEmptyProcQ(&readyQ[i]);
}

/* Main scheduling function. */
void schedule() {

	int i = getPRID();

	if ( running[i] != NULL ||
			((running[i] = removeProcQ(&readyQ[i])) != NULL) ) {

		if (--running[i]->priority < 0)
			running[i]->priority = 0;

		/* ENTER CS. */
		while (!CAS(&process_counter_Lock,0,1)) ;

		process_counter++;

		/* EXIT CS. */
		process_counter_Lock = 0;

		setTIMER(TIMESLICE);

		LDST(&running[i]->p_s);

	}

	WAIT();

}

/* Main scheduling function.
void schedule2() {

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

} */

/* Returns the PRID of the processor with less processes assigned. */
int getShortestQ() {

	int i,min = MAXPROC;

	for (i = 0; i < NUM_CPU; i++)
		if (numProc[i] < min)
			min = i;

	return i;
}

/* Assigns a process to a Processor, waking it up if needed. */
void assignProcess(pcb_t* p) {

	int min = getShortestQ();

	insertProcQ(&readyQ[min],p);

	numProc[min]++;

	if (numProc[min] == 1) {

		state_t scheduler;

		STST(&scheduler);
		scheduler.status |= STATUS_IEc | STATUS_INT_UNMASKED;
		scheduler.reg_sp = RAMTOP - FRAME_SIZE;
		scheduler.pc_epc = scheduler.reg_t9 = (memaddr) schedule;

		INITCPU(min,&scheduler,new_old_areas[min]);

	}

}
