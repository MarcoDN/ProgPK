/*
 * scheduler.h
 *
 *  Created on: 11/giu/2012
 *      Author: marco
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

extern unsigned int process_counter[MAX_CPU];
extern unsigned int soft_block_counter;

pcb_t* getRunningProcess(int prid);
void restartScheduler();
extern void initScheduler(int offset);
extern void assignProcess(pcb_t *p);
extern void enqueueProcess(pcb_t* p, int prid);

#endif /* SCHEDULER_H_ */
