/*
 * traps.c
 *
 *  Created on: 11/giu/2012
 *      Author: marco
 */

#include "types11.h"
#include "const.h"
#include "copy.h"
#include "sysvars.h"
#include "scheduler.h"
#include "syscalls.h"
#include "terminate.h"

/*Macro per accesso alla OldArea come stato del processore*/
#define progtrap_oldarea	((state_t *) PGMTRAP_OLDAREA)
#define sysbp_oldarea  		((state_t *) SYSBK_OLDAREA)
#define tbltrap_oldarea		((state_t *) TLB_OLDAREA)

extern state_t new_old_areas[MAX_CPU][8];

/*Gestore delle eccezione statenate da ProgramTrap*/
void trapHandler(){
	/*Variabili per l'utilizzo delle critical section*/
	int p=0;
  
	/*Ottengo il numero del processo che ha generato la exception*/
	int cpu = getPRID();
	pcb_t *current = getRunningProcess(cpu);
	
	/*Salvo il vecchio stato del processo*/
	copyState(progtrap_oldarea,(&current->p_s));
	current->p_s.pc_epc += WORD_SIZE;;

	/* Se non è definita la proTrapmanager il processo viene ucciso */
	if(current->def_areas[3] == 0){
	  
		/*Inizio Critical section*/
		while (!CAS(&p,0,1));
	  
		/*Killo il processo*/
		terminateprocess(current, cpu);
		
		/*Termine della Critical Section*/
		CAS(&p,1,0);
		
		current = NULL;
	}
	
	/* Altrimenti viene salvata la pgmTrap Old Area all'interno del processo corrente*/
	else{
		copyState(progtrap_oldarea, current->def_areas[2]);
		/*Carico lo stato personalizzato all'interno della new area*/
		copyState(current->def_areas[3], (&current->p_s));
	}
	

	
	/*Passo il controllo allo scheduler*/
	restartScheduler();
}

/*Gestore delle eccezione statenate da ProgramTrap*/
void tlbHandler(){
	/*Variabili per l'utilizzo delle critical section*/
	int d=0;
  
	/*Ottengo il numero del processo che ha generato la exception*/
	int cpu = getPRID();
	pcb_t *current = getRunningProcess(cpu);

	/*Salvo il vecchio stato del processo*/
	copyState(tbltrap_oldarea, (&current->p_s));
	current->p_s.pc_epc += WORD_SIZE;

	/* Se non è definita la proTrapmanager il processo viene ucciso */
	if(current-> def_areas[1] == 0){
	  
		/*Inizio Critical section*/
		while (!CAS(&d,0,1));
	  
		/*Killo il processo*/
		terminateprocess(current, cpu);
		
		/*Termine della Critical Section*/
		CAS(&d,1,0);
		
		current = NULL;
	}
	
	/* Altrimenti viene salvata la pgmTrap Old Area all'interno del processo corrente*/
	else
		copyState(tbltrap_oldarea, current->def_areas[0]);
		/*Carico lo stato personalizzato all'interno della new area*/
		copyState(current->def_areas[1], (&current->p_s));
	
	/*Passo il controllo allo scheduler*/
	restartScheduler();
}

/*TODO: Pezzo di codice per la gestione delle syscalltrap, non so se Lory lo debba aggiungere nel suo gestore o se me ne devo occupare io
void syscall_Handler(){
	/*Variabile per la critical section
	int s=0;

	/*Ottengo il numero del processo che ha generato la exception
	int cpu = getPRID();
	pcb_t *current = getRunningProcess(cpu);

	/*Salvo il vecchio stato del processo
	copyState(sysbp_oldarea, (&current->p_s));
	current->p_s.pc_epc += WORD_SIZE;

	/* Se non è definita la proTrapmanager il processo viene ucciso 
	if(current-> def_areas[5] == 0){
	  
		/*Inizio Critical section
		while (!CAS(&s,0,1));
	  
		/*Killo il processo
		terminateprocess(current, cpu);
		
		/*Termine della Critical Section
		CAS(&s,1,0);
		current = NULL;
	}
	
	/* Altrimenti viene salvata la pgmTrap Old Area all'interno del processo corrente
	else
		copyState(sysbp_oldarea, current->def_areas[4]);
		/*Carico lo stato personalizzato all'interno della new area
		copyState(current->def_areas[5], (&current->p_s));
	
	/*Passo il controllo allo scheduler
	restartScheduler();
}
*/
