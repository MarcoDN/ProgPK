/*
 * sysvars.h
 *
 *  Created on: 12/giu/2012
 *      Author: marco
 */

#ifndef SYSVARS_H_
#define SYSVARS_H_

/* Processor Local Timer bit's position. */
#define STATUS_TE 0x08000000

/* Maximum number of supported CPUs. */
#define MAX_CPU 16

/* Address of register containing the number of CPUs. */
#define NCPUs 0x10000500
/* Initialize the correct number of CPUs the system will be using. */
#define NUM_CPU *((memaddr*) NCPUs)

/* Timeslice in microseconds. */
#define TIMESLICE 5000

/* Highest priority processes can have. */
#define MAX_PRIORITY 5

/* Entry point function's name. */
#define ENTRY_POINT (memaddr) test

/* Typedefinition for queues of pcb. */
typedef struct list_head PCB_Queue;

/* Base of CPU0 new-old areas in ROM. */
#define CPU0_NEW_OLD_AREAS ((state_t*) INT_OLDAREA)

/* Variables for debugging/testing purpose. */
U32 TEST;
U32 TEST2;

#endif /* SYSVARS_H_ */
