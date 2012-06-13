/*
 * sysvars.h
 *
 *  Created on: 12/giu/2012
 *      Author: marco
 */

#ifndef SYSVARS_H_
#define SYSVARS_H_

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

/* Memory Locations */
#define SCHEDULER1 (RAMTOP - FRAME_SIZE)

#endif /* SYSVARS_H_ */
