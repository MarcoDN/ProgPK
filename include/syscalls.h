/*
 * syscalls.h
 *
 *  Created on: 11/giu/2012
 *      Author: marco
 */

#ifndef SYSCALLS_H_
#define SYSCALLS_H_

extern void sysHandler();
extern unsigned int sem_esclusion[MAXPROC];
extern void P(semd_t *s, int key, pcb_t *caller);
extern void V(semd_t *s, int key);

#endif /* SYSCALLS_H_ */
