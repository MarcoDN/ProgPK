/*
 * scheduler.h
 *
 *  Created on: 11/giu/2012
 *      Author: marco
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

extern unsigned int process_counter;
extern unsigned int soft_block_counter;
extern struct list_head readyQ[];
extern pcb_t *running[];
extern int numProc[];
extern state_t scheduler;

extern void initScheduler(int offset);
extern void assignProcess(pcb_t *p);

#endif /* SCHEDULER_H_ */
