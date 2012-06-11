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

void main(void) {

	pcb_t *starter; //first process (test phase 2)

	//0 populating new areas CP0, reading from file const.h where they are defined
	state_t * newArea_int;
	//kernel mode on, e memoria virtuale spenta
	newArea_int -> status &= ~(STATUS_IEc | STATUS_KUc | STATUS_VMc);
	newArea_int -> reg_sp = RAMTOP;
	newArea_int -> pc_epc = newArea_int->reg_t9 (memaddr)intHandler; //TODO


	//per gli altri processori devo creare una matrice del tipo
	state_t new_areas[NUM_CPU][8];
	//ma come le inizializzo? nello stesso modo?


	//inizializing kernel variables
	unsigned int Process_Counter = 0;
	unsigned int SBlock_Counter = 0;
	//queste variabili, numero processi arrivi e numero processi bloccati, le
	//dichiariamo così noi oppure vanno prese da qualche parte?
	//nei file di inclusione non le ho viste.


	//declaration of the ready queue (an element of type list_head)
	struct list_head ready_h;
	INIT_LIST_HEAD(&ready_h);
	//1 inizializing semaphore and process lists
	initPcbs();
	initASL();



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



	//2 inizializing first process descriptor
	starter = allocPcb();
	starter ->p_s.status = (starter->p_s.status) | STATUS_IEc | STATUS_KUc | STATUS_VMc;
	//dal file const.h leggiamo che questi tre valori significano
	//bit spenti
	//kernel mode on
	//memoria virtuale spenta
	starter->p_s.reg_sp = RAMTOP - FRAMESIZE;
	starter->p_s.pc_epc = starter->p_s.reg_t9 = (memaddr)test();

	//3 insert first process into ready queue
	insertProcQ(&ready_h, starter);

	//4 start scheduler
	scheduler();

}



