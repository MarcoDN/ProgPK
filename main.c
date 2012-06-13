/*
 * main.c
 *
 *  Created on: 11/giu/2012
 *      Author: kira
 */

/* E' sufficiente il nome del file da includere. Il path completo è specificato nel makefile. */

#include "asl.e"
#include "pcb.e"

#include "libumps.h"
#include "types11.h"

#include "scheduler.h"
#include "traps.h"
#include "syscalls.h"
#include "interrupts.h"

#include "sysvars.h"
#include "utilTest.h"

/* Multiple Priority Queues. */
extern struct list_head readyQ[];

/* We need to create a matrix in RAM to populate the other new areas for CPU
since there is no space in ROM for them. */
state_t new_old_areas[MAX_CPU][8];

/* Sets up the state_t pointed by area to invoke the handler stored at handlerAddr memaddr.
 * Disables interrupts and virtual memory. Enable Kernel Mode.
 *  */
void populateArea(state_t* area, memaddr handlerAddr) {

	area->status &= ~(STATUS_IEc | STATUS_KUc | STATUS_VMc);
	area->reg_sp = RAMTOP;
	area->pc_epc = area->reg_t9 = handlerAddr;

}

void test() {

	int i;

	for (i = 0; i < 6; i++)
		addokbuf("1 ");

}

void test2() {

	int i;

	for (i = 0; i < 6; i++)
		addokbuf("2 ");

}

int main() {

	int i,j;

	/* Setting up CPU0 new areas in ROM. */
	populateArea((state_t *)SYSBK_NEWAREA,(memaddr)sysHandler);
	populateArea((state_t *)INT_NEWAREA,(memaddr)intHandler);
	populateArea((state_t *)PGMTRAP_NEWAREA,(memaddr)trapHandler);
	populateArea((state_t *)TLB_NEWAREA,(memaddr)tlbHandler);

	/* Setting up CPUn (with n >= 1) new areas in RAM, in a defined array of state_t pointers. */
	for (i = 1; i < NUM_CPU; i++)
		for (j = 0; j < 4; j++)

			switch (j) {

			case 0: populateArea(&new_old_areas[i][0],(memaddr)intHandler); break;
			case 1: populateArea(&new_old_areas[i][2],(memaddr)sysHandler); break;
			case 2: populateArea(&new_old_areas[i][4],(memaddr)trapHandler); break;
			case 3: populateArea(&new_old_areas[i][6],(memaddr)tlbHandler); break;

			}

	/* Initialization of underlaying data structures. */
	initPcbs();
	initASL();

	/* Inizialization of device semaphores. TODO */

	semd_t *terminalRead, *terminalWrite, *psClock_timer;

	if ((psClock_timer = getSemd(0)) != NULL)
		psClock_timer -> s_value = 0;

	if ((terminalWrite = getSemd(1)) != NULL)
		terminalWrite -> s_value = 0;

	if ((terminalRead = getSemd(2)) != NULL)
		terminalRead -> s_value = 0;

	/* Scheduler initialization process. */
	initScheduler();

	/* Inizialization of first process, and its insertion into readyQueue. */
	pcb_t *starter = allocPcb();

	STST(&starter->p_s);
	starter->p_s.status |= STATUS_IEc | STATUS_INT_UNMASKED;
	starter->p_s.reg_sp = RAMTOP - (FRAME_SIZE*16);
	starter->p_s.pc_epc = starter->p_s.reg_t9 = (memaddr) test;

	insertProcQ(&readyQ[0], starter);

	/*

	pcb_t *starter2 = allocPcb();

	STST(&starter2->p_s);
	starter2->p_s.status |= STATUS_IEc | STATUS_INT_UNMASKED;
	starter2->p_s.reg_sp = starter->p_s.reg_sp - FRAME_SIZE;
	starter2->p_s.pc_epc = starter2->p_s.reg_t9 = (memaddr) test2;

	insertProcQ(&readyQ[1], starter2); */

	schedule();

	return 1;
}
