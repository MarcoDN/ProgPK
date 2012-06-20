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

#include "libumps.h"

/*Macro per accesso alla OldArea come stato del processore*/
#define progtrap_oldarea	((state_t *) PGMTRAP_OLDAREA)
#define sysbp_oldarea  		((state_t *) SYSBK_OLDAREA)
#define tbltrap_oldarea		((state_t *) TBL_OLDAREA)

extern state_t new_old_areas[MAX_CPU][8];

/*Gestore delle SysCall e BreackPoint*/
void trapsHandler() {
	int cause_exCode, KoUMode;
  
	/*Recupero il bit di modalità della sysBp Old Area (controllo se Kernel o User mode)*/
	KoUMode = ((sysbp_oldarea->status) & STATUS_KUp) >> 3;

	/*Recupero il codice del tipo di eccezione scatetanata*/
	cause_exCode = CAUSE_EXCCODE_GET(sysbp_oldarea->cause);
	
	/*Analizzo i casi possibili di trap, mandando tutto ai vari gestori*/
	switch (cause_exCode){
	
	  case 1: {
	    
	  }
	  
	  case 2: {
	    
	  }
	  
	  case 3:{
	    
	  }
	  
	}
}

/*Gestore delle eccezione statenate da ProgramTrap*/
void progTrapHandler(){

	/*Ottengo il numero del processo che ha generato la exception*/
	int cpu = getPRID();
	pcb_t *current = getRunningProcess(cpu);

	/*Salvo il vecchio stato del processo*/
	copyState(progtrap_oldarea, (&current->p_s));
	current->p_s.pc_epc += WORD_SIZE;

	/* Se il processo è in esecuzione, viene direttamente caricato il gestore*/
	if(current != NULL)
		copyState(progtrap_oldarea, (&current->p_s));

	/* Se non è definita la proTrapmanager il processo viene ucciso
	if(current-> nome di quel cazzo di vettore per le trap == 0){
		termintate(current);
		current = NULL;
		restartScheduler();
	}
	/* Altrimenti viene salvata la pgmTrap Old Area all'interno del processo corrente
	else{
		copyState(pgmTrap_old, current->propria area di memoria di progtrap);
		/*Routine trovata in libumps.h che permette di ricaricare il vecchio stato corrente del processore.
		NB: routine ROM --> SOLO IN KERNEL MODE
		LDST(current->nuova area di memoria di progtrap)
	}*/
}

/*Gestore delle eccezioni scatenate da TBLEXCEPTION*/
void tblExcHandler(){

	/*Ottengo il numero del processo che ha generato la exception*/
	int cpu = getPRID();
	pcb_t *current = getRunningProcess(cpu);

	/*Salvo il vecchio stato del processo*/
	copyState(tbltrap_oldarea, (&current->p_s));
	current->p_s.pc_epc += WORD_SIZE;
	
	/* Se non è definita la tblExcmanager il processo viene ucciso
	if(current-> nome di quel cazzo di vettore per le trap == 0){
		termintate(current);
		current = NULL;
		restartScheduler();
	}
	/* Altrimenti viene salvata la pgmTrap Old Area all'interno del processo corrente
	else{
		copyState(pgmTrap_old, current->propria area di memoria di tblexception);
		/*Routine trovata in libumps.h che permette di ricaricare il vecchio stato corrente del processore.
		NB: routine ROM --> SOLO IN KERNEL MODE
		LDST(current->nuova area di memoria di progtrap)
	}*/
}
