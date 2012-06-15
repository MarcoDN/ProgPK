/*
 * traps.c
 *
 *  Created on: 11/giu/2012
 *      Author: marco
 */

#include "types11.h"

#include "copy.h"
#include "sysvars.h"
#include "scheduler.h"

extern state_t new_old_areas[MAX_CPU][8];

void trapHandler() {

	int cpu = getPRID();
	pcb_t *current = getRunningProcess(cpu);

	copyState(((state_t*)PGMTRAP_OLDAREA),(&current->p_s));
	current->p_s.pc_epc += WORD_SIZE;

	enqueueProcess(current,cpu);

	restartScheduler();

}

void tlbHandler() {

}

