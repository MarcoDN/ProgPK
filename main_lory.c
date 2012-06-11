/*
 * main.c
 *
 *  Created on: 11/giu/2012
 *      Author: kira
 */

#include "/home/kira/University/Fase_1/include/asl.e"
#include "/home/kira/University/Fase_1/include/pcb.e"
#include "/home/kira/University/Fase_1/include/base.h"
#include "/home/kira/University/Fase_1/include/const.h"
#include "/home/kira/University/Fase_1/include/libumps.h"
#include "/home/kira/University/Fase_1/include/const11.h"
#include "/home/kira/University/Fase_1/include/types11.h"
#include "/home/kira/University/Fase_1/include/uMPStypes.h"

void wait() {
	while(1) ;
}


void main(void) {

	/*extern void test();
	extern void intHandler();
	extern void sysHandler();
	extern void trapHandler();
	extern void scheduler();
	extern void tlbHandler();*/

	pcb_t *starter; //first process (test phase 2)
	int i, j;

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


	//We need to create a matrix in RAM to populate the other new areas for CPU
	//since there is no space in ROM for them.

	state_t new_old_areas[NUM_CPU][8];
	int i, j;
	for(i=1;i<NUM_CPU;i++) {
		for(j=0;j < 8;j++) {
			new_old_areas[i][j].status &= (STATUS_IEc | STATUS_KUc | STATUS_VMc);
			new_old_areas[i][j].reg_sp = RAMTOP;

			if(j==0){
				new_old_areas[i][j].pc_epc = new_old_areas[i][j].reg_t9 = (memaddr)sysHandler;
			}

			if(j==2){
				new_old_areas[i][j].pc_epc = new_old_areas[i][j].reg_t9 = (memaddr)trapHandler;
			}

			if(j==4){
				new_old_areas[i][j].pc_epc = new_old_areas[i][j].reg_t9 = (memaddr)tlbHandler;
			}

			if(j==6){
				new_old_areas[i][j].pc_epc = new_old_areas[i][j].reg_t9 = (memaddr)trapHandler;
			}

		}
	}

	//we decide to initialize the other CPU by keeping them idle. They keep on waiting in a while
	//cycle until the scheduler gives them a process to execute

	state_t idle[NUM_CPU]; //at the beginning all the other CPU are in this state

	for(i=1;i<NUM_CPU;i++) {
		idle[i].reg_sp = RAMTOP;
		idle[i].pc_epc = idle[i].reg_t9 = (memaddr)wait;
		idle[i].status = idle[i].status | STATUS_IEc | STATUS_INT_UNMASKED;
	}

	for(i=1;i<NUM_CPU;i++) {
		INITCPU(i, &idle[i], &new_old_areas[i]);
	}

	//new areas are located in even position, instead old areas are in odd positions.
	//that's why we insert the program counter routine in that positions.
	//see first slide whose title is "Inizializzazione sistema"



	//inizializing kernel variables
	unsigned int Process_Counter = 0;
	unsigned int SBlock_Counter = 0;
	//queste variabili, numero processi arrivi e numero processi bloccati, le
	//dichiariamo così noi oppure vanno prese da qualche parte?
	//nei file di inclusione non le ho viste.


	//1 inizializing semaphore and process lists
	initPcbs();
	initASL();
	//declaration of the ready queue (an element of type list_head)
	struct list_head ready_h;
	INIT_LIST_HEAD(&ready_h);




	//2 inizializing system semaphores

		/*
		//siccome i semafori della ASL sono per i processi, immagino che i
		//semafori di sistema vadano creati a parte, o no?
		semd_t terminalRead;
		terminalRead.s_value = 0;

		semd_t terminalWrite;
		terminalWrite.s_value = 0;

		semd_t psClock_timer;
		psClock_timer.s_value = 0;*/

		//oppure prelevo i primi tre semafori dal vettore sem_table

		semd_t terminalRead, terminalWrite, psClock_timer;
		psClock_timer = getSemd(0);
		psClock_timer -> s_value = 0;

		terminalWrite = getSemd(1);
		terminalWrite -> s_value = 0;

		terminalRead = getSemd(2);
		terminalWrite -> s_value = 0;



	//2 inizializing first process descriptor, the test process
	starter = allocPcb();
	starter ->p_s.status = (starter->p_s.status) | STATUS_IEc | STATUS_INT_UNMASKED | STATUS_KUc | (~STATUS_VMc);
	//dal file const.h leggiamo che questi tre valori significano
	//interrupt abilitati
	//kernel mode on
	//memoria virtuale accesa, quindi devo negarla
	starter->p_s.reg_sp = RAMTOP - FRAMESIZE;
	starter->p_s.pc_epc = starter->p_s.reg_t9 = (memaddr)test;

	//3 insert first process into ready queue
	insertProcQ(&ready_h, starter);

	//4 start scheduler
	scheduler();

}



