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

/* Counter of processes assigned to each CPU, for load balancing and Deadlock Detection. */
unsigned int process_counter[MAX_CPU];
/* Global counter of processes waiting on I/O. */
unsigned int soft_block_counter = 0;

/* Critical Sections Variables. */
HIDDEN int prid_Lock[MAX_CPU];
/* Multiple Queues. One for each processor */
HIDDEN PCB_Queue readyQ[MAX_CPU];
/* Active process on each CPU. */
HIDDEN pcb_t *running[MAX_CPU];
/* State representing the scheduler execution. */
HIDDEN state_t scheduler;

/* Extern variables, loaded from other modules. */
extern state_t new_old_areas[MAX_CPU][8];
extern void test();

/* Enqueue the process into its previously assigned CPU's Ready Queue. */
void enqueueProcess(pcb_t* p, int prid) {

	/* ENTER CS. */
	while (!CAS(&prid_Lock[prid],FREE,BUSY)) ;

	insertProcQ(&readyQ[prid],p);

	/* EXIT CS. */
	prid_Lock[prid] = FREE;

}

/* Dequeue a process from the given CPU's Ready Queue. */
pcb_t* dequeueProcess(int prid) {

	/* ENTER CS. */
	while (!CAS(&prid_Lock[prid],FREE,BUSY)) ;

	pcb_t* result = removeProcQ(&readyQ[prid]);

	/* EXIT CS. */
	prid_Lock[prid] = FREE;

	return result;

}

/* Main scheduling function. */
void schedule() {

	int prid = getPRID();

	if ((running[prid] = dequeueProcess(prid)) != NULL) {

		/* Decrements the process's priority, since it's being scheduled for execution. */
		if (--running[prid]->priority < 0)
			running[prid]->priority = 0;
		/* Enable the Process Local Timer interrupts for this process. */
		running[prid]->p_s.status |= STATUS_TE;

		/* Sets the Interval Timer delay with the given timeslice, for RR purpose. */
		setTIMER(SCHED_TIME_SLICE);

		LDST(&running[prid]->p_s);

	}
	else if (process_counter[prid] > 0 && soft_block_counter == 0)  /* Performs some Deadlock Detection. */
		PANIC();

	WAIT();

}

/* Restart the scheduler. Loading the proper state. */
void restartScheduler() {

	LDST(&scheduler);

}

/* Returns a pointer to the process currently executed by the CPU with given prid. */
pcb_t* getRunningProcess(int prid) {

	return running[prid];

}

/* Returns the PRID of the processor with less processes assigned. */
int getShortestQ() {

	int i,prid,min = MAXPROC; /* One processor can have at most MAXPROC processes. */

	for (i = 0; i < NUM_CPU; i++)
		if (process_counter[i] < min) {
			min = process_counter[i];
			prid = i;
		}

	return prid;
}

/* Assigns a new process to a Processor, waking it up if needed. */
void assignProcess(pcb_t* p) {

	/* Uses a policy of load balancing. */
	int prid = getShortestQ();

	enqueueProcess(p,prid);

	/* ENTER CS. */
	while (!CAS(&prid_Lock[prid],FREE,BUSY)) ;

	/* Increments the choosen CPU's process counter. */
	process_counter[prid]++;

	/* EXIT CS. */
	prid_Lock[prid] = FREE;

	if (process_counter[prid] == 1)
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

		process_counter[i] = 0;
		prid_Lock[i] = FREE;

		if (i == 0) {

			/* Inizialization of first process, and its insertion into readyQueue. */
			pcb_t *starter = allocPcb();

			/* TESTING
			starter->p_s.status &= ~(STATUS_IEc | STATUS_KUc | STATUS_VMc);
			 */
			starter->p_s.status |= STATUS_IEp | STATUS_INT_UNMASKED;
			starter->p_s.reg_sp = RAMTOP - (FRAME_SIZE * (offset+1));
			starter->p_s.pc_epc = starter->p_s.reg_t9 = ENTRY_POINT;

			/* Sets the global Interval Timer for an accounting of Pseudo-Clock ticks. */
			SET_IT(SCHED_PSEUDO_CLOCK);

			/* Entry point. */
			assignProcess(starter);

		}
		else
			INITCPU(i,&scheduler,new_old_areas[i]);

	}

}
