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

	copyState(((state_t*)PGMTRAP_OLDAREA),(&running[cpu]->p_s));
	running[cpu]->p_s.pc_epc += WORD_SIZE;

	enqueueProcess(running[cpu],cpu);

	LDST(&scheduler);


}

void tlbHandler() {

}

