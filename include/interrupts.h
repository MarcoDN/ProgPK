/*
 * interrupts.h
 *
 *  Created on: 11/giu/2012
 *      Author: marco
 */

#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_

extern semd_t *terminalRead;
extern semd_t *terminalWrite;
extern semd_t *psClock_timer;
extern void intHandler();

#endif /* INTERRUPTS_H_ */
