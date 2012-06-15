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
#include "sysvars.h"

#include "scheduler.h"
#include "traps.h"
#include "syscalls.h"
#include "interrupts.h"

#include "utilTest.h"

/* We need to create a matrix in RAM to populate the other new areas for CPU
since there is no space in ROM for them. */
state_t new_old_areas[MAX_CPU][8];

/* Sets up the state_t pointed by area to invoke the handler stored at handlerAddr memaddr.
 * Position the stack pointer at the given offset from RAMTOP.
 * Disables interrupts and virtual memory. Enable Kernel Mode.
 *  */
void populateArea(state_t* area, memaddr handlerAddr, int offset) {

	area->status = (area->status & ~(STATUS_IEc | STATUS_KUc | STATUS_VMc)) | STATUS_TE;
	area->reg_sp = RAMTOP - (FRAME_SIZE*offset);
	area->pc_epc = area->reg_t9 = handlerAddr;

}

void test2() {

	int i = 0;

	while (i++ < 100000)
		;

	addokbuf("Test2 ended");

}

void test() {

	int i = 0;

	while (i++ < 100000)
		;
/*
	pcb_t *starter2 = allocPcb();

	STST(&starter2->p_s);
	starter2->p_s.status = starter2->p_s.status | STATUS_IEp | STATUS_INT_UNMASKED;
	starter2->p_s.reg_sp = starter2->p_s.reg_sp - FRAME_SIZE;
	starter2->p_s.pc_epc = starter2->p_s.reg_t9 = (memaddr) test2;

	SYSCALL(1,(int) &starter2->p_s,1,0); */

	//addokbuf("Test ended");

}

int main() {

	int i,j;

	/* Setting up CPU0 new areas in ROM. */
	populateArea((state_t *)INT_NEWAREA,(memaddr)intHandler,0);
	populateArea((state_t *)TLB_NEWAREA,(memaddr)tlbHandler,0);
	populateArea((state_t *)PGMTRAP_NEWAREA,(memaddr)trapHandler,0);
	populateArea((state_t *)SYSBK_NEWAREA,(memaddr)sysHandler,0);

	/* Setting up CPU-n (with n >= 1) new areas in RAM, in a defined array of state_t pointers. */
	for (i = 1; i < NUM_CPU; i++)
		for (j = 0; j < 4; j++)

			switch (j) {

			case 0: populateArea(&new_old_areas[i][1],(memaddr)intHandler,i); break;
			case 1: populateArea(&new_old_areas[i][3],(memaddr)tlbHandler,i); break;
			case 2: populateArea(&new_old_areas[i][5],(memaddr)trapHandler,i); break;
			case 3: populateArea(&new_old_areas[i][7],(memaddr)sysHandler,i); break;

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
	initScheduler(i);

	return 1;
}
