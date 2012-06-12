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

#include "utilTest.h"

#define MAX_CPU 16

/* Address of register containing the number of CPUs. */
#define NCPUs 0x10000500

/* Multiple Priority Queues. */
extern struct list_head readyQ[];

/* We need to create a matrix in RAM to populate the other new areas for CPU
since there is no space in ROM for them. */
state_t new_old_areas[MAX_CPU][8];

/* Numbers of CPUs the system will be using. */
int NUM_CPU;

int main() {

	int i,j;
	pcb_t *starter; /* first process (test phase 2) */

	/* Initialize the correct number of CPUs the system will be using. */
	NUM_CPU = *((memaddr*) NCPUs);

	//0 populating new areas CPU0, using direct reference in ROM area (using memaddr)
	//interrupt
	state_t * newArea_int = (state_t *)INT_NEWAREA;
	newArea_int -> status &= ~(STATUS_IEc | STATUS_KUc | STATUS_VMc);
	newArea_int -> reg_sp = RAMTOP;
	newArea_int -> pc_epc = newArea_int->reg_t9 = (memaddr)intHandler; //TODO

	//system call
	state_t * newArea_sys = (state_t *)SYSBK_NEWAREA;
	newArea_sys -> status &= ~(STATUS_IEc | STATUS_KUc | STATUS_VMc);
	newArea_sys -> reg_sp = RAMTOP;
	newArea_sys -> pc_epc = newArea_sys->reg_t9 = (memaddr)sysHandler; //TODO

	//trap
	state_t * newArea_trap = (state_t *)PGMTRAP_NEWAREA;
	newArea_trap -> status &= ~(STATUS_IEc | STATUS_KUc | STATUS_VMc);
	newArea_trap -> reg_sp = RAMTOP;
	newArea_trap -> pc_epc = newArea_trap->reg_t9 = (memaddr)trapHandler; //TODO

	//tlb
	state_t * newArea_tlb = (state_t *)TLB_NEWAREA;
	newArea_tlb -> status &= ~(STATUS_IEc | STATUS_KUc | STATUS_VMc);
	newArea_tlb -> reg_sp = RAMTOP;
	newArea_tlb -> pc_epc = newArea_tlb->reg_t9 = (memaddr)tlbHandler; //TODO

	for(i=1; i < NUM_CPU; i++)
		for(j=0; j < 8; j++) {

			new_old_areas[i][j].status &= (STATUS_IEc | STATUS_KUc | STATUS_VMc);
			new_old_areas[i][j].reg_sp = RAMTOP;

			if(j==0)
				new_old_areas[i][j].pc_epc = new_old_areas[i][j].reg_t9 = (memaddr)intHandler;

			if(j==2)
				new_old_areas[i][j].pc_epc = new_old_areas[i][j].reg_t9 = (memaddr)sysHandler;

			if(j==4)
				new_old_areas[i][j].pc_epc = new_old_areas[i][j].reg_t9 = (memaddr)trapHandler;

			if(j==6)
				new_old_areas[i][j].pc_epc = new_old_areas[i][j].reg_t9 = (memaddr)tlbHandler;

		}

	//new areas are located in even position, instead old areas are in odd positions.
	//that's why we insert the program counter routine in that positions.
	//see first slide whose title is "Inizializzazione sistema"

	//1 inizializing semaphore and process lists
	initPcbs();
	initASL();

	//2 inizializing system semaphores


	semd_t *terminalRead, *terminalWrite, *psClock_timer;

	if ((psClock_timer = getSemd(0)) != NULL)
		psClock_timer -> s_value = 0;

	if ((terminalWrite = getSemd(1)) != NULL)
		terminalWrite -> s_value = 0;

	if ((terminalRead = getSemd(2)) != NULL)
		terminalRead -> s_value = 0;


	//2 inizializing first process descriptor, the test process

	starter = allocPcb();

	/* Adesso funziona! Ci voleva l'and bit a bit, non l'or. */
	starter ->p_s.status &= ( STATUS_IEc | STATUS_INT_UNMASKED | STATUS_KUc | ~STATUS_VMc );

	//dal file const.h leggiamo che questi tre valori significano
	//interrupt abilitati
	//kernel mode on
	//memoria virtuale accesa, quindi devo negarla
	starter->p_s.reg_sp = RAMTOP - FRAME_SIZE;
	starter->p_s.pc_epc = starter->p_s.reg_t9 = (memaddr) test;

	initReadyQueues();

	/* Insert first process into the CPU0 ready queue. */
	insertProcQ(&readyQ[0], starter);

	//4 start scheduler
	scheduler();

	return 1;

}
