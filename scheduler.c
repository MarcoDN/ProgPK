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

#define MAX_CPU 16

/* Variables describing number of running/waiting processes. */
unsigned int Process_Counter = 0;
unsigned int Soft_Block_Counter = 0;

/* Ready Queues. */
struct list_head readyQ[MAX_CPU];
/* Running processes. */
pcb_t* running[MAX_CPU];

/* Number of currently used CPUs. */
extern int NUM_CPU;

struct list_head* initReadyQueues(int num_cpu) {

	int i;

	for (i = 0; i < num_cpu; i++)
		mkEmptyProcQ(&readyQ[i]);

	return &readyQ[0];

}

void test() {

	int i;

	for (i = 0; i < 10000; i++)
		addokbuf("Test!");
}

void scheduler() {

	int i;

	for (i = 0; i < NUM_CPU; i++)
		if (!emptyProcQ(&readyQ[i]))
			running[i] = headProcQ(&readyQ[i]);

	if (running[0] != NULL)
		LDST(&running[0]->p_s);

}
