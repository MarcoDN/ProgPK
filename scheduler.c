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
PCB_Queue readyQ[MAX_CPU];
/* Active process on each CPU. */
pcb_t *running[MAX_CPU];
/* Counter of processes assigned to each CPU, for load balancing. */
int numProc[MAX_CPU];

state_t scheduler;

extern state_t new_old_areas[MAX_CPU][8];
extern void test();

/* Main scheduling function. */
void schedule() {

	int i = getPRID();

	if ((running[i] = removeProcQ(&readyQ[i])) != NULL) {

		if (--running[i]->priority < 0)
			running[i]->priority = 0;

		/* ENTER CS. */
		while (!CAS(&process_counter_Lock,0,1)) ;

		process_counter++;

		/* EXIT CS. */
		process_counter_Lock = 0;

		/* Sets the Interval Timer delay with the given timeslice, for RR purpose. */
		SET_IT(TIMESLICE);

		LDST(&running[i]->p_s);

	}

	WAIT();

}

/*
void schedule2() {

	int i,j;

	for (i = NUM_CPU-1; i >= 0; i--) {

		if ((running[i] = removeProcQ(&readyQ[i])) != NULL) {

			if (--running[i]->priority < 0)
				running[i]->priority = 0;

			process_counter++;

			SET_IT(TIMESLICE);

			if (i > 0)
				INITCPU(i,&running[i]->p_s,new_old_areas[i]);
			else
				LDST(&running[i]->p_s);
		}

	}

} */

/* Returns the PRID of the processor with less processes assigned. */
int getShortestQ() {

	int i,prid,min = MAXPROC; /* One processor can have at most MAXPROC processes. */

	for (i = 0; i < NUM_CPU; i++)
		if (numProc[i] < min) {
			min = numProc[i];
			prid = i;
		}

	return prid;
}

/* Assigns a process to a Processor, waking it up if needed. */
void assignProcess(pcb_t* p) {

	int prid = getShortestQ();

	insertProcQ(&readyQ[prid],p);

	numProc[prid]++;

	if (numProc[prid] == 1)
		if (prid > 0)
			INITCPU(prid,&scheduler,new_old_areas[prid]);
		else
			INITCPU(prid,&scheduler,CPU0_NEW_OLD_AREAS);

}

/* Initiate all the scheduler's structures. */
void initScheduler(int offset) {

	int i;

	scheduler.status &= ~(STATUS_IEc | STATUS_KUc | STATUS_VMc);
	scheduler.reg_sp = RAMTOP - (FRAME_SIZE * offset);
	scheduler.pc_epc = scheduler.reg_t9 = (memaddr) schedule;

	for (i = NUM_CPU-1; i >= 0; i--) {

		mkEmptyProcQ(&readyQ[i]);

		numProc[i] = 0;

		if (i == 0) {

			/* Inizialization of first process, and its insertion into readyQueue. */
			pcb_t *starter = allocPcb();

			/* TESTING
			starter->p_s.status &= ~(STATUS_IEc | STATUS_KUc | STATUS_VMc);
			 */
			starter->p_s.status |= STATUS_IEp | STATUS_INT_UNMASKED;
			starter->p_s.reg_sp = RAMTOP - (FRAME_SIZE * (offset+1));
			starter->p_s.pc_epc = starter->p_s.reg_t9 = ENTRY_POINT;

			/* Entry point. */
			assignProcess(starter);

		}
		else
			INITCPU(i,&scheduler,new_old_areas[i]);

	}

}
