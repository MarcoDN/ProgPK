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

/* Highest priority processes can have. */
#define MAX_PRIORITY 5

/* Entry point function's name. */
#define ENTRY_POINT (memaddr) test

/* Typedefinition for queues of pcb. */
typedef struct list_head PCB_Queue;

/* Base of CPU0 new-old areas in ROM. */
#define CPU0_NEW_OLD_AREAS ((state_t*) INT_OLDAREA)

/* Indexes of CPUn new-old areas in defined arrays.
 *
 * Defined in the same ascending order of CPU0's ones.
 *
 */
#define INT_OLDAREA_INDEX 0
#define INT_NEWAREA_INDEX 1
#define TLB_OLDAREA_INDEX 2
#define TLB_NEWAREA_INDEX 3
#define PGMTRAP_OLDAREA_INDEX 4
#define PGMTRAP_NEWAREA_INDEX 5
#define SYSBK_OLDAREA_INDEX 6
#define SYSBK_NEWAREA_INDEX 7

/* System calls type numbers. */
#define CREATEPROCESS 1
#define CREATEBROTHER 2
#define TERMINATEPROCESS 3
#define VERHOGEN 4
#define PASSEREN 5
#define GETCPUTIME 6
#define WAITCLOCK 7
#define WAITIO 8
#define SPECPRGVEC 9
#define SPECTLBVEC 10
#define SPECSYSVEC 11

/* Number of devices the system might be using. Edit this constant accordingly.
 * Pseudo-Clock
 * 8 Terminals
 * */
#define NUMDEVICE 9

/* Number of semaphores the system will be using. MAXPROC for user space's processes. NUMDEVICE for devices. 
All of them used for reading/writing, except for the pseudo-clock, which is read only. */
#define NUMSEM MAXPROC + (NUMDEVICE*2) - 1

/*
 * Constant definitions and macros to get reserved Semaphores' keys.
 *
 */
#define PSEUDO_CLOCK_SEM MAXPROC // First device reserved Semaphore. Its index is equal to MAXPROC.

#define TERMINAL_SEM MAXPROC + 1 // Terminal reserved Semaphores (reading/writing)
#define TERMINAL_SEM_R(i) TERMINAL_SEM + (i*2)
#define TERMINAL_SEM_W(i) TERMINAL_SEM + (i*2) + 1

/* Variables for debugging/testing purpose. */
U32 TEST;
U32 TEST2;

/* Utility definition. */
#define FREE 1
#define BUSY 0

/*macros definition for terminal device*/

#define RECV_STATE 0x0
#define RECV_CMD 0x4
#define TRANSM_STATE 0x8
#define TRANSM_CMD 0xC

#endif /* SYSVARS_H_ */
