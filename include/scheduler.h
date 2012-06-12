/*
 * scheduler.h
 *
 *  Created on: 11/giu/2012
 *      Author: marco
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

struct list_head* initReadyQueues(int num_cpu);
void scheduler();
void test();

#endif /* SCHEDULER_H_ */
